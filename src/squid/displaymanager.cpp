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

#include "displaymanager.h"
#include "myexception.h"
#include "cameramanager.h"
#include "squidsettings.h"
#include <QDesktopWidget>
#include <QApplication>
#include <sstream>
#include <glog/logging.h>

using namespace qsquid;

// ======================================================================
// PUBLIC METHODS

DisplayManager::DisplayManager(std::vector<std::string> displayNames) {

    numDisplays_ = displayNames.size();
    std::string title;
    CameraDisplay* display = NULL;
    const unsigned int selectedCamera = squid::CameraManager::getInstance()->getCameraIndex();
    for (int i = numDisplays_ - 1; i >= 0; i--) {
        display = new CameraDisplay();
        title = displayNames.at(numDisplays_ - i - 1).c_str();
        if ( (numDisplays_ - i - 1) == selectedCamera)
            title += " (selected)";
        display->setWindowTitle(title.c_str());
        display->show();
        displays_.push_back(display);

        // at that time displays are empty shell (no images printed yet)
        display->move(0, 0);
    }
}

// ----------------------------------------------------------------------

DisplayManager::~DisplayManager() {

    closeAll();
}

// ----------------------------------------------------------------------

void DisplayManager::displayFrame(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int /*us*/, bool /*saveFrame*/) {

    if (displays_.empty())
        return;

    // for now only print on the first display
    CameraDisplay* display = dynamic_cast<CameraDisplay*>(displays_.at(cameraIndex));
    display->displayFrame(frame);
}

// ----------------------------------------------------------------------

void DisplayManager::displayAll() {

    for (unsigned int i = 0; i < numDisplays_; i++) {
        if (displays_.at(i) != NULL)
            dynamic_cast<CameraDisplay*>(displays_.at(i))->setVisible(true);
    }
}

// ----------------------------------------------------------------------

void DisplayManager::hideAll() {

    for (unsigned int i = 0; i < numDisplays_; i++) {
        if (displays_.at(i) != NULL)
            dynamic_cast<CameraDisplay*>(displays_.at(i))->setVisible(false);
    }
}

// ----------------------------------------------------------------------

void DisplayManager::closeAll()
{
    CameraDisplay* display = NULL;

    for (unsigned int i = 0; i < numDisplays_; i++) {
        display = dynamic_cast<CameraDisplay*>(displays_.at(i));
        display->close();
        delete display;
        display = NULL;
    }
    displays_.clear();
    numDisplays_ = 0;
}
