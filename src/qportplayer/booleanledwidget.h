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

#ifndef BOOLEANLEDWIDGET_H
#define BOOLEANLEDWIDGET_H

#include "iopin.h"
#include <QWidget>

//! Elements of the graphical interface.
namespace Ui {
    class BooleanLedWidget;
}
//! Graphical interface to control the environment (valves, LEDs, robots, etc.).
namespace qportplayer {

/**
 * \brief Represents a labeled LED to display the state of a boolean variable.
 *
 * Here the boolean item is associated to a IOPin object.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class BooleanLedWidget : public QWidget {

    Q_OBJECT

private:

    /** Reference of the GUI. */
    Ui::BooleanLedWidget* ui;
    /** Reference to the boolean item (IOPin). */
    portplayer::IOPin* booleanItem_;

public:

    /** Constructor. */
    BooleanLedWidget(QWidget *parent = 0);
    /** Destructor. */
    ~BooleanLedWidget();

    /** Returns the reference to the boolean item (IOPin). */
    portplayer::IOPin* getBooleanItem();
    /** Returns the name of the boolean item. */
    std::string getBooleanItemName();
    
public slots:

    /** Sets the reference of the boolean item (IOPin). */
    void setBooleanItem(portplayer::IOPin* booleanItem);
    /** Sets the name of the boolean item. */
    void setBooleanItemName(const std::string name);

    /** Turns on the boolean item. */
    void setOn();
    /** Turns off the boolean item. */
    void setOff();
    /** Sets the state of the boolean item. */
    void setState(const bool state);

protected:

    void changeEvent(QEvent *e);
    void retranslate();
};

}

#endif // BOOLEANLEDWIDGET_H
