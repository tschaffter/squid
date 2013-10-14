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

#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include "dc1394/dc1394.h"
#include <QDialog>
#include <QLabel>

//! Elements of the graphical interface.
namespace Ui {
    class CameraDisplay;
}
//! Graphical interface of sQuid.
namespace qsquid {

/**
 * \brief Implements a dialog to display the frames grabbed by a camera.
 *
 * @version February 28, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class CameraDisplay : public QDialog {

    Q_OBJECT

private:

    /** Reference to the GUI. */
    Ui::CameraDisplay* ui_;

    /** QLabel on which image are printed. */
    QLabel* frameLabel_;
    /** Required to print images on QLabel. */
    QPixmap* framePixmap_;
    /** QImage representing dc1394 frames. */
    QImage* frameImage_;
    /** Size of the frames. */
    QSize* frameSize_;

    /** uchar frame. */
    unsigned char* chArr_;
    /** frameWidth * frameHeight */
    unsigned int wh_;

public:

    /** Constructor. */
    CameraDisplay(QWidget *parent = 0);
    /** Destructor. */
    ~CameraDisplay();

public slots:

    /** Displays the dc1394 frame received. */
    void displayFrame(dc1394video_frame_t* frame);

signals:

    /** Sent when the dialog is closed. */
    void displayClosed(bool visible);

private:

    /** Called when the viewer is closed (emits displayClosed(false)). */
    void closeEvent(QCloseEvent*);
};

}

#endif // CAMERADISPLAY_H
