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

#include "enhancedprogressbar.h"
#include "ui_enhancedprogressbar.h"
#include "myutility.h"
#include <cstdio>
#include <glog/logging.h>

using namespace qportplayer;

// ======================================================================
// PUBLIC METHODS

EnhancedProgressBar::EnhancedProgressBar(QWidget *parent) : QWidget(parent), ui(new Ui::EnhancedProgressBar) {

    ui->setupUi(this);
    reset();
}

// ----------------------------------------------------------------------

EnhancedProgressBar::~EnhancedProgressBar() {

    delete ui;
    delete text_;

    ui = NULL;
    text_ = NULL;
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::changeEvent(QEvent* event) {

    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::reset() {

    tmax_ = 0;
    h_ = 0;
    min_ = 0;
    s_ = 0;
    ms_ = 0;
    text_ = NULL;

    QIcon clockIcon(":/clock");
    ui->comboBox->addItem(clockIcon, "Elapsed");
    ui->comboBox->addItem(clockIcon, "Remaining");

    ui->progressBar->setValue(0);

    if (tmax_ == 0) {
        ui->comboBox->setCurrentIndex(0);
        ui->comboBox->setEnabled(false);
    }
    else
        ui->comboBox->setEnabled(true);

    updateGui();
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::updateGui() {

    update();
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::setTimeInMs(const unsigned int time) {

    if (ui->comboBox->currentIndex() == 0) {
        formatTimeInMs(time, h_, min_, s_, ms_);
        text_ = new char[8];
        sprintf(text_, "%02d:%02d:%02d", h_, min_, s_);
        ui->label->setText(text_);
    } else {
        // this step is required to round at the "second" level
        // otherwise 999 ms is 0 s but it's closest to 1 s
        float minus = std::max(0, (int)tmax_ - (int)time);
        formatTimeInMs(minus, h_, min_, s_, ms_);
        text_ = new char[8];
        sprintf(text_, "%02d:%02d:%02d", h_, min_, s_);
        ui->label->setText(text_);
    }

    if (tmax_ > 0)
        ui->progressBar->setValue(100. * (double)time / (double)tmax_);

    updateGui();
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::setTimeInUs(const unsigned int t_us) {

    setTimeInMs(t_us / 1000);
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::setMaxDurationInMs(const unsigned int tmax) {

    tmax_ = tmax;
    bool b = (tmax_ > 0);
    ui->comboBox->setEnabled(b);
    if (!b)
        ui->comboBox->setCurrentIndex(0);
}

// ----------------------------------------------------------------------

void EnhancedProgressBar::setValue(int value) {

    ui->progressBar->setValue(value);
}
