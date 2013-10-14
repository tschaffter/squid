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

#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "cameradisplay.h"
#include <vector>
#include <QObject>

//! Graphical interface of sQuid.
namespace qsquid {

/**
 * \brief Manages CameraDisplay objects.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class DisplayManager : public QObject {

    Q_OBJECT

private:

    /** List of displays. */
    std::vector<CameraDisplay*> displays_;
    /** Number of displays is the number of sub-experiments. */
    unsigned int numDisplays_;

public:

    /** Constructor. */
    DisplayManager(std::vector<std::string> displayNames);
    /** Destructor. */
    ~DisplayManager();

public slots:

    /** Displays the frame on a display. */
    void displayFrame(dc1394video_frame_t* frame, unsigned int cameraIndex = 0, unsigned int us = 0, bool saveFrame = false);

    /** Shows all displays. */
    void displayAll();
    /** Hides all displays. */
    void hideAll();
    /** Closes all displays. */
    void closeAll();
};

}

#endif // DISPLAYMANAGER_H
