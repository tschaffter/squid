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

#include "squidplayer.h"
#include "ui_qportplayerdialog.h"
#include "global.h"
#include "squidsettings.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <glog/logging.h>

using namespace squid;
using namespace qsquid;

/** Singleton instance. */
static SquidPlayer* instance_ = NULL;

// ======================================================================
// PRIVATE METHODS

SquidPlayer::SquidPlayer(QWidget* parent) : qportplayer::QPortPlayerDialog(parent) {

    setWindowTitle("sQuid Player");
//    ui->tip->setText("Turn OFF or ON a camera can take between 200 and 500 ms");
    ui->externalTriggerRadioButton->setEnabled(false);
}

// ======================================================================
// PUBLIC METHODS

SquidPlayer* SquidPlayer::getInstance() {

    if (instance_ == NULL)
        instance_ = new SquidPlayer(NULL);

    return instance_;
}

// ----------------------------------------------------------------------

void SquidPlayer::show() {

    getInstance(); // the object may not have been created yet
    QPortPlayerDialog::show();
    // move window to desired coordinates
    this->move(0, 0);
}

// ----------------------------------------------------------------------

void SquidPlayer::setup() {

    setPinCells();
    setDurationCells();
    setSaveCells();
//    setCameraCells();
    setOutputLeds();
    setPlaylistPlayer();

    // get all the camera to fill the comboboxes
//    setCameras(Settings::getInstance()->getSquid()->getCameraManager());
    loadSaveAndCameraConfiguration(qportplayer::Global::getInstance()->getStates());

    ui->editionRadioButton->setChecked(true);

    const int unit = qportplayer::Global::getInstance()->getDurationUnit();
    if (unit == 0)
        ui->minRadioButton->setChecked(true);
    else if (unit == 1)
        ui->secRadioButton->setChecked(true);
    else if (unit == 2)
        ui->msecRadioButton->setChecked(true);

    // safety
    pManager_->setAllPinsLow();

    // adpat dialog size to number of columns
    const double customWidth = 1 * ui->tableWidget->columnCount();
    const QSize qs(customWidth, this->minimumHeight());
    this->resize(qs);
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    updateGui();
}

// ----------------------------------------------------------------------

void SquidPlayer::setPortManager(portplayer::IOPinManager* pManager) {

    delete pManager_;
    pManager_ = pManager;

    if (pManager_ != NULL) {
        initialize();
        setup();
        connect(ui->repeatSequenceCheckBox, SIGNAL(stateChanged(int)), pManager_->getPinPlaylist(), SLOT(repeatPlaylist(int)));
    }
    // select the playlist by default
    ui->sequenceRadioButton->click();
}

// ----------------------------------------------------------------------

void SquidPlayer::setSaveCells() {

    // add one column to the table
    ui->tableWidget->setColumnCount(ui->tableWidget->columnCount() + 1);

    // set header
    QTableWidgetItem* header = new QTableWidgetItem();
    header->setText("Save");
    const unsigned int n = ui->tableWidget->columnCount() - 1;
    const unsigned int m = ui->tableWidget->rowCount();
    ui->tableWidget->setHorizontalHeaderItem(n, header);

    // set content
    QCheckBox* cb = NULL;
    for (unsigned int i = 0; i < m; i++) {
        cb = new QCheckBox();
        cb->setChecked(true);
        createCellWidget(i, n, cb);
    }
}

// ----------------------------------------------------------------------

void SquidPlayer::setCameraCells() {

    // add one column to the table
    ui->tableWidget->setColumnCount(ui->tableWidget->columnCount() + 1);

    // set header
    QTableWidgetItem* header = new QTableWidgetItem();
    header->setText("Camera");
    const unsigned int n = ui->tableWidget->columnCount() - 1;
    ui->tableWidget->setHorizontalHeaderItem(n, header);

    QComboBox* cb = NULL;
    for (unsigned int i = 0; i < n; i++) {
        cb = new QComboBox();
        cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        createCellWidget(i, n, cb);
    }
}

// ----------------------------------------------------------------------

void SquidPlayer::setCameras(CameraManager* cameras) {

    QComboBox* combo = NULL;
    const unsigned int column = ui->tableWidget->columnCount() - 1;
    const unsigned int numRows = ui->tableWidget->rowCount();
    const unsigned int numCameras = cameras->getNumCameras();
    std::string cameraId;
    QIcon cameraIcon(":/camera");

    for (unsigned int i = 0; i < numRows; i++) {
        combo = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, column)->layout()->itemAt(0)->widget());
        for (unsigned int j = 0; j < numCameras; j++) {
            cameraId = cameras->getCamera(j)->getCameraName() + " (" + cameras->getCamera(j)->getCameraGuid() + ")";
            combo->addItem(cameraIcon, QString(cameraId.c_str()));
        }
        combo->setCurrentIndex(0);
    }
}

// ----------------------------------------------------------------------

void SquidPlayer::loadGlobal(std::string filename) {

    LOG(INFO) << "Opening settings file " << filename;
    qportplayer::Global::getInstance()->load(filename);
    this->importSettings();
    changeMode();
}

// --------------------------------------------------------------------

void SquidPlayer::loadGlobal() {

    QString file = QFileDialog::getOpenFileName(this, tr("Open Settings"));

    if (!file.isEmpty()) {
        try {
            loadGlobal(file.toStdString());

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to load port player settings file: " << e->what();
            QMessageBox msgBox;
            msgBox.setWindowTitle("sQuid message");
            msgBox.setText("Unable to load port player settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}

// ----------------------------------------------------------------------

void SquidPlayer::saveGlobal() {

    qportplayer::Global* global = qportplayer::Global::getInstance();
    QString file = QFileDialog::getSaveFileName(this, tr("Save Settings"));

    if (!file.isEmpty()) {
        try {
            LOG(INFO) << "Writing port player settings file " << global->getSettingsFile();
            exportSettings();
            global->setSettingsFile(file.toStdString());
            global->save(global->getSettingsFile());

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to write port player settings file: " << e->what();
            QMessageBox msgBox;
            msgBox.setWindowTitle("sQuid message");
            msgBox.setText("Unable to write port player settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}

// ----------------------------------------------------------------------

void SquidPlayer::importSettings() {

    qportplayer::Global* global = qportplayer::Global::getInstance();

    if (pManager_ == NULL)
        pManager_ = new portplayer::ParallelPortManager();

    pManager_->load(global->getPortDeviceAbsPath(), global->getPins());
    // because the sequence is recreated inside load(), we have to reconnect it.
    connect(ui->repeatSequenceCheckBox, SIGNAL(stateChanged(int)), pManager_->getPinPlaylist(), SLOT(repeatPlaylist(int)));

    std::string config = global->getStates();
    std::string playlist = "";
    std::string durations = "";
    getParallelPortConfiguration(config, pManager_->getNumPins(), playlist, durations);
    pManager_->getPinPlaylist()->loadPlaylist(playlist);
    pManager_->getPinPlaylist()->loadStateDurations(durations);

    setup();
}

// ----------------------------------------------------------------------

void SquidPlayer::exportSettings() {

    qportplayer::Global* global = qportplayer::Global::getInstance();
    //CameraManager* cmanager = Settings::getInstance()->getSquid()->getCameraManager();
    saveTableContent();

    global->setPins(pManager_->save());

    std::stringstream text;
    const unsigned int numStates = ui->tableWidget->rowCount();
    const unsigned int numPins = pManager_->getNumPins();
    for (unsigned int i = 0; i < numStates; i++) {
        for (unsigned int j = 0; j < numPins; j++) {
            QCheckBox* box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(i, j)->layout()->itemAt(0)->widget());
            text << box->isChecked();
            text << " ";
        }

        // duration
        QSpinBox* box = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(i, numPins)->layout()->itemAt(0)->widget());
        text << box->value();
        text << " ";

        // save
        QCheckBox* box2 = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(i, numPins + 1)->layout()->itemAt(0)->widget());
        text << box2->isChecked();
        text << " ";

//        // camera
//        QComboBox* combo = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(i, numPins + 2)->layout()->itemAt(0)->widget());
//        text << cmanager->getCamera(combo->currentIndex())->getCameraGuid();

        if (i < numStates - 1)
            text << ";";
    }
    global->setStates(text.str());

    if (ui->minRadioButton->isChecked())
        global->setDurationUnit(0);
    else if (ui->secRadioButton->isChecked())
        global->setDurationUnit(1);
    else if (ui->msecRadioButton)
        global->setDurationUnit(2);
}

// ----------------------------------------------------------------------

void SquidPlayer::loadSaveAndCameraConfiguration(const std::string config) {

    if (config.length() == 0)
        return;

    //CameraManager* cmanager = Settings::getInstance()->getSquid()->getCameraManager();
    const unsigned int numPins = pManager_->getNumPins();

    unsigned int value = 0;
    unsigned int row = 0;
    std::string buffer = "";
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = config.find(';', pos)) != std::string::npos) {
        std::string substring(config.substr(prev_pos, pos-prev_pos));
        std::stringstream ss(substring);
        for (unsigned int i = 0; i < numPins; i++)
            ss >> buffer;
        // duration
        ss >> buffer;

        // save checkbox
        QCheckBox* box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, numPins + 1)->layout()->itemAt(0)->widget());
        ss >> value;
        box->setChecked(value);

//        // camera
//        QComboBox* combo = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, numPins + 2)->layout()->itemAt(0)->widget());
//        ss >> buffer;
//        combo->setCurrentIndex(cmanager->getCameraIndex(buffer));

        prev_pos = ++pos;
        row++;
    }
    std::string substring(config.substr(prev_pos, pos-prev_pos)); // Last word
    std::stringstream ss(substring);
    for (unsigned int i = 0; i < numPins; i++)
        ss >> buffer;
    // duration
    ss >> buffer;

    // save checkbox
    QCheckBox* box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, numPins + 1)->layout()->itemAt(0)->widget());
    ss >> value;
    box->setChecked(value);

//    // camera
//    QComboBox* combo = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, numPins + 2)->layout()->itemAt(0)->widget());
//    ss >> buffer;
//    combo->setCurrentIndex(cmanager->getCameraIndex(buffer));
}

// ======================================================================
// GETTERS AND SETTERS

bool SquidPlayer::getSave(const unsigned int index) {

    const unsigned int saveColumn = ui->tableWidget->columnCount() - 1;
    QCheckBox* box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(index, saveColumn)->layout()->itemAt(0)->widget());
    return box->isChecked();
}
