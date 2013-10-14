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

#ifndef SQUID_H
#define SQUID_H

#include "cameramanager.h"
#include "displaymanager.h"
#include "experiment.h"
#include "enhancedprogressbar.h"
#include "squidplayer.h"
#include "fdtriggermanager.h"
#include <cstring>
#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QActionGroup>

#define SQUID_VERSION "1.0.10 Beta"
#define SQUID_VERSION_DATE "February 2012"

//! Elements of the graphical interface.
namespace Ui {
    class Squid;
}
//! Graphical interface of sQuid.
namespace qsquid {

/**
 * \brief Implements the main dialog of sQuid.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Squid : public QMainWindow {

    Q_OBJECT

private:

    /** Reference of the GUI. */
    Ui::Squid* ui_;

    /** Camera manager. */
    squid::CameraManager* cmanager_;
    /** Software trigger manager (promotion to realtime priority is achieved using rtkit). */
    squid::FdTriggerManager* fdTmanager_;
    /** Current experiment. */
    squid::Experiment* experiment_;
    /** Display manager. */
    DisplayManager* dmanager_;

    /** Enhanced progress bar attached to the experiment progress. */
    qportplayer::EnhancedProgressBar experimentProgressBar_;

    /** Contains the camera resolution supported by sQuid (mainly MONO8 modes). */
    QActionGroup* resolutionActionGroup_;
    /** Contains the camera FPS supported by sQuid. */
    QActionGroup* fpsActionGroup_;
    /** Action of opening a dialog to specify an area of interest. */
    QAction* aoiAction_;

protected:

    /** Called when the application window is closed. */
    void closeEvent(QCloseEvent* e);

public:

    /** Constructor. */
    Squid(int argc, char* argv[], QWidget* parent = 0);
    /** Destructor. */
    ~Squid();

    /** Sets the location of the sQuid window on the display (default: top-right corner). */
    void setLocationOnDisplay();

public slots:

    /** Returns the reference of the GUI. */
    Ui::Squid* getUi();

    /** Updates GUI. */
    void updateGui();

    /** Returns the camera manager. */
    squid::CameraManager* getCameraManager();
    /** Returns the current experiment. */
    squid::Experiment* getExperiment();

    /** Enables of disables the cameras. */
    void runCamera(bool run);
    /** Stops all the running cameras. */
    void stopCameras();
    /** Changes selected camera. */
    void changeCamera(int cameraIndex) throw(MyException*);
    /** Changes camera mode. */
    void changeCameraMode() throw(MyException*);
    /** Holds camera capture. */
    void holdCamera();
    /** Resumes camera capture. */
    void resumeCamera();
    /** Updates selected camera FPS. */
    void updateFps(const float fps);

    /** Initializes experiment. */
    void initializeExperiment();
    /** Runs a new experiment. */
    void runExperiment();
    /** Stops running experiment. */
    void stopExperiment();

    /** Enables or disables the port player. */
    void playerEnabled(bool enable);

    /** Called when the selected state in the port player changed. */
    void playerStateChanged(const unsigned int currentState);

    /** Opens a dialog to select the output directory. */
    void setOutputDirectory();

    /** Changes camera resolution. */
    bool changeCameraResolution(QAction* action);
    /** Changes camera FPS. */
    bool changeCameraFramerate(QAction* action);

    /** Displays About dialog. */
    void displayAbout();
    /** Displays a dialog to set the list of email addresses. */
    void setEmailAddress();
    /** Displays a dialog to set an area of interest. */
    void displayAoiDialog();

    /** Opens settings file. */
    void openSettings();
    /** Reloads last settings file opened. */
    void reloadSettings();
    /** Saves settings files */
    void saveSettings();
    /** Saves As settings file. */
    void saveAsSettings();

    /** Wrapper method to save frames. */
    void saveFrame(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int us, bool saveFrame);

private:

    /** Initialization. */
    void initialize() throw(MyException*);

    /** Checks if at lease one camera is available. */
    void checkCamerasAvailability();

    /** Lists all cameras into a combobox. */
    void listAllCameras();

    /** Initializes the GUI. */
    void initializeGuiComponents() throw(MyException*);
    /** Builds camera menu. */
    void buildCameraMenu();
    /** Builds camera resolution sub-menu. */
    void buildCameraResolutionSubMenu();
    /** Builds camera framerate sub-menu. */
    void buildCameraFramerateSubMenu();

    /** Connects SIGNALS/SLOTS. */
    void makeConnections() ;

    /** Updates the settings of the cameras. */
    void updateCameraControllers() throw(MyException*);

    /** Checks for a confirmation before leaving (only if cameras/experiment are running). */
    bool fineToExit();

    /** Apply seetings stored in SquidSettings. */
    void importSettings() throw(MyException*);
    /** Store sQuid parameters to SquidSettings. */
    void exportSettings() throw(MyException*);
};

}

#endif // SQUID_H
