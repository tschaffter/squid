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

#include "cameradisplay.h"
#include "ui_cameradisplay.h"
#include "squidsettings.h"
#include <iostream>
#include <QVBoxLayout>
#include <glog/logging.h>

using namespace squid;
using namespace qsquid;

// ======================================================================
// PUBLIC METHODS

void CameraDisplay::closeEvent(QCloseEvent*) {

    emit displayClosed(false);
}

 // ----------------------------------------------------------------------

CameraDisplay::CameraDisplay(QWidget* parent) : QDialog(parent), ui_(new Ui::CameraDisplay) {

    ui_->setupUi(this);
    setWindowIcon(SquidSettings::getInstance()->getApplicationWindowIcon());

    frameLabel_ = NULL;
    framePixmap_ = NULL;
    frameImage_ = NULL;
    frameSize_ = NULL;
    chArr_ = NULL;
    wh_ = 0;

    frameLabel_ = new QLabel();
    frameLabel_->setBackgroundRole(QPalette::Base);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    frameLabel_->setScaledContents(true);

     QVBoxLayout* layout = new QVBoxLayout;
     layout->addWidget(frameLabel_);
     layout->setContentsMargins(0, 0, 0, 0);
     setLayout(layout);
}

// ----------------------------------------------------------------------

CameraDisplay::~CameraDisplay() {

    delete ui_;
    delete frameLabel_;
    delete framePixmap_;
    delete frameImage_;
    delete frameSize_;
    // DO NOT DELETE chArr_

    ui_ = NULL;
    frameLabel_ = NULL;
    framePixmap_ = NULL;
    frameImage_ = NULL;
    frameSize_ = NULL;
    chArr_ = NULL;
}

// ----------------------------------------------------------------------

void CameraDisplay::displayFrame(dc1394video_frame_t* frame) {

    if (frameImage_ == NULL) {
        frameSize_ = new QSize((int)frame->size[0], (int)frame->size[1]);
        frameImage_ = new QImage(*frameSize_, QImage::Format_Indexed8);

        // Setup grayscale image (MONO8)
        // TODO: handle MONO16 and RGB images
        frameImage_->setNumColors(256);
        for (unsigned int i = 0; i < 256; i++)
            frameImage_->setColor(i, qRgb(i, i, i)) ;
        framePixmap_ = new QPixmap();
        chArr_ = frameImage_->bits();
        wh_ = frameSize_->width() * frameSize_->height();
    }
    memcpy(chArr_, frame->image, wh_);
    frameLabel_->setPixmap(QPixmap::fromImage(*frameImage_));


    // We suggest only using repaint() if you need an immediate repaint,
    // for example during animation. In almost all circumstances update()
    // is better, as it permits Qt to optimize for speed and minimize flicker.
    //this->repaint();
    update();
}
