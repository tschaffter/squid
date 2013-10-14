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

#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include "dc1394camera.h"
#include "fdtriggermanager.h"
#include "fpsevaluator.h"
#include "highresolutiontime.h"
#include <vector>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/** Max number of cameras handled (arbitrary number). */
#define MAX_CAMERAS 16
/** Max number of 1s-separated tries to get one camera ready. */
#define MAX_CAMERA_DETECTION_TRIES 20

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Manage the dc1394 cameras (Singleton pattern).
 *
 * TODO: Allow changing the video mode/resolution online in FREERUN mode.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class CameraManager : public QThread {

    Q_OBJECT

public:

    /** Camera mode. */
    enum cameraMode {
        FREERUN = 0,
        SOFTWARE_TRIGGERS = 1
    };

    /** Reference to a time to get timestamp for grabbed image. Declared as public for prototyping. */
    HighResolutionTime* grabReferenceTimer_;

private:

    /** Pack of dc1394 cameras. */
    dc1394_t* d_;
    /** List of dc1394 cameras. */
    dc1394camera_list_t* list_;
    /** Detected dc1394 cameras. */
    Dc1394Camera* cameras_[MAX_CAMERAS];
    /** List of active cameras to run (identified by their guid). */
    std::vector<Dc1394Camera*> activeCameras_;
    /** Number of camera detected. */
    int numCameras_;
    /** Index of the camera selected. */
    int cameraIndex_;
    /** Camera mode (0 = FREERUN, 1 = SOFTWARE_TRIGGERS). */
    cameraMode mode_;
    /** Tag the frame sent to know if it must be saved or not. */
    bool saveFrame_;

    /** Used to send the triggers to the camera. */
    FdTriggerManager* tmanager_;

    /** Mutex. */
    QMutex mutex_;
    /** Condition for waiting. */
    QWaitCondition condition_;
    /** Set to true when the camera is sleeping to make it restart. */
    bool restart_;
    /** Stop the camera. */
    bool abort_;

public:

    /** Dc1394 mode (a = FireWire400, b = FireWire800). */
    std::string dc1394_;

    /** Errors support. */
    dc1394error_t err_;

    /** Get singleton instance. */
    static CameraManager* getInstance();

    /** Destructor. */
    ~CameraManager();

    /** Initialization. */
    void initialize(std::string) throw(MyException*);

    /** Returns trigger manager. */
    FdTriggerManager* getTriggerManager();

    /** Sets to true if the trigger as shoot from an external trigger. */
    void setUseExternalTrigger(bool b);
    /** Returns true if an external trigger is used to shoot. */
    bool getUseExternalTrigger();

    /** Returns the total number of cameras detected. */
    int getNumCameras();
    /** Returns the index of the current camera. */
    int getCameraIndex();
    /** Returns the index of the camera associated to the given guid. */
    int getCameraIndex(std::string guid);

    /** Returns the current camera. */
    Dc1394Camera* getCamera();
    /** Returns the camera associated to the given index. */
    Dc1394Camera* getCamera(int index) throw(MyException*);
    /** Returns the camera associated to the guid. */
    Dc1394Camera* getCamera(std::string guid);

    /** Sets camera mode. */
    void setCameraMode(cameraMode mode);
    /** Returns camera mode. */
    cameraMode getCameraMode();

    /** Sets all detected camera as active. */
    void setAllCamerasActive();
    /** Sets all detected cameras as passive. */
    void setAllCamerasPassive();
    /** Sets a camera active (identified by guid) */
    bool setCameraActive(std::string guid) throw(MyException*);
    /** Sets a camera passive. */
    bool setCameraPassive(std::string guid) throw(MyException*);
    /** Returns the number of active cameras. */
    unsigned int getNumActiveCameras();
    /** Returns active camera (identified by index). */
    Dc1394Camera* getActiveCamera(const unsigned int index);
    /** Setups all active camera. */
    void setupActiveCameras();

    /** Sets dc1394 mode. */
    void setDc1394(const std::string dc1394);
    /** Returns dc1394 mode. */
    std::string getDc1394() const;

    /** Indiquates that the frames emitted should be saved. */
    void setSaveFrame(bool saveFrame);
    /** Returns true if frames are currently being saved. */
    bool getSaveFrame();

public slots:

    /** Start thread. */
    void run();
    /** Stop thread. */
    void stop();
    /** Call setRestart(false) to put the thread to sleep. */
    void setRestart(bool restart);
    /** Call setAbort(true) to stop the thread. */
    void setAbort(bool abort);
    /** If the thread has been put to sleep with setRestart(false), call this method to wake it. */
    void wake();

    /** Detect cameras. */
    void detectCameras() throw(MyException*);
    /** Set index of the current camera. */
    void setCameraIndex(int index);
    /** Call this function before running a camera. */
    void reset();

signals:

    /** Sent each time a frame is captured. */
    void frameCaptured(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int us, bool save);

private:

    /** Constructor. */
    CameraManager() {}

    /** Make connections. */
    void makeConnections();

    /** Function called each time a trigger is generated. */
    void triggerFunction(int triggerId);
};

} // end namespace squid

#endif // CAMERAMANAGER_H
