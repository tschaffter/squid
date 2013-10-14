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

#include "cameramanager.h"
#include "experimenttime.h"
#include <glog/logging.h>
#include <sys/select.h>

#define NUM_BUFFERS 1

using namespace squid;

/** Singleton instance */
static CameraManager* instance_ = NULL;

// ======================================================================
// PRIVATE METHODS

void CameraManager::makeConnections() {}

// ======================================================================
// PUBLIC METHODS

CameraManager* CameraManager::getInstance() {

    if (instance_ == NULL)
        instance_ = new CameraManager();

    return instance_;
}

// ----------------------------------------------------------------------

CameraManager::~CameraManager() {

    mutex_.lock();
    abort_ = true;
    // to wake up the thread if it's sleeping
    // (the thread is put to sleep when it has nothing to do)
    condition_.wakeOne();
    mutex_.unlock();
    // to wait until run() has exited before the base class destructor is invoked
    wait();

    // delete all the cameras
    for (int i = 0; i < numCameras_; i++) {
        delete cameras_[i];
        cameras_[i] = NULL;
    }
    delete tmanager_;

    dc1394_free(d_); // After this, no libdc1394 function can be used.

    d_ = NULL;
    list_ = NULL;
    tmanager_ = NULL;
}

// ----------------------------------------------------------------------

void CameraManager::initialize(std::string dc1394) throw(MyException*) {

    dc1394_ = dc1394;
    d_ = NULL;
    list_ = NULL;
    tmanager_ = NULL;
    numCameras_ = 0;
    cameraIndex_ = 0;
    restart_ = false;
    abort_ = false;
    mode_ = FREERUN;
    saveFrame_ = false;

    LOG(INFO) << "Detecting dc1394 cameras.";
    detectCameras();
    setCameraIndex(0);
    // Used in TRIGGERS mode
    tmanager_ = new FdTriggerManager();
    grabReferenceTimer_ = NULL;

    makeConnections();
}

// ----------------------------------------------------------------------

void CameraManager::detectCameras() throw(MyException*) {
    
    d_ = dc1394_new();
    if (!d_)
        throw new MyException("dc1394_new() failed.");
    if (dc1394_camera_enumerate (d_, &list_) != DC1394_SUCCESS)
        throw new MyException("Failed to enumerate cameras.");
    if (list_->num == 0)
        throw new MyException("No cameras found.");

    LOG(INFO) << "Number of cameras found: " << list_->num;

    int j = 0;
    int numTries = 0;
    for (unsigned int i = 0; i < list_->num; i++) {
        if (j >= MAX_CAMERAS)
            break;
        LOG(INFO) << "Initializing camera with long guid " << list_->ids[i].guid << ".";
        dc1394camera_t* dc1394Camera = NULL;
        do {
            if (numTries > 0)
                LOG(INFO) << "Camera is not ready yet. Attempting to access the camera (" << (numTries+1) << "/" << MAX_CAMERA_DETECTION_TRIES << ").";
            // if the application is started when the cameras are not ready, dc1394_camera_new() returns NULL
            dc1394Camera = dc1394_camera_new(d_, list_->ids[i].guid);
            numTries++;
            if (dc1394Camera == NULL)
                sleep(1);

        } while(dc1394Camera == NULL && numTries <= MAX_CAMERA_DETECTION_TRIES);
        numTries = 0;

        if (dc1394Camera != NULL) {
            cameras_[j] = new Dc1394Camera(dc1394Camera); //cameras_[j] = dc1394_camera_new (d_, list_->ids[i].guid);
            if (!cameras_[j]->getCamera()) {
                dc1394_log_warning("Failed to initialize camera with guid %llx.", list_->ids[i].guid);
                continue;
            }
            j++;
        } else
            LOG (WARNING) << "Camera " << list_->ids[i].guid << " is not is currently not usable. Skipping it.";
    }
    numCameras_ = j;
    dc1394_camera_free_list(list_);
}

// ----------------------------------------------------------------------

/** Resets all cameras. */
void CameraManager::reset() {
    
    unsigned int numActiveCameras = activeCameras_.size();
    FpsEvaluator* fps = NULL;
    for (unsigned int i = 0; i < numActiveCameras; i++) {
        fps = activeCameras_[i]->getFpsEvaluator();
        fps->initialize();
        fps->setRestart(true);
    }

    if (mode_ == CameraManager::SOFTWARE_TRIGGERS) {
        for (unsigned int i = 0; i < numActiveCameras; i++) {
            Dc1394Camera* camera = activeCameras_[i];
            camera->setSoftwareTrigger();
        }
    }
    grabReferenceTimer_ = NULL;
}

// ----------------------------------------------------------------------

void CameraManager::run() {
    
    try {
        // first, define the camera which should be run
        if (activeCameras_.size() == 0)
            activeCameras_.push_back(this->getCamera());

        unsigned int numRunningCameras = activeCameras_.size();
        for (unsigned int i = 0; i < numRunningCameras; i++)
            LOG (INFO) << "Starting ISO transmission with " << activeCameras_[i]->getCameraNameAndGuid() << ".";

        // initialize camera and start ISO transmission
        setupActiveCameras();
        for (unsigned int i = 0; i < numRunningCameras; i++) {
            if((err_ = dc1394_video_set_transmission(activeCameras_[i]->getCamera(), DC1394_ON)) != DC1394_SUCCESS)
                dc1394_log_error("Could not start camera iso transmission.");
            if ((err_ = dc1394_capture_setup(activeCameras_[i]->getCamera(), NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT)) != DC1394_SUCCESS)
                dc1394_log_error("Could not setup camera make sure that the video mode and framerate are supported by your camera.");
        }

        // prepare trigger for cameras that use it
        const bool triggerMode = (mode_ == CameraManager::SOFTWARE_TRIGGERS);
        if (triggerMode) {
            LOG(INFO) << "Setting trigger manager with period " << tmanager_->getIntervalInUs() << " us.";
            tmanager_->setPostTriggerAction((pfv) &CameraManager::triggerFunction);
            LOG(INFO) << "Starting camera software trigger." << std::endl;
            tmanager_->start();
        }

        for (unsigned int i = 0; i < numRunningCameras; i++) {
            LOG (INFO) << "Starting FPS evaluator for camera " << activeCameras_[i]->getCameraNameAndGuid() << ".";
            activeCameras_[i]->getFpsEvaluator()->start(QThread::LowestPriority);
        }


        // do not save the frame by default when starting the camera
        saveFrame_ = false;


        forever {
            if (abort_) {
                LOG(INFO) << "Stopping cameras." << std::endl;
                if (triggerMode) {
                    LOG (INFO) << "Stopping camera software trigger.";
                    tmanager_->stop();
                    tmanager_->setPostTriggerAction((pfv) &FdTriggerManager::defaultPostTriggerAction);
                }

                // stop FPS evaluators
                for (unsigned int i = 0; i < numRunningCameras; i++) {
                    LOG (INFO) << "Stopping FPS evaluator of camera " << activeCameras_[i]->getCameraNameAndGuid() << ".";
                    activeCameras_[i]->getFpsEvaluator()->stop();
                }

                // stop cameras
                for (unsigned int i = 0; i < numRunningCameras; i++) {
                    LOG (INFO) << "Stopping camera " << activeCameras_[i]->getCameraNameAndGuid() << ".";
                    if (triggerMode) {
                        if ((err_ = dc1394_external_trigger_set_power(activeCameras_[i]->getCamera(), DC1394_OFF)) != DC1394_SUCCESS)
                            dc1394_log_error("Could not disable trigger.");
                    }
                    if ((err_ = dc1394_capture_stop(activeCameras_[i]->getCamera())) != DC1394_SUCCESS)
                        dc1394_log_error("Could not stop the captures.");
                    if ((err_ = dc1394_video_set_transmission(activeCameras_[i]->getCamera(), DC1394_OFF)) != DC1394_SUCCESS)
                        dc1394_log_error("Could not stop iso transmission.");
                }
                return;
            }

            // when capturing a frame you can choose to either wait for the frame indefinitely (WAIT)
            // or return immediately if no frame arrived yet (POLL).
            for (unsigned int i = 0; i < numRunningCameras; i++) {
                Dc1394Camera* camera = activeCameras_[i];
                dc1394video_frame_t* frame = camera->frame_.frame_;

                // dequeue to get the image :)
                if (dc1394_capture_dequeue(activeCameras_[i]->getCamera(), DC1394_CAPTURE_POLICY_WAIT, &frame) != DC1394_SUCCESS)
                    dc1394_log_error("Failed to capture frame.");

                // SIGNAL SENT WHEN A FRAME IS GRABBED
                unsigned int us = 0;
                if (grabReferenceTimer_ != NULL)
                    us = grabReferenceTimer_->getElapsedTimeInUs();
                emit frameCaptured(frame, i, us, saveFrame_);

                camera->getFpsEvaluator()->incrementNumFrames();

                // otherwise the buffer will saturate
                if ((err_ = dc1394_capture_enqueue(activeCameras_[i]->getCamera(), frame)) != DC1394_SUCCESS)
                    dc1394_log_error("Could not enqueue frame.");
            }

            mutex_.lock();
            if (!restart_) {
                // it's required to first stop the trigger and then put the camera to sleep
                tmanager_->pause(true);
                condition_.wait(&mutex_);
            }
            tmanager_->pause(false);
            mutex_.unlock();
        }
    } catch (MyException* e) {
        LOG(ERROR) << "Unable to run the cameras: " << e->getMessage();
    } catch (std::exception& e) {
        LOG(ERROR) << "Unable to trun the cameras: " << e.what();
    }
}

// ----------------------------------------------------------------------

void CameraManager::stop() {

    abort_ = true;
}

// ----------------------------------------------------------------------

void CameraManager::wake() {
    
//    tmanager_->setRestart(true);
    condition_.wakeOne();
//    tmanager_->wake();
}

// ----------------------------------------------------------------------

void CameraManager::setAllCamerasActive() {
    
    setAllCamerasPassive();
    for (int i = 0; i < numCameras_; i++)
        activeCameras_.push_back(cameras_[i]);
}

// ----------------------------------------------------------------------

void CameraManager::setAllCamerasPassive() {
    
    activeCameras_.clear();
}

// ----------------------------------------------------------------------

bool CameraManager::setCameraActive(std::string guid) throw(MyException*) {
    
    Dc1394Camera* c = this->getCamera(guid);
    if (c == NULL)
        throw new MyException("Invalid camera guid.");

    for (unsigned int i = 0; i < activeCameras_.size(); i++) {
        if (*c == *(activeCameras_[i]))
            return false;
    }

    activeCameras_.push_back(c);
    return true;
}

// ----------------------------------------------------------------------

bool CameraManager::setCameraPassive(std::string guid) throw(MyException*) {
    
    Dc1394Camera* c = this->getCamera(guid);
    if (c == NULL)
        throw new MyException("Invalid camera guid.");

    for (unsigned int i = 0; i < activeCameras_.size(); i++) {
        if (*c == *(activeCameras_[i])) {
            activeCameras_.erase(activeCameras_.begin() + i);
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------

/** Only use the CameraManager pointer got from getInstance(). */
void CameraManager::triggerFunction(int /*triggerId*/) {

    CameraManager* cmanager = CameraManager::getInstance(); // access could be improved
    const unsigned int n = cmanager->getNumActiveCameras();
    for (unsigned int i = 0; i < n; i++) {
        if ((cmanager->err_ = dc1394_software_trigger_set_power(cmanager->getActiveCamera(i)->getCamera(), DC1394_ON)) != DC1394_SUCCESS)
            LOG(WARNING) << "Could not send software trigger.";
    }
}

// ----------------------------------------------------------------------

void CameraManager::setupActiveCameras() {

    const unsigned int n = activeCameras_.size();
    for (unsigned int i = 0; i < n; i++)
        activeCameras_[i]->setupCamera(dc1394_);
}

// ======================================================================
// SETTERS AND GETTERS

Dc1394Camera* CameraManager::getCamera(int index) throw(MyException*) {

    if (index < 0 && index > numCameras_)
        throw new MyException("Invalid camera index.");
    return cameras_[index];
}

// ----------------------------------------------------------------------

Dc1394Camera* CameraManager::getCamera(std::string guid) {

    int index = getCameraIndex(guid);

    if (index >= 0 && index < numCameras_)
        return cameras_[index];
    else
        return NULL;
}

// ----------------------------------------------------------------------

Dc1394Camera* CameraManager::getCamera() { return cameras_[cameraIndex_]; }

int CameraManager::getCameraIndex(std::string guid) {

    for (int i = 0; i<numCameras_; i++) {
        if (strcmp(guid.c_str(), cameras_[i]->getCameraGuid().c_str()) == 0)
            return i;
    }
    return -1;
}

// ----------------------------------------------------------------------

void CameraManager::setCameraIndex(int index) { cameraIndex_ = index; }
int CameraManager::getCameraIndex() { return cameraIndex_; }

int CameraManager::getNumCameras() { return numCameras_; }

FdTriggerManager* CameraManager::getTriggerManager() { return tmanager_; }

void CameraManager::setRestart(bool restart) { restart_ = restart; }
void CameraManager::setAbort(bool abort) { abort_ = abort; }

void CameraManager::setCameraMode(cameraMode mode) { mode_ = mode; }
CameraManager::cameraMode CameraManager::getCameraMode() { return mode_; }

unsigned int CameraManager::getNumActiveCameras() { return activeCameras_.size(); }
Dc1394Camera* CameraManager::getActiveCamera(const unsigned int index) { return activeCameras_[index]; }

void CameraManager::setSaveFrame(bool saveFrame) {

    mutex_.lock();
    saveFrame_ = saveFrame;
    mutex_.unlock();
}
bool CameraManager::getSaveFrame() { return saveFrame_; }
