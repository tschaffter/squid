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

#include "dc1394camera.h"
#include "dc1394utility.h"
#include "cameramanager.h"
#include <sstream>
#include <glog/logging.h>

/** Camera ring buffer */
#define NUM_BUFFERS 4

using namespace squid;

// ======================================================================
// PRIVATE METHODS

void Dc1394Camera::setDc1394a() {

    LOG (INFO) << "Setting camera " << this->getCameraNameAndGuid() << " in DC1394a mode (FireWire400).";
    if ((err_ = dc1394_video_set_operation_mode(camera_, DC1394_OPERATION_MODE_LEGACY)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Could not set operation mode DC1394a (FireWire400).");
    }
    if ((err_ = dc1394_video_set_iso_speed(camera_, DC1394_ISO_SPEED_400)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Could not set ISO speed for DC1394a (FireWire400).");
    }
}

// ----------------------------------------------------------------------

void Dc1394Camera::setDc1394b()
{
    LOG (INFO) << "Settings camera " << this->getCameraNameAndGuid() << " in DC1394b mode (FireWire800)." << std::endl;
    if ((err_ = dc1394_video_set_operation_mode(camera_, DC1394_OPERATION_MODE_1394B)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Could not set operation mode DC1394b (FireWire800).");
    }
    if ((err_ = dc1394_video_set_iso_speed(camera_, DC1394_ISO_SPEED_800)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Could not set ISO speed for DC1394b (FireWire800).");
    }
}

// ======================================================================
// PUBLIC METHODS

Dc1394Camera::Dc1394Camera(dc1394camera_t* camera) {

    try {
        camera_ = camera;
        useAoi_ = false;
        initialize();
    } catch (MyException* e) {
        LOG(WARNING) << "Failed to instantiate camera: " << e->what();
    }
}

// ----------------------------------------------------------------------

Dc1394Camera::~Dc1394Camera() {

    cleanup();
    resetBus();
    freeCamera();

    delete fpsEvaluator_;
    fpsEvaluator_ = NULL;
}

// ----------------------------------------------------------------------

void Dc1394Camera::initialize() throw(MyException*) {

    LOG(INFO) << "Initializing camera " << getCameraName() << " (" + getCameraGuid() << ")" << ".";
    supportedResolutions_ = getSupportedResolutions(camera_);
    resolution_ = getHighestSupportedResolution(camera_, &supportedResolutions_, DC1394_COLOR_CODING_MONO8); // XXX: squid only handle MONO8 images (for display and saving)
    supportedFps_ = getSupportedFps(camera_, resolution_); // get the supported FPS from the current video mode
    fps_ = getHighestSupportedFps(&supportedFps_);
    // Object to compute the FPS
    fpsEvaluator_ = new FpsEvaluator();
    // backup these values from the camera
    gainBkp_ = getGain();
    stdShutterBkp_  = getStdShutter();
    brightnessBkp_ = getBrightness();
}

// ----------------------------------------------------------------------

void Dc1394Camera::setupCamera(std::string dc1394) throw(MyException*) {

    if (dc1394.compare("a") == 0)
        setDc1394a();
    else if (dc1394.compare("b") == 0)
        setDc1394b();
    else {
        std::string str = "Invalid dc1394 protocol (" + dc1394 + ").";
        throw new MyException(str);
    }

    setGain(gainBkp_);
    setStdShutter(stdShutterBkp_);
    setBrightness(brightnessBkp_);

    // set delay between trigger and time when the integration starts (must already be set to 0)
    if ((err_ = dc1394_feature_set_value(camera_, DC1394_FEATURE_TRIGGER_DELAY, 0)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Could not set framerate.");
    }
}

// ----------------------------------------------------------------------

void Dc1394Camera::setSoftwareTrigger() {

    LOG (INFO) << "Setting camera " << getCameraNameAndGuid() << " to respond to software trigger.";

    /* Lines added to show software trigger */
    // Mode 0: Exposure starts with a falling edge and stops when the the exposure specified by the SHUTTER feature is elapsed.
    if ((err_ = dc1394_external_trigger_set_mode(camera_, DC1394_TRIGGER_MODE_0)) != DC1394_SUCCESS)
    throw new MyException("Could not select trigger mode.");

    if ((err_ = dc1394_external_trigger_set_source(camera_, DC1394_TRIGGER_SOURCE_SOFTWARE)) != DC1394_SUCCESS)
    throw new MyException("Could not select software trigger.");

    if ((err_ = dc1394_external_trigger_set_power(camera_, DC1394_ON)) != DC1394_SUCCESS)
    throw new MyException("Could not activate trigger.");

    // WaitingForTrigger on GPOut2
    if ((err_ = dc1394_set_adv_control_register(camera_, 0x324, 0x800A0000)) != DC1394_SUCCESS)
        throw new MyException("Could not set WaitingForTrigger on GPOut2.");
}

// ----------------------------------------------------------------------

void Dc1394Camera::cleanup(bool verbose) {

    if (verbose)
        LOG(INFO) << "Cleaning up camera " << getCameraName() << " (" << getCameraGuid() << ")" << ".";

    // If failed, "Could not stop iso transmission"
    err_ = dc1394_video_set_transmission(camera_, DC1394_OFF);
    // If failed, "Could not stop the captures"
    err_ = dc1394_capture_stop(camera_);
    if (CameraManager::getInstance()->getCameraMode() == CameraManager::SOFTWARE_TRIGGERS) {
        // If failed, "Could not disable trigger"
        err_ = dc1394_external_trigger_set_power(camera_, DC1394_OFF);
    }
}

// ----------------------------------------------------------------------

bool Dc1394Camera::operator==(const Dc1394Camera& c) const {

    return (getCameraGuid().compare(c.getCameraGuid()) == 0);
}

// ----------------------------------------------------------------------

void Dc1394Camera::resetBus() {

    if ((err_ = dc1394_set_adv_control_register(camera_, 0x510, 0x82000000)) != DC1394_SUCCESS)
        LOG (WARNING) << "Could not reset the camera bus.";
}

// ----------------------------------------------------------------------

void Dc1394Camera::freeCamera() {

   dc1394_camera_free(camera_);
   camera_ = NULL;
}

// ----------------------------------------------------------------------

void Dc1394Camera::printCameraInfo() {

    dc1394featureset_t featureSet;

    if ((err_ = dc1394_feature_get_all(camera_, &featureSet)) != DC1394_SUCCESS) {
        LOG (WARNING) << "Failed to dump camera info.";
        return;
    }
    dc1394_feature_print_all(&featureSet, stderr);
}

// ======================================================================
// GETTERS AND SETTERS

/** WARNING: Don't forget to call Dc1394Camera::setupCamera() to take into account the new parameters */
void Dc1394Camera::setResolution(dc1394video_mode_t resolution) throw(MyException*) {

    if (!isResolutionSupported(&supportedResolutions_, resolution))
        throw new MyException("Resolution not supported.");

    resolution_ = resolution;
    // update the list of supported FPS related to this resolution
    supportedFps_ = getSupportedFps(camera_, resolution);

    if ((err_ = dc1394_video_set_mode(camera_, resolution_)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Unable to set the resolution.");
    }

    if (!isFpsSupported(&supportedFps_, fps_)) {
        LOG(WARNING) << "Current FPS no longer valid. Automatically change for the highest FPS supported.";
        setFps( getHighestSupportedFps(&supportedFps_));
    }
}

// ---------------------------------------------------------------------- //

/** WARNING: Don't forget to call Dc1394Camera::setupCamera() to take into account the new parameters */
void Dc1394Camera::setFps(dc1394framerate_t fps) throw(MyException*) {

    if (!isFpsSupported(&supportedFps_, fps))
        throw new MyException("FPS not supported.");

    fps_ = fps;

    if ((err_ = dc1394_video_set_framerate(camera_, fps_)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Unable to set the FPS.");
    }
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::getGainRange(unsigned int &min, unsigned int &max) throw(MyException*) {

    if ((err_ = dc1394_feature_get_boundaries(camera_, DC1394_FEATURE_GAIN, &min, &max)) != DC1394_SUCCESS)
        throw new MyException("Unable to detect min/max camera gain.");
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::getStdShutterRange(unsigned int &min, unsigned int &max) throw(MyException*) {

    if ((err_ = dc1394_feature_get_boundaries(camera_, DC1394_FEATURE_SHUTTER, &min, &max)) != DC1394_SUCCESS)
        throw new MyException("Unable to detect min/max camera standard shutter.");
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::getBrightnessRange(unsigned int &min, unsigned int &max) throw(MyException*) {

    if ((err_ = dc1394_feature_get_boundaries(camera_, DC1394_FEATURE_BRIGHTNESS, &min, &max)) != DC1394_SUCCESS)
        throw new MyException("Unable to detect min/max camera brightness.");
}

// ---------------------------------------------------------------------- //

unsigned int Dc1394Camera::getGain() throw(MyException*) {

    unsigned int value;
    if ((err_ = dc1394_feature_get_value(camera_, DC1394_FEATURE_GAIN, &value)) != DC1394_SUCCESS)
        throw new MyException("Unable to get camera gain.");
    else
        return value;
}

// ---------------------------------------------------------------------- //

unsigned int Dc1394Camera::getStdShutter() throw(MyException*) {

    unsigned int value;
    if ((err_ = dc1394_feature_get_value(camera_, DC1394_FEATURE_SHUTTER, &value)) != DC1394_SUCCESS)
        throw new MyException("Unable to get camera standard shutter.");
    else
        return value;
}

// ---------------------------------------------------------------------- //

unsigned int Dc1394Camera::getBrightness() throw(MyException*) {

    unsigned int value;
    if ((err_ = dc1394_feature_get_value(camera_, DC1394_FEATURE_BRIGHTNESS, &value)) != DC1394_SUCCESS)
        throw new MyException("Unable to get camera brightness.");
    else
        return value;
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::setGain(int gain) throw(MyException*) {

    if ((err_ = dc1394_feature_set_value(camera_, DC1394_FEATURE_GAIN, gain)) != DC1394_SUCCESS)
        throw new MyException("Unable to set camera gain.");
    gainBkp_ = gain;
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::setStdShutter(int shutter) throw(MyException*) {

    if ((err_ = dc1394_feature_set_value(camera_, DC1394_FEATURE_SHUTTER, shutter)) != DC1394_SUCCESS)
        throw new MyException("Unable to set camera standard shutter.");
    stdShutterBkp_ = shutter;
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::setBrightness(int brightness) throw(MyException*) {

    if ((err_ = dc1394_feature_set_value(camera_, DC1394_FEATURE_BRIGHTNESS, brightness)) != DC1394_SUCCESS)
        throw new MyException("Unable to set camera brightness.");
    brightnessBkp_ = brightness;
}

// ---------------------------------------------------------------------- //

std::string Dc1394Camera::getCameraName() {

    std::string name(camera_->vendor);
    name = name + " " + camera_->model;
    return name;
}

// ---------------------------------------------------------------------- //

std::string Dc1394Camera::getCameraGuid() const {

    char* guid = new char[20];
    sprintf(guid, "%llx", camera_->guid);
    return std::string(guid);
}

// ---------------------------------------------------------------------- //

std::string Dc1394Camera::getCameraNameAndGuid() {

    std::string str = this->getCameraName() + " (" + this->getCameraGuid() + ")";
    return str;
}

// ---------------------------------------------------------------------- //

void Dc1394Camera::setAoi(dc1394video_mode_t mode, Aoi* aoi) throw(MyException*) {

    if ((err_ = dc1394_video_set_mode(camera_, mode)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Unable to set camera scalable video mode.");
    }
    if ((err_ = dc1394_format7_set_roi(camera_, mode, DC1394_COLOR_CODING_MONO8, DC1394_QUERY_FROM_CAMERA, aoi->x_, aoi->y_, aoi->width_, aoi->height_)) != DC1394_SUCCESS) {
        cleanup(false);
        throw new MyException("Unable to set AOI.");
    }
    LOG (INFO) << "Applying AOI to camera " << getCameraName() << " (" << getCameraGuid() << ").";
    LOG (INFO) << "AOI: x=" << aoi->x_<< ", y=" << aoi->y_ << ", width=" << aoi->width_ << ", height=" << aoi->height_ << ".";
}

// ---------------------------------------------------------------------- //

FpsEvaluator* Dc1394Camera::getFpsEvaluator() { return fpsEvaluator_; }

void Dc1394Camera::setAoi(Aoi aoi) { aoi_ = aoi; }
Aoi* Dc1394Camera::getAoi() { return &aoi_; }

void Dc1394Camera::useAoi(bool b) { useAoi_ = b; }
bool Dc1394Camera::useAoi() { return useAoi_; }

dc1394camera_t* Dc1394Camera::getCamera() { return camera_; }
dc1394video_mode_t Dc1394Camera::getResolution() { return resolution_; }
dc1394framerate_t Dc1394Camera::getFps() { return fps_; }
