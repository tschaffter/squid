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

#include "about.h"
#include "squid.h"
#include "ui_about.h"
#include <sstream>
#include <QImage>
#include <QPixmap>
#include <glog/logging.h>

using namespace squid;
using namespace qsquid;

// ======================================================================
// PUBLIC METHODS

About::About(QWidget* parent) : QDialog(parent), ui_(new Ui::About) {

    ui_->setupUi(this);
    setWindowTitle("About sQuid");

    QImage banner = QImage(":/header", "PNG");
    ui_->squidBanner->setPixmap(QPixmap::fromImage(banner));

    ui_->aboutText->setTextFormat(Qt::RichText);
    ui_->aboutText->setOpenExternalLinks(true);
    std::stringstream str;
    str << "<b>sQuid " << SQUID_VERSION << "</b> (" << SQUID_VERSION_DATE << ")" << "<br>";
    str << "<br>";
    str << "Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)<br>";
    str << "Project website: <a href=\"http://tschaffter.ch/projects/squid/\">http://tschaffter.ch/projects/squid/</a>"; // << "<br>";


//    str << "<a href=\"http://lis.epfl.ch\">Laboratory of Intelligent Systems</a>" << "<br>";
//    str << "<a href=\"http://www.epfl.ch\">Swiss Federal Institute of Technology Lausanne (EPFL)</a><br>";
    ui_->aboutText->setText(QString(str.str().c_str()));
    connect(ui_->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

// ----------------------------------------------------------------------

About::~About() {

    delete ui_;
    ui_ = NULL;
}
