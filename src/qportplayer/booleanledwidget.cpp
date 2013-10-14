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

#include "booleanledwidget.h"
#include "ui_booleanledwidget.h"
#include "qled.h"

using namespace qportplayer;

// ======================================================================
// PUBLIC METHODS

BooleanLedWidget::BooleanLedWidget(QWidget *parent) : QWidget(parent), ui(new Ui::BooleanLedWidget) {

    ui->setupUi(this);
    QLed* a = new QLed();
    a->setOnColor(QLed::Blue);
}

// ----------------------------------------------------------------------

BooleanLedWidget::~BooleanLedWidget() {

    delete ui;
    ui = NULL;
}

// ----------------------------------------------------------------------

void BooleanLedWidget::changeEvent(QEvent *e) {

    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslate();
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------

void BooleanLedWidget::retranslate() {

    //ui->retranslateUi(this);
}

// ----------------------------------------------------------------------

void BooleanLedWidget::setOn() { ui->qLed->setValue(true); }
void BooleanLedWidget::setOff() { ui->qLed->setValue(false); }
void BooleanLedWidget::setState(const bool state) { ui->qLed->setValue(state); }

// ======================================================================
// GETTERS AND SETTERS

void BooleanLedWidget::setBooleanItem(portplayer::IOPin* booleanItem) { booleanItem_ = booleanItem; }
portplayer::IOPin* BooleanLedWidget::getBooleanItem() { return booleanItem_; }

void BooleanLedWidget::setBooleanItemName(const std::string name) { ui->booleanItemLabel->setText(name.c_str()); }
std::string BooleanLedWidget::getBooleanItemName() { return ui->booleanItemLabel->text().toStdString(); }
