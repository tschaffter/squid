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

#ifndef ENHANCEDPROGRESSBAR_H
#define ENHANCEDPROGRESSBAR_H

#include <QtGui/QWidget>
#include <QTime>

//! Elements of the graphical interface.
namespace Ui {
    class EnhancedProgressBar;
}
//! Graphical interface to control the environment (valves, LEDs, robots, etc.).
namespace qportplayer {

/**
 * \brief Enhanced progress bar including a QProgressBar, a QComboBox and a QLabel to
 * select and display elapsed or remaining time.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class EnhancedProgressBar : public QWidget {

    Q_OBJECT

private:

    /** Reference to the GUI. */
    Ui::EnhancedProgressBar* ui;

    /** Time max of the process. */
    unsigned int tmax_;
    /** Hours. */
    unsigned int h_;
    /** Minutes. */
    unsigned int min_;
    /** Seconds. */
    unsigned int s_;
    /** Milliseconds. */
    unsigned int ms_;
    /** To display the time as a string. */
    char* text_;

public:

    /** Constructor. */
    EnhancedProgressBar(QWidget *parent = 0);
    /** Destructor. */
    ~EnhancedProgressBar();

public slots:

    /** Resets the progress bar to make it ready to be used again. */
    void reset();
    /** Sets the time in milliseconds. */
    void setTimeInMs(const unsigned int time);
    /** Sets the time in microseconds. */
    void setTimeInUs(const unsigned int time);
    /** Sets the maximum duration in milliseconds. */
    void setMaxDurationInMs(const unsigned int maxDuration);
    /** Refreshes the progress bar. */
    void updateGui();
    /** Sets the value of the progress bar. */
    void setValue(int value);

protected:

    void changeEvent(QEvent *e);
};

}

#endif // ENHANCEDPROGRESSBAR_H
