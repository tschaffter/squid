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

#include "aoidialog.h"
#include "ui_aoidialog.h"
#include "myutility.h"
#include "squidsettings.h"
#include <vector>
#include <QMessageBox>
#include <QPushButton>
#include <glog/logging.h>

using namespace squid;
using namespace qsquid;

// ======================================================================
// PUBLIC METHODS

AoiDialog::AoiDialog(QWidget *parent) : QDialog(parent), ui_(new Ui::AoiDialog) {

    ui_->setupUi(this);
    setWindowTitle("Set AOI");

    initialize();
}

// ----------------------------------------------------------------------

AoiDialog::~AoiDialog() {

    delete ui_;
    ui_ = NULL;
}

// ----------------------------------------------------------------------

void AoiDialog::initialize() throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();

    // camera
    CameraManager* cmanager = settings->getSquid()->getCameraManager();
    /*const unsigned int numCameras = cmanager->getNumCameras();
    std::string cameraId;
    QIcon cameraIcon(":/camera");
    for (unsigned int i = 0; i < numCameras; i++)
    {
        cameraId = cmanager->getCamera(i)->getCameraName() + " (" + cmanager->getCamera(i)->getCameraGuid() + ")";
        ui_->camerasComboBox->addItem(cameraIcon, QString(cameraId.c_str()));
    }
    ui_->camerasComboBox->setCurrentIndex(cmanager->getCameraIndex());*/

    update(cmanager->getCamera());

//    // format7 video mode
//    const std::vector<std::string> strResolution = getSupportedResolutionsLabels(settings->getSquid()->getCameraManager()->getCamera()->getCamera());
//    const unsigned int n = strResolution.size();
//    std::string str = "";
//    for (unsigned int i = 0; i < n; i++)
//    {
//        str = strResolution.at(i);
//        if (str.find("FORMAT7") != str.npos)
//            ui_->format7ModeComboBox->addItem(str.c_str());
//    }
//    const std::string mode = ui_->format7ModeComboBox->currentText().toStdString();
//
//    Dc1394Camera* camera = settings->getSquid()->getCameraManager()->getCamera();
//    dc1394format7mode_t info;
//    dc1394_format7_get_mode_info(camera->getCamera(), stringToDc1394Resolution(mode), &info);
//
//    ui_->wMax->setText(QString(intToIntString(info.max_size_x).c_str()));
//    ui_->hMax->setText(QString(intToIntString(info.max_size_y).c_str()));
//    ui_->leftSpinBox->setRange(0, info.max_size_x);
//    ui_->topSpinBox->setRange(0, info.max_size_y);
//    ui_->widthSpinBox->setRange(0, info.max_size_x);
//    ui_->heightSpinBox->setRange(0, info.max_size_y);
//    info_ = &info;
//
//    // fill with values of current camera
//    changeCamera(cmanager->getCameraIndex());

    //connect(ui_->camerasComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCamera(int)));
    connect(ui_->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
}

// ----------------------------------------------------------------------

void AoiDialog::update(Dc1394Camera* camera) throw(MyException*) {

    SquidSettings* settings = SquidSettings::getInstance();

    // format7 video mode
    const std::vector<std::string> strResolution = getSupportedResolutionsLabels(settings->getSquid()->getCameraManager()->getCamera()->getCamera());
    const unsigned int n = strResolution.size();
    std::string str = "";
    for (unsigned int i = 0; i < n; i++) {
        str = strResolution.at(i);
        if (str.find("FORMAT7") != str.npos)
            ui_->format7ModeComboBox->addItem(str.c_str());
    }

    const std::string mode = ui_->format7ModeComboBox->currentText().toStdString();
    dc1394format7mode_t info;
    dc1394error_t err;
    if ((err = dc1394_format7_get_mode_info(camera->getCamera(), stringToDc1394Resolution(mode), &info)) != DC1394_SUCCESS)
        throw new MyException("Unable dc1394_format7_get_mode_info().");
    if (!info.present)
        throw new MyException("Format 7 not present.");

    LOG(INFO) << "plop";

    LOG (INFO) << "AOI contraints for " << mode << ".";
    LOG (INFO) << "(x + width) <= " << info.max_size_x;
    LOG (INFO) << "(y + height) <= " << info.max_size_y;
    info_ = &info;

//    LOG(INFO) << info.max_size_y;

    ui_->wMax->setText(QString(longToLongString((long) info.max_size_x).c_str()));
    ui_->hMax->setText(QString(longToLongString((long) info.max_size_y).c_str()));
    ui_->leftSpinBox->setRange(0, info.max_size_x-1);
    ui_->topSpinBox->setRange(0, info.max_size_y-1);
    ui_->widthSpinBox->setRange(4, info.max_size_x);
    ui_->heightSpinBox->setRange(4, info.max_size_y);

    Aoi* aoi = camera->getAoi();
    ui_->leftSpinBox->setValue(aoi->x_);
    ui_->topSpinBox->setValue(aoi->y_);
    ui_->widthSpinBox->setValue(aoi->width_);
    ui_->heightSpinBox->setValue(aoi->height_);
}

// ----------------------------------------------------------------------

void AoiDialog::changeCamera(int cameraIndex) {

    Dc1394Camera* camera = SquidSettings::getInstance()->getSquid()->getCameraManager()->getCamera(cameraIndex);
    update(camera);
}

// ----------------------------------------------------------------------

void AoiDialog::accept() {

    bool ok = apply();
    if (ok) {
        // make the camera use AOI and quit
        CameraManager* cmanager = SquidSettings::getInstance()->getSquid()->getCameraManager();
        Dc1394Camera* camera = cmanager->getCamera();//cmanager->getCamera(ui_->camerasComboBox->currentIndex());
        camera->setAoi(stringToDc1394Resolution(ui_->format7ModeComboBox->currentText().toStdString()), camera->getAoi());
        camera->useAoi(true);
        done(QMessageBox::Ok);
    }
}

// ----------------------------------------------------------------------

bool AoiDialog::apply() {

    try {
        SquidSettings* settings = SquidSettings::getInstance();

        const unsigned int left = ui_->leftSpinBox->value();
        const unsigned int top = ui_->topSpinBox->value();
        const unsigned int width = ui_->widthSpinBox->value();
        const unsigned int height = ui_->heightSpinBox->value();
        std::string error = "";

        if (width % 4 != 0)
            error = "Width must be a multiple of 4";
        else if (height % 4 != 0)
            error = "Height must be a multiple of 4";
        else if (left + width > info_->max_size_x)
            error = "left + width = " + intToIntString(left + width) + " > " + intToIntString(info_->max_size_x);
        else if (top + height > info_->max_size_y)
            error = "top + height = " + intToIntString(top + height) + " > " + intToIntString(info_->max_size_y);
        if (!error.empty())
            throw new MyException(error);

        CameraManager* cmanager = settings->getSquid()->getCameraManager();
        Dc1394Camera* camera = cmanager->getCamera();//cmanager->getCamera(ui_->camerasComboBox->currentIndex());
        Aoi* aoi = camera->getAoi();
        aoi->x_ = left;
        aoi->y_ = top;
        aoi->width_ = width;
        aoi->height_ = height;
        camera->setAoi(*aoi);

        LOG (INFO) << "AOI saved (but not set).";

        return true;

    } catch (MyException* e) {
        LOG (WARNING) << "Unable to set AOI: " << e->getMessage();
        QMessageBox msgBox;
        msgBox.setWindowTitle("sQuid message");
        msgBox.setText("Unable to set AOI.");
        msgBox.setInformativeText((char*) e->what());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    return false;
}
