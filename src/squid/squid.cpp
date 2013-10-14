/**
 * Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)
 *
 * We release this software open source under a Creative Commons Attribution-
 * NonCommercial 3.0 Unported License. Please cite the papers listed on
 * http://tschaffter.ch/projects/squid/ when using sQuid in your publication.
 *
 * For commercial use, please contact Thomas Schaffter.
 *
 * A brief description of the license is available at:
 * http://creativecommons.org/licenses/by-nc/3.0/
 *
 * The full license is available at:
 * http://creativecommons.org/licenses/by-nc/3.0/legalcode
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "squid.h"
#include "ui_squid.h"
#include "about.h"
#include "squidsettings.h"
#include "aoidialog.h"
#include "myutility.h"
#include "dc1394utility.h"
#include "global.h"
#include "cameraconfiguration.h"
#include "booleanplaylist.h"
#include "experimenttime.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <QString>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QtGui/QCloseEvent>
#include <QDesktopWidget>
#include <glog/logging.h>
#include <sys/time.h>

using namespace squid;
using namespace qsquid;

// ======================================================================
// PRIVATE METHODS

void Squid::initialize() throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();
    setWindowIcon(settings->getApplicationWindowIcon());
    std::string title = std::string("sQuid ") + std::string(SQUID_VERSION);
    this->setWindowTitle(QString(title.c_str()));

    try {
        cmanager_ = CameraManager::getInstance();
        cmanager_->initialize(settings->getDc1394());
    } catch (MyException* e) {
        checkCamerasAvailability();
    }

    aoiAction_ = NULL;
    dmanager_ = NULL;
    experiment_ = NULL;

    importSettings();
    initializeGuiComponents();
    updateGui();

    makeConnections();
}

// ----------------------------------------------------------------------

void Squid::checkCamerasAvailability() {

    // if no camera has been detected, prompt a dialog to the user
    if (cmanager_ == NULL || cmanager_->getNumCameras() < 1) {
        LOG(WARNING) << "No camera available. Redetect cameras ?";
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setWindowIcon(SquidSettings::getInstance()->getApplicationWindowIcon());
        msgBox.setText("No camera available.");
        msgBox.setInformativeText("Redetect cameras ?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIconPixmap(QPixmap::fromImage(QImage(":/redetect_camera")));
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            cmanager_ = CameraManager::getInstance(); // try to redetect the cameras and redo the check
            cmanager_->initialize(SquidSettings::getInstance()->getDc1394());
            checkCamerasAvailability();
            return;
            break;
        case QMessageBox::Cancel: // exit

            exit(0);
            break;
        default:
            // should never be reached
            break;
        }
    }
}

// ----------------------------------------------------------------------

void Squid::initializeGuiComponents() throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();

    // CAMERAS SECTION
    listAllCameras(); // fill dedicated combobox
    buildCameraMenu();
    updateCameraControllers();
    ui_->triggerPeriodSpinBox->setRange(1, 1000);

    // EXPERIMENT SECTION
    ui_->experimentNameEdit->setText(settings->getExperimentName().c_str());
    ui_->durationSpinbox->setRange(1, 9999); // from 1 to 9999 min
    ui_->durationSpinbox->setValue(settings->getExperimentDuration());
    ui_->experimentDescriptionEdit->setText("");
    ui_->experimentProgressBarLayout->layout()->addWidget(&experimentProgressBar_);
    experimentProgressBar_.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui_->runExperimentButton->setEnabled(true);
    ui_->stopExperimentButton->setEnabled(false);
    ui_->outputFormat->addItem(QString("PGM images"));
    ui_->outputFormat->addItem(QString("TIFF images"));
    ui_->outputFormat->setCurrentIndex(settings->getOutputFormat());

    // OUTPUT SECTION
    ui_->workingDirectoryEdit->setText(QString(settings->getWorkingDirectory().c_str()));

    this->resize(this->minimumSize());
    //repaint();
    update();
}

// ----------------------------------------------------------------------

void Squid::listAllCameras() {

    const unsigned int numCameras = cmanager_->getNumCameras();
    std::string cameraId;
    QIcon cameraIcon(":/camera");

    for (unsigned int i = 0; i < numCameras; i++) {
        cameraId = cmanager_->getCamera(i)->getCameraName() + " (" + cmanager_->getCamera(i)->getCameraGuid() + ")";
        ui_->camerasCombobox->addItem(cameraIcon, QString(cameraId.c_str()));
    }
    ui_->camerasCombobox->setCurrentIndex(cmanager_->getCameraIndex());
}

// ----------------------------------------------------------------------

void Squid::buildCameraMenu() {

    buildCameraResolutionSubMenu();
    buildCameraFramerateSubMenu();
    connect(ui_->resolutionAction->menu(), SIGNAL(triggered(QAction*)), this, SLOT(changeCameraResolution(QAction*)));
    connect(ui_->framerateAction->menu(), SIGNAL(triggered(QAction*)), this, SLOT(changeCameraFramerate(QAction*)));
}

// ----------------------------------------------------------------------

void Squid::buildCameraResolutionSubMenu() {

    if (ui_->resolutionAction->menu() != NULL) {
        delete ui_->resolutionAction->menu();
        delete resolutionActionGroup_;
    }

    QAction* a;
    QMenu* menu = new QMenu(this);

    resolutionActionGroup_ = new QActionGroup(this);
    // get ONLY the resolution supported by the current camera
    std::vector<std::string> strResolution = getSupportedResolutionsLabels(cmanager_->getCamera()->getCamera());
    std::string str = "";
    unsigned int n = strResolution.size();

    for (unsigned int i = 0; i < n; i++) {

        str = strResolution.at(i);
        a = new QAction(str.c_str(), this);
        a->setCheckable(true);

        // among all the resolutions supported by the camera, discards:
        // TODO: YUV, MONO16 and RGB images not supported by sQuid (both display and saving images), idem for FORMAT7
        if (str.find("YUV") != str.npos || str.find("MONO16") != str.npos || str.find("RGB") != str.npos || str.find("FORMAT7") != str.npos)
            a->setEnabled(false);
        else
            resolutionActionGroup_->addAction(a);

        menu->addAction(a);
    }
    menu->addSeparator();

    aoiAction_ = new QAction("Set AOI...", this);
    aoiAction_->setCheckable(true);
    menu->addAction(aoiAction_);

    // from the available resolution, selects the one used by the current camera
    Dc1394Camera* camera = cmanager_->getCamera();
    if (camera->useAoi())
        aoiAction_->setChecked(true);
    else {
        QList<QAction*> list = resolutionActionGroup_->actions();
        str = dc1394ToSringResolution(cmanager_->getCamera()->getResolution());
        n = list.size();
        for (unsigned int i = 0; i < n; i++) {
            if (strcmp(list.at(i)->text().toStdString().c_str(), str.c_str()) == 0)
                list.at(i)->setChecked(true);
        }
    }
    // add sub-menu
    ui_->resolutionAction->setMenu(menu);
}

// ----------------------------------------------------------------------

void Squid::buildCameraFramerateSubMenu() {

    if (ui_->framerateAction->menu() != NULL) {
        delete ui_->framerateAction->menu();
        delete fpsActionGroup_;
    }

    QAction* a = NULL;
    QMenu* menu = new QMenu(this);

    fpsActionGroup_ = new QActionGroup(this);
    // get ONLY the framerate supported by the current camera AND current resolution used
    std::vector<std::string> strFps = getSupportedFpsLabels(cmanager_->getCamera()->getCamera(), cmanager_->getCamera()->getResolution());
    unsigned int n = strFps.size();
    std::string str = "";
    for (unsigned int i = 0; i < n; i++) {
        str = strFps.at(i);
        a = new QAction(str.c_str(), this);
        a->setCheckable(true);
        fpsActionGroup_->addAction(a);
        menu->addAction(a);
    }

    // from the available framerate, selects the one used by the current camera
    QList<QAction*> list = fpsActionGroup_->actions();
    str = dc1394ToStringFps(cmanager_->getCamera()->getFps());
    n = list.size();
    for (unsigned int i = 0; i < n; i++) {
        if (strcmp(list.at(i)->text().toStdString().c_str(), str.c_str()) == 0)
            list.at(i)->setChecked(true);
    }
    // add sub-menu
    ui_->framerateAction->setMenu(menu);
}

// ----------------------------------------------------------------------

void Squid::updateCameraControllers() throw(MyException*) {

    // get the current camera mode and check the corresponding radio button
    CameraManager::cameraMode mode = cmanager_->getCameraMode();
    if (mode == CameraManager::FREERUN)
        ui_->freerunRadioButton->setChecked(true);
    else if (mode == CameraManager::SOFTWARE_TRIGGERS)
        ui_->triggersRadioButton->setChecked(true);
    else
        throw new MyException("Invalid camera mode.");

    // set the sliders associated to the control of the camera (gain, shutter and brightness)
    unsigned int min, max;
    cmanager_->getCamera()->getGainRange(min, max);
    ui_->gainSlider->setRange(min, max);
    ui_->gainSlider->setValue(cmanager_->getCamera()->getGain());
    ui_->gainSpinbox->setRange(min, max);
    ui_->gainSpinbox->setValue(cmanager_->getCamera()->getGain());

    cmanager_->getCamera()->getStdShutterRange(min, max);
    ui_->stdShutterSlider->setRange(min, max);
    ui_->stdShutterSlider->setValue(cmanager_->getCamera()->getStdShutter());
    ui_->stdShutterSpinbox->setRange(min, max);
    ui_->stdShutterSpinbox->setValue(cmanager_->getCamera()->getStdShutter());

    cmanager_->getCamera()->getBrightnessRange(min, max);
    ui_->brightnessSlider->setRange(min, max);
    ui_->brightnessSlider->setValue(cmanager_->getCamera()->getBrightness());
    ui_->brightnessSpinbox->setRange(min, max);
    ui_->brightnessSpinbox->setValue(cmanager_->getCamera()->getBrightness());
}

// ----------------------------------------------------------------------

void Squid::makeConnections() {

    // CAMERAS
    // video control
    connect(ui_->camerasCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCamera(int)));
    connect(ui_->freerunRadioButton, SIGNAL(clicked()), this, SLOT(changeCameraMode()));
    connect(ui_->triggersRadioButton, SIGNAL(clicked()), this, SLOT(changeCameraMode()));
    connect(ui_->holdCameraButton, SIGNAL(clicked()), this, SLOT(holdCamera()));
    connect(ui_->resumeCameraButton, SIGNAL(clicked()), this, SLOT(resumeCamera()));
    // FPS
    connect(cmanager_->getCamera()->getFpsEvaluator(), SIGNAL(fpsUpdated(const float)), this, SLOT(updateFps(const float)));
    // camera parameters
    connect(ui_->gainSlider, SIGNAL(valueChanged(int)), ui_->gainSpinbox, SLOT(setValue(int)));
    connect(ui_->stdShutterSlider, SIGNAL(valueChanged(int)), ui_->stdShutterSpinbox, SLOT(setValue(int)));
    connect(ui_->brightnessSlider, SIGNAL(valueChanged(int)), ui_->brightnessSpinbox, SLOT(setValue(int)));
    connect(ui_->gainSpinbox, SIGNAL(valueChanged(int)), ui_->gainSlider, SLOT(setValue(int)));
    connect(ui_->stdShutterSpinbox, SIGNAL(valueChanged(int)), ui_->stdShutterSlider, SLOT(setValue(int)));
    connect(ui_->brightnessSpinbox, SIGNAL(valueChanged(int)), ui_->brightnessSlider, SLOT(setValue(int)));

    // PORT PLAYER
    connect(ui_->manageControllerButton, SIGNAL(clicked()), SquidPlayer::getInstance(), SLOT(show()));

    // EXPERIMENT
    connect(ui_->manualDurationRadioButton, SIGNAL(clicked()), this, SLOT(updateGui()));
    connect(ui_->specifiedDurationRadioButton, SIGNAL(clicked()), this, SLOT(updateGui()));
    connect(ui_->sequenceDoneRadioButton, SIGNAL(clicked()), this, SLOT(updateGui()));

    // OUTPUT
    connect(ui_->browseButton, SIGNAL(clicked()), this, SLOT(setOutputDirectory()));

    // MENU
    // file menu
    connect(ui_->openAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui_->reloadAction, SIGNAL(triggered()), this, SLOT(reloadSettings()));
    connect(ui_->saveAction, SIGNAL(triggered()), this, SLOT(saveSettings()));
    connect(ui_->saveAsAction, SIGNAL(triggered()), this, SLOT(saveAsSettings()));
    connect(ui_->exitAction, SIGNAL(triggered()), this, SLOT(close()));
    // camera menu (now made right after building the menus)
    // help menu
    connect(ui_->aboutAction, SIGNAL(triggered()), this, SLOT(displayAbout()));
    connect(ui_->emailAddressAction, SIGNAL(triggered()), this, SLOT(setEmailAddress()));

    connect(ui_->runCameraButton, SIGNAL(toggled(bool)), this, SLOT(runCamera(bool)));
    connect(ui_->runExperimentButton, SIGNAL(clicked()), this, SLOT(runExperiment()));
}

// ----------------------------------------------------------------------

void Squid::displayAoiDialog() {

    try {
        // show a dialog to set AOI parameters
        AoiDialog dialog;
        int value = dialog.exec();
        QList<QAction*> list = resolutionActionGroup_->actions();
        const unsigned int n = list.size();
        for (unsigned int i = 0; i < n; i++) {
            if (list.at(i)->isChecked()) {
                aoiAction_->setChecked(value);
                list.at(i)->setChecked(!value);
                return;
            }
        }
        aoiAction_->setChecked(true);

    } catch (MyException* e) {
        LOG(WARNING) << "Unable to set AOI: " << e->getMessage();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to set AOI.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

// ----------------------------------------------------------------------

void Squid::importSettings() throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();

    // first, we configure the cameras
    CameraConfiguration::configureAllCameras(cmanager_, settings->getCameraConfigurations());

    // try to find the camera specified in the settings among the available cameras
    Dc1394Camera* camera = cmanager_->getCamera(settings->getCameraGuid());
    int defaultCameraIndex = 0;

    if (camera != NULL) {
        LOG(INFO) << "Selecting camera " << camera->getCameraName() << " (" << camera->getCameraGuid() << ") found in settings.";
        defaultCameraIndex = cmanager_->getCameraIndex(camera->getCameraGuid());
    }

    // set trigger
    ui_->triggerPeriodSpinBox->setValue(settings->getTriggerPeriod());
    cmanager_->getTriggerManager()->setIntervalInUs(1000 * settings->getTriggerPeriod()); // in us

    // WARNING: don't forget to call Dc1394Camera::setupCamera() after having modifying camera settings
    // (included in Squid::changeCamera())
    changeCamera(defaultCameraIndex);

//    // set parallel port controller
//    qportplayer::Global* global = qportplayer::Global::getInstance();
//    std::string globalFilename = settings->getPlayerSettingsFilename();
//    if (globalFilename.length() > 0)
//        global->load(globalFilename);

    portplayer::ParallelPortManager* ppManager = NULL;
    SquidPlayer* player = SquidPlayer::getInstance();

    // If no settings are given to configure the port player, let's create a default paralllel port player
    std::string portPlayerFilename = settings->getPlayerSettingsFilename();
    if (portPlayerFilename.empty()) {
        ppManager = portplayer::ParallelPortManager::generateParallelPortExample();
        player->setPortManager(ppManager);
    } else {
        player->loadGlobal(portPlayerFilename);
    }

    // at this point, experiment instance is NULL
    // -> directly initialize the GUI components controlling experiments
    ui_->experimentNameEdit->setText(settings->getExperimentName().c_str());
    ui_->experimentDescriptionEdit->clear();

    // set experiment duration mode
    if (settings->getExperimentDurationMode() == Experiment::MANUAL)
        ui_->manualDurationRadioButton->setChecked(true);
    else if (settings->getExperimentDurationMode() == Experiment::FIXED)
        ui_->specifiedDurationRadioButton->setChecked(true);
    else if (settings->getExperimentDurationMode() == Experiment::PLAYER)
        ui_->sequenceDoneRadioButton->setChecked(true);

    ui_->durationSpinbox->setValue(settings->getExperimentDuration());
    ui_->experimentSendEmail->setChecked(settings->getExperimentEmail());

    ui_->workingDirectoryEdit->setText(QString(settings->getWorkingDirectory().c_str()));
}

// ----------------------------------------------------------------------

void Squid::exportSettings() throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();

    // GENERAL
    settings->setWorkingDirectory(ui_->workingDirectoryEdit->text().toStdString());

    // CAMERAS
    std::string allCameraConfigurations = CameraConfiguration::getAllCameraConfigurations(cmanager_);
    settings->setCameraConfigurations(allCameraConfigurations);
    settings->setCameraGuid(cmanager_->getCamera()->getCameraGuid());
    settings->setTriggerPeriod(ui_->triggerPeriodSpinBox->value());

    // EXPERIMENTS
    settings->setExperimentName(ui_->experimentNameEdit->text().toStdString());
    settings->setOutputFormat(ui_->outputFormat->currentIndex());

    if (ui_->manualDurationRadioButton->isChecked())
        settings->setExperimentDurationMode(0);
    else if (ui_->specifiedDurationRadioButton->isChecked())
        settings->setExperimentDurationMode(1);
    else if (ui_->sequenceDoneRadioButton->isChecked())
        settings->setExperimentDurationMode(2);
    else
        throw new MyException("Undefined experiment duration mode.");

    settings->setExperimentDuration(ui_->durationSpinbox->value());
    settings->setExperimentEmail(ui_->experimentSendEmail->isChecked());
}

// ----------------------------------------------------------------------

Squid::Squid(int argc, char* argv[], QWidget* parent) : QMainWindow(parent), ui_(new Ui::Squid) {

    try {
        SquidSettings* settings = SquidSettings::getInstance();
        settings->configureLogging();
        settings->setSquid(this);

//        try {
//            settings->parseArguments(argc, argv);
//        } catch (MyException* e) {
//            LOG(WARNING) << "Unable to open or parse settings file: " << e->getMessage();
//            QMessageBox msgBox;
//            msgBox.setWindowTitle("sQuid message");
//            msgBox.setText("Unable to open or parse settings file.");
//            msgBox.setInformativeText((char*) e->what());
//            msgBox.setIcon(QMessageBox::Warning);
//            msgBox.exec();
//        }

        LOG(INFO) << "Running sQuid " << SQUID_VERSION << " (" << SQUID_VERSION_DATE << ")" << ".";
        ui_->setupUi(this);
        initialize();

        try {
            settings->parseArguments(argc, argv);
            importSettings();

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to open or parse settings file: " << e->getMessage();
            QMessageBox msgBox;
            msgBox.setWindowTitle("sQuid message");
            msgBox.setText("Unable to open or parse settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        updateGui();

    } catch (MyException* e) {
        LOG(ERROR) << "Unable to instantiate sQuid: " << e->getMessage();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to instantiate sQuid.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        exit(1);
    }
}

// ----------------------------------------------------------------------

Squid::~Squid() {

    delete ui_;
    delete cmanager_;
    delete SquidPlayer::getInstance();
    delete experiment_;
    delete resolutionActionGroup_;
    delete fpsActionGroup_;
    delete SquidSettings::getInstance();

    ui_ = NULL;
    cmanager_ = NULL;
    experiment_ = NULL;
    resolutionActionGroup_ = NULL;
    fpsActionGroup_ = NULL;

    LOG(INFO) << std::endl;
    LOG(INFO) << "Project website: http://tschaffter.ch/projects/squid/" << std::endl;
    LOG(INFO) << "Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)" << std::endl;
}

// ----------------------------------------------------------------------

void Squid::setLocationOnDisplay() {

    // place the main window of sQuid on the top-right corner of the display
    QDesktopWidget *desktop = QApplication::desktop();
    int x = desktop->width() - this->size().width();
    int y = 0;
    // move window to desired coordinates
    this->move(x, y);
}

// ----------------------------------------------------------------------

void Squid::updateGui() {

    // CAMERAS
    // Enable camera control only in FREERUN mode
    CameraManager::cameraMode mode = cmanager_->getCameraMode();
    bool b = (mode == CameraManager::FREERUN);
    ui_->gainLabel->setEnabled(b);
    ui_->gainSlider->setEnabled(b);
    ui_->gainSpinbox->setEnabled(b);
    ui_->stdShutterLabel->setEnabled(b);
    ui_->stdShutterSlider->setEnabled(b);
    ui_->stdShutterSpinbox->setEnabled(b);
    ui_->brightnessLabel->setEnabled(b);
    ui_->brightnessSlider->setEnabled(b);
    ui_->brightnessSpinbox->setEnabled(b);
    ui_->triggerPeriodSpinBox->setEnabled(!b);

    // don't allow the user to change the camera when the camera is running
    ui_->camerasCombobox->setEnabled(!cmanager_->isRunning());
    ui_->camerasCombobox->setCurrentIndex(cmanager_->getCameraIndex());

    ui_->cameraModeGroupbox->setEnabled(!cmanager_->isRunning());

    if (!cmanager_->isRunning()) {
        ui_->resumeCameraButton->setEnabled(false);
        ui_->holdCameraButton->setEnabled(false);
    } else {
        // resume and hold buttons handled in their respective functions
    }

    ui_->displayAllButton->setEnabled(cmanager_->isRunning());
    ui_->hideAllButton->setEnabled(cmanager_->isRunning());

    if (!cmanager_->isRunning())
        ui_->fpsLabel->setText(QString("FPS: 0.00"));

    // EXPERIMENT
    ui_->durationSpinbox->setEnabled(ui_->specifiedDurationRadioButton->isChecked());
    ui_->experimentNameEdit->setEnabled(!(experiment_ != NULL && experiment_->isRunning()));
    ui_->durationGroupbox->setEnabled(!(experiment_ != NULL && experiment_->isRunning()));
    ui_->outputFormat->setEnabled(!(experiment_ != NULL && experiment_->isRunning()));
    // experiment run and stop buttons handled in their respective functions

    ui_->manageControllerButton->setEnabled(ui_->sequenceDoneRadioButton->isChecked());
}

// ----------------------------------------------------------------------

bool Squid::changeCameraResolution(QAction* action) {

    if (cmanager_->isRunning()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to set camera resolution.");
        msgBox.setInformativeText("Setting resolution while the camera is running is not supported.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        LOG(INFO) << "Setting resolution while the camera is running is not supported.";
        return false;
    }

    try {
        if (action->text().compare("Set AOI...") == 0) {
            displayAoiDialog();
            return true;
        }
        else
            aoiAction_->setChecked(false);

        // sets new resolution
        Dc1394Camera* camera = cmanager_->getCamera();
        camera->setResolution( stringToDc1394Resolution(action->text().toStdString()) );

        // requires to update framerate sub-menu (dependent of the new resolution)
        QAction* a = NULL;
        std::string str = "";
        std::vector<std::string> fpsLabels = getSupportedFpsLabels(camera->getCamera(), camera->getResolution());
        std::string currentFramerate = dc1394ToStringFps(camera->getFps());
        unsigned int size = fpsLabels.size();
        ui_->framerateAction->menu()->clear();
        for (unsigned int i = 0; i < size; i++) {
            str = fpsLabels.at(i);
            a = new QAction(str.c_str(), this);
            a->setCheckable(true);
            a->setChecked( strcmp(currentFramerate.c_str(), str.c_str()) == 0 );

            fpsActionGroup_->addAction(a);
            ui_->framerateAction->menu()->addAction(a);
        }
    } catch (MyException* e) {
        LOG(WARNING) << "Unable to set camera resolution: " << e->getMessage();
    }
    return true;
}

// ----------------------------------------------------------------------

bool Squid::changeCameraFramerate(QAction* action) {

    if (cmanager_->isRunning()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to set camera framerate.");
        msgBox.setInformativeText("Settings framerate while the camera is running is not supported.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        LOG(WARNING) << "Settings framerate while the camera is running is not supported.";
        return false;
    }

    try {
        Dc1394Camera* camera = cmanager_->getCamera();
        camera->setFps( stringToDc1394Fps(action->text().toStdString()) );
    } catch (MyException* e) {
        LOG(WARNING) << "Unable to set camera framerate: " << e->getMessage();
    }
    return true;
}

// ----------------------------------------------------------------------

void Squid::changeCamera(int cameraIndex) throw(MyException*) {

    // remove the connection from the current camera
    disconnect(ui_->gainSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setGain(int)));
    disconnect(ui_->stdShutterSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setStdShutter(int)));
    disconnect(ui_->brightnessSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setBrightness(int)));
    disconnect(cmanager_->getCamera()->getFpsEvaluator());
    // select the camera affected by the new settings
    cmanager_->setCameraIndex(cameraIndex);
    cmanager_->setupActiveCameras();
    // load the new parameter
    updateCameraControllers();
    // setup new connections with the new camera
    connect(ui_->gainSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setGain(int)));
    connect(ui_->stdShutterSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setStdShutter(int)));
    connect(ui_->brightnessSlider, SIGNAL(valueChanged(int)), cmanager_->getCamera(), SLOT(setBrightness(int)));
    connect(cmanager_->getCamera()->getFpsEvaluator(), SIGNAL(fpsUpdated(const float)), this, SLOT(updateFps(const float)));
    // update the dynamic content of menu "camera"
    buildCameraMenu();
    updateGui();
}

// ----------------------------------------------------------------------

void Squid::changeCameraMode() throw(MyException*) {

    if (ui_->freerunRadioButton->isChecked())
        cmanager_->setCameraMode(CameraManager::FREERUN);
    else if (ui_->triggersRadioButton->isChecked())
        cmanager_->setCameraMode(CameraManager::SOFTWARE_TRIGGERS);
    else
        throw new MyException("Squid::changeCameraMode(): Invalid camera mode");

    cmanager_->setupActiveCameras();
    updateGui();
}

// ----------------------------------------------------------------------

void Squid::runCamera(bool run) {

    if (cmanager_->isRunning() && run) {
        LOG (WARNING) << "Camera is already running.";
        return;
    } else if (!cmanager_->isRunning() && !run) {
        LOG (WARNING) << "Camera is already not running.";
        return;
    }

    if (run) {
        ui_->resumeCameraButton->setEnabled(false);
        cmanager_->setAllCamerasActive(); // TODO: in a next version we can imaging to select specific set of cameras
        //cmanager_->setCameraActive("a47011109a1ec");

        // create the displays and show them
        std::vector<std::string> list;
        for (unsigned int i = 0; i < cmanager_->getNumActiveCameras(); i++)
            list.push_back(cmanager_->getActiveCamera(i)->getCameraNameAndGuid());

        LOG (INFO) << "Starting display manager for " << cmanager_->getNumActiveCameras() << " camera(s).";
        dmanager_ = new DisplayManager(list);
        connect(cmanager_, SIGNAL(frameCaptured(dc1394video_frame_t*, unsigned int, unsigned int, bool)), dmanager_, SLOT(displayFrame(dc1394video_frame_t*, unsigned int, unsigned int, bool)));
        connect(ui_->displayAllButton, SIGNAL(clicked()), dmanager_, SLOT(displayAll()));
        connect(ui_->hideAllButton, SIGNAL(clicked()), dmanager_, SLOT(hideAll()));

        // initialization
        cmanager_->setRestart(true);
        cmanager_->setAbort(false);
        cmanager_->reset();

        // useful only in trigger mode
        cmanager_->getTriggerManager()->setIntervalInUs(1000 * ui_->triggerPeriodSpinBox->value());

        // start camera
        cmanager_->start(QThread::HighPriority);
        ui_->holdCameraButton->setEnabled(true);

    } else {
        stopCameras();
    }

    updateGui();
}

// ----------------------------------------------------------------------

void Squid::stopCameras() {

    if (!cmanager_->isRunning()) {
        LOG (WARNING) << "Camera is already not running.";
        return;
    }

    // we must ensure that the thread is not sleeping,
    // otherwise stop it would have no effect!
    cmanager_->setRestart(true);
    cmanager_->wake();
    cmanager_->stop();
    cmanager_->wait(); // wait until the thread stops

    if (dmanager_ != NULL) {
        disconnect(ui_->displayAllButton);
        disconnect(ui_->displayAllButton);
        disconnect(dmanager_);

        delete dmanager_;
        dmanager_ = NULL;
    }
    cmanager_->setAllCamerasPassive();
}

// ----------------------------------------------------------------------

void Squid::updateFps(const float fps) {

    std::ostringstream buffer;
    buffer.setf(std::ios::fixed, std::ios::floatfield); // so that .setprecision(3) always print three decimals
    buffer << "FPS: ";
    buffer << std::setprecision(2) << fps;
    ui_->fpsLabel->setText(buffer.str().c_str());
}

// ----------------------------------------------------------------------

/** This method is not used as a slot but is provided as a function pointer, thus it is executed immediately. */
void Squid::playerStateChanged(const unsigned int currentState) {

    SquidPlayer* player = SquidPlayer::getInstance();
    // set the suffix of the frame names, composed of the name of the active pins
    Experiment* experiment = SquidSettings::getInstance()->getSquid()->getExperiment();
    if (experiment != NULL)
        experiment->setFrameSuffix(player->getStateKeys(currentState));
    // specify if the frames must be saved since now on
    CameraManager::getInstance()->setSaveFrame(player->getSave(currentState));
}

// ----------------------------------------------------------------------

void Squid::initializeExperiment() {

    if (experiment_ != NULL && experiment_->isRunning())
        throw new MyException("The experiment " + experiment_->getName() + " is still running. First stop this experiment before starting a new one.");

    experiment_ = new Experiment();
    experiment_->setName(ui_->experimentNameEdit->text().toStdString());

    // now we create as many sub-experiments as number of cameras
    std::vector<std::string> list;
    for (unsigned int i = 0; i < (unsigned int)cmanager_->getNumCameras(); i++)
        list.push_back(cmanager_->getCamera(i)->getCameraGuid());
    experiment_->setSubExperimentIds(list);
    experiment_->setOutputFormat(ui_->outputFormat->currentIndex()); // image format
    experiment_->setWorkingDirectory(ui_->workingDirectoryEdit->text().toStdString());

    // set the experiment duration mode, either MANUAL (user must click on the Stop button to stop experiment)
    // or SPECIFIED (the experiment stops when the specified time is elapsed)
    SquidPlayer* player = SquidPlayer::getInstance();
    experimentProgressBar_.setTimeInMs(0);
    if (ui_->manualDurationRadioButton->isChecked()) {
        experiment_->setDurationMode(Experiment::MANUAL);
        experiment_->setDurationInUs(-1);
        experiment_->setSaveFirstFrames(true); // setSaveFirstFrames() defined to not start saving right now
        experimentProgressBar_.setMaxDurationInMs(0);
        experimentProgressBar_.setEnabled(false);
    }
    else if (ui_->specifiedDurationRadioButton->isChecked()) {
        experiment_->setDurationMode(Experiment::FIXED);
        LOG(INFO) << "TODO: re-take into account the experiment duration instead of only 5 seconds" << std::endl;
        unsigned int durationUs = ui_->durationSpinbox->value() * 60 * 1000 * 1000; //ui_->durationSpinbox->value() * 60 * 1000 * 1000;
        experiment_->setDurationInUs(durationUs); // set duration time in us (was s)
        experiment_->setSaveFirstFrames(true); // setSaveFirstFrames() defined to not start saving right now
        experimentProgressBar_.setMaxDurationInMs(durationUs/1000);
        experimentProgressBar_.setEnabled(true);
    }
    else if (ui_->sequenceDoneRadioButton->isChecked()) {
        unsigned int playlistDuration = player->getPortManager()->getPinPlaylist()->getPlaylistTotalTime();
        experiment_->setDurationMode(Experiment::PLAYER);
        experiment_->setDurationInUs(playlistDuration * 1000);
        experiment_->setSaveFirstFrames(player->getSave(0)); // setSaveFirstFrames() defined to not start saving right now
        experiment_->setFrameSuffix(SquidPlayer::getInstance()->getStateKeys(0));
        experimentProgressBar_.setMaxDurationInMs(playlistDuration);
        experimentProgressBar_.setEnabled(true);
    }

    // allow to change the variable save_ of experiment when the selected state in the playlist changed
    player->setPostNextStateAction((qportplayer::pfv) &Squid::playerStateChanged);

    // create required directories
    experiment_->setup();

    // make connections
    connect(ui_->stopExperimentButton, SIGNAL(clicked()), experiment_, SLOT(stop()));
    connect(experiment_, SIGNAL(finished()), this, SLOT(stopExperiment()));
    connect(experiment_, SIGNAL(timeElapsedInUs(unsigned int)), &experimentProgressBar_, SLOT(setTimeInUs(unsigned int)));
    connect(cmanager_, SIGNAL(frameCaptured(dc1394video_frame_t*, unsigned int, unsigned int, bool)), this, SLOT(saveFrame(dc1394video_frame_t*, unsigned int, unsigned int, bool)));
}

// ----------------------------------------------------------------------

void Squid::runExperiment() {

    try {
        LOG (INFO) << "Running experiment " << ui_->experimentNameEdit->text().toStdString() << ".";
        if (experiment_ != NULL && experiment_->isRunning())
            throw new MyException("The experiment " + experiment_->getName() + " is still running. First stop this experiment before starting a new one.");

        if (experiment_ != NULL) {
            experiment_->disconnect();
            disconnect(cmanager_, SIGNAL(frameCaptured(dc1394video_frame_t*, unsigned int, unsigned int, bool)), this, SLOT(saveFrame(dc1394video_frame_t*, unsigned int, unsigned int, bool)));
            experimentProgressBar_.disconnect();
//            disconnect(SquidPlayer::getInstance()->getPortManager()->getPinPlaylist(), SIGNAL(stateChanged(const unsigned int)), this, SLOT(playerStateChanged(const unsigned int)));
        }
        ui_->runExperimentButton->setEnabled(false);
        ui_->stopExperimentButton->setEnabled(true);

        initializeExperiment();
        // start the player if 1) player is selected in the app and 2) playlist is selected in the player
        if (ui_->sequenceDoneRadioButton->isChecked() && SquidPlayer::getInstance()->getPortManager()->getMode() == portplayer::IOPinManager::PLAYLIST)
            SquidPlayer::getInstance()->startPlaylist();
        experiment_->start();

    } catch (boost::filesystem::filesystem_error& e) {
        LOG(INFO) << "Unable to run experiment: " << e.what();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Directory creation failed.");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        
        try {
            delete experiment_; // destructor -> mutex_.lock() fails
            experiment_ = NULL;
        } catch (std::exception* e) {
            // do nothing
        }
        ui_->runExperimentButton->setEnabled(true);

    } catch (MyException* e) {
        LOG(ERROR) << "Unable to run experiment: " << e->getMessage();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to run experiment.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        
        try {
            delete experiment_; // destructor -> mutex_.lock() fails
            experiment_ = NULL;
        } catch (std::exception* e) {
            // do nothing
        }
        ui_->runExperimentButton->setEnabled(true);
    }

    updateGui();
}

// ----------------------------------------------------------------------

void Squid::stopExperiment() {

    std::string errors = "";

    try {
        LOG (INFO) << "Stopping experiment." << std::endl;

        if (!(experiment_ != NULL && !experiment_->isRunning()))
            throw new MyException("There is no experiment running.");

        // safety
        LOG(INFO) << "Turning off player.";
        SquidPlayer::getInstance()->turnOff();
        SquidPlayer::getInstance()->setPostNextStateAction((qportplayer::pfv) &qportplayer::QPortPlayerDialog::defaultPostNextStateAction);

        // write report to file
        std::string reportContent = "";
        try {
            LOG(INFO) << "Writing report to " << experiment_->getFolder() << "/" << REPORT_FILENAME;
            experiment_->setDescription(ui_->experimentDescriptionEdit->toPlainText().toStdString());
            reportContent = experiment_->saveDescription();
        } catch (std::exception* e) {
            LOG(WARNING) << "Unable to save experiment report: " << e->what();
            errors += "Unable to save report\n";
        }

        // send report to email addresses
        SquidSettings* settings = SquidSettings::getInstance();
        try {
            if (ui_->experimentSendEmail->isChecked() && settings->getEmailAddress().length() > 0) {
                LOG(INFO) << "Sending report to " << settings->getEmailAddress();
                std::string subject = settings->getExperimentEmailSubjectPrefix() + " (" + experiment_->getName() + ")";
                if (!google::SendEmail(settings->getEmailAddress().c_str(), subject.c_str(), reportContent.c_str()))
                    LOG(WARNING) << "Unable to send experiment report by email using google::SendEmail().";
                LOG(INFO) << "Check your spambox if the email doesn't appear in the inbox.";
            }
        } catch (std::exception* e) {
            LOG(WARNING) << "Unable to send experiment report by email.";
            errors += "Unable to send report by email\n";
        }

        std::string playerSettingsFilename = "";
        try {
            // save player settings file
            playerSettingsFilename = experiment_->getExperimentFolder() + "/settings_player.txt";
            LOG(INFO) << "Writing player settings to " << playerSettingsFilename;
            SquidPlayer::getInstance()->exportSettings(); // save to Global
            qportplayer::Global::getInstance()->save(playerSettingsFilename);
        } catch (std::exception* e) {
            LOG(WARNING) << "Unable to save player settings file.";
            errors += "Unable to save player settings file\n";
        }

        // save settings file
        try {
            std::string filename = "settings.txt";
            std::string settingsFilename = experiment_->getExperimentFolder() + "/" + filename;
            LOG(INFO) << "Writing sQuid settings to " << settingsFilename;
            // before settings the setting file, we set the filename of the player settings file
            std::string playerSettingsFilenameBkp = settings->getPlayerSettingsFilename();
            settings->setPlayerSettingsFilename(playerSettingsFilename);
            settings->save(settingsFilename);
            settings->setPlayerSettingsFilename(playerSettingsFilenameBkp);
        } catch (std::exception* e) {
            LOG(WARNING) << "Unable to save sQuid settings file.";
            errors += "Unable to save sQuid settings file";
        }

        ui_->stopExperimentButton->setEnabled(false);
        ui_->runExperimentButton->setEnabled(true);

    } catch (MyException* e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to stop experiment.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        LOG(ERROR) << "Unable to stop experiment: " << e->getMessage();
    }

    if (errors.length() > 0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Issues:");
        msgBox.setInformativeText(errors.c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    updateGui();
}

// ----------------------------------------------------------------------

void Squid::saveFrame(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int us, bool saveFrame) {

    if (experiment_ != NULL && saveFrame)
        experiment_->saveFrame(frame, cameraIndex, us);
}

// ----------------------------------------------------------------------

void Squid::closeEvent(QCloseEvent* event) {

    if (fineToExit()) {
        // Close all the camera displays here (if any). This can not been done in
        // the destructor since it is only called when all the windows are closed...
        delete dmanager_;
        dmanager_ = NULL;
        SquidPlayer::getInstance()->close();
        event->accept();
    }
    else
        event->ignore();
}

// ----------------------------------------------------------------------

bool Squid::fineToExit() {

    std::string msg = "";

    if (experiment_ != NULL && experiment_->isRunning())
        msg = "The experiment is still running.";
    else if (cmanager_->isRunning())
        msg = "The camera is still running.";

    if (msg != "") {
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setWindowIcon(SquidSettings::getInstance()->getApplicationWindowIcon());
        msgBox.setText(msg.c_str());
        msgBox.setInformativeText("Exit sQuid ?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIconPixmap(QPixmap::fromImage(QImage(":/exit")));
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Ok:
            return true;
            break;
        case QMessageBox::Cancel:
            return false;
            break;
        default:
            // should never be reached
            break;
        }
    }
    return true;
}

// ----------------------------------------------------------------------

void Squid::holdCamera() {

    ui_->holdCameraButton->setEnabled(false);
    cmanager_->setRestart(false);
    ui_->resumeCameraButton->setEnabled(true);

    updateGui();
}

// ----------------------------------------------------------------------

void Squid::resumeCamera() {

    ui_->resumeCameraButton->setEnabled(false);
    cmanager_->setRestart(true);
    cmanager_->wake();
    ui_->holdCameraButton->setEnabled(true);

    updateGui();
}

// ----------------------------------------------------------------------

void Squid::setOutputDirectory() {

    SquidSettings *settings = SquidSettings::getInstance();
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(settings->getWorkingDirectory().c_str()));
    if (!directory.isEmpty()) {
        settings->setWorkingDirectory(directory.toStdString());
        ui_->workingDirectoryEdit->setText(directory);
    }
 }

// ----------------------------------------------------------------------

void Squid::openSettings() {

    SquidSettings* settings = SquidSettings::getInstance();
    QString file = QFileDialog::getOpenFileName(this, tr("Open"), QString(settings->getWorkingDirectory().c_str()));
    if (!file.isEmpty()) {
        try {
            LOG(INFO) << "Opening settings file " << file.toStdString();
            settings->load(file.toStdString());
            importSettings();
            updateCameraControllers();
            updateGui();

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to open settings file: " << e->what();
            QMessageBox msgBox;
            msgBox.setWindowTitle("sQuid message");
            msgBox.setText("Unable to open settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}

// ----------------------------------------------------------------------

void Squid::reloadSettings() {

    try {
        SquidSettings* settings = SquidSettings::getInstance();
        LOG(INFO) << "Reloading settings file " << settings->getSettingsFile();
        settings->load(settings->getSettingsFile());

        importSettings();
        updateCameraControllers();
        updateGui();

    } catch (MyException* e) {
        LOG(WARNING) << "Unable to reload settings file: " << e->what();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to reload settings file.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

// ----------------------------------------------------------------------

void Squid::saveSettings() {

    SquidSettings* settings = SquidSettings::getInstance();
    try {
        if (settings->getSettingsFile().empty())
            saveAsSettings();
        else {
            LOG(INFO) << "Writing settings file " << settings->getSettingsFile();
            exportSettings();
            settings->save(settings->getSettingsFile());
        }
    } catch (MyException* e) {
        LOG(WARNING) << "Unable to write settings file: " << e->what();
        QMessageBox msgBox;
        msgBox.setText("Unable to save settings file.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

// ----------------------------------------------------------------------

void Squid::saveAsSettings() {

    SquidSettings *settings = SquidSettings::getInstance();
    QString file = QFileDialog::getSaveFileName(this, tr("Save As"), QString(settings->getWorkingDirectory().c_str()));
    if (!file.isEmpty()) {
        settings->setSettingsFile(file.toStdString());
        saveSettings();
    }
}

// ----------------------------------------------------------------------

void Squid::displayAbout() {

    About a(this);
    a.exec();
}

// ----------------------------------------------------------------------

void Squid::setEmailAddress() {

    SquidSettings* settings = SquidSettings::getInstance();
    bool ok;

    QString text = QInputDialog::getText(this, tr("Set Emails"),
        tr("<html>List of comma-separated email addresses:</html>"), QLineEdit::Normal,
        QString(settings->getEmailAddress().c_str()), &ok);
    if (ok && !text.isEmpty()) {
        settings->setEmailAddress(text.toStdString());
        settings->configureEmailLogging();
        LOG(INFO) << "Setting list of comma-separated email addresses to " << text.toStdString() << ".";
    }
}

// ----------------------------------------------------------------------

void Squid::playerEnabled(bool enable) {

    if (!enable)
        SquidPlayer::getInstance()->turnOff();
}

// ======================================================================
// GETTERS AND SETTERS

Ui::Squid* Squid::getUi() { return ui_; }

CameraManager* Squid::getCameraManager() { return cmanager_; }
Experiment* Squid::getExperiment() { return experiment_; }
