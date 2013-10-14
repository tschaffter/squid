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

#include "cameraconfiguration.h"
#include "dc1394utility.h"
#include <sstream>
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PUBLIC METHODS

CameraConfiguration::CameraConfiguration(std::string config) {

    std::stringstream ss(config);
    std::string buffer;

    ss >> guid_;
    ss >> resolution_;
    ss >> fps_;
    ss >> gain_;
    ss >> shutter_;
    ss >> brightness_;

    ss >> aoi_.x_;
    ss >> aoi_.y_;
    ss >> aoi_.width_;
    ss >> aoi_.height_;
    ss >> useAoi_;
}

// ----------------------------------------------------------------------

CameraConfiguration::CameraConfiguration(Dc1394Camera* camera) {

    guid_ = camera->getCameraGuid();
    resolution_ = dc1394ToSringResolution(camera->getResolution());
    fps_ = dc1394ToStringFps(camera->getFps());
    gain_ = camera->getGain();
    shutter_ = camera->getStdShutter();
    brightness_ = camera->getBrightness();

    aoi_.x_ = camera->getAoi()->x_;
    aoi_.y_ = camera->getAoi()->y_;
    aoi_.width_ = camera->getAoi()->width_;
    aoi_.height_ = camera->getAoi()->height_;
    useAoi_ = camera->useAoi();
}

// ----------------------------------------------------------------------

std::vector<CameraConfiguration*> CameraConfiguration::getAllCameraConfigurations(std::string config) {

    std::vector<CameraConfiguration*> cameras;

    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = config.find(';', pos)) != std::string::npos) {
        std::string substring(config.substr(prev_pos, pos-prev_pos));
        cameras.push_back(new CameraConfiguration(substring));
        prev_pos = ++pos;
    }
    std::string substring(config.substr(prev_pos, pos-prev_pos)); // Last word
    cameras.push_back(new CameraConfiguration(substring));

    return cameras;
}

// ----------------------------------------------------------------------

std::string CameraConfiguration::getAllCameraConfigurations(CameraManager* cmanager) {

    const unsigned int numCameras = cmanager->getNumCameras();
    std::string text = "";

    for (unsigned int i = 0; i < numCameras; i++) {
        Dc1394Camera* camera = cmanager->getCamera(i);
        text += (new CameraConfiguration(camera))->toString();
        if (i != numCameras - 1)
            text += ";";
    }
    return text;
}

// ----------------------------------------------------------------------

CameraConfiguration* getCameraConfiguration(std::string config) {

    return new CameraConfiguration(config);
}

// ----------------------------------------------------------------------

std::string CameraConfiguration::toString() {

    std::stringstream ss;
    ss << guid_;
    ss << " " + resolution_;
    ss << " " + fps_;
    ss << " ";
    ss << gain_;
    ss << " ";
    ss << shutter_;
    ss << " ";
    ss << brightness_;
    ss << " ";
    ss << aoi_.x_;
    ss << " ";
    ss << aoi_.y_;
    ss << " ";
    ss << aoi_.width_;
    ss << " ";
    ss << aoi_.height_;
    ss << " ";
    ss << useAoi_;

    return ss.str();
}

// ----------------------------------------------------------------------

void CameraConfiguration::configureAllCameras(CameraManager* cmanager, std::string config) {

    std::vector<CameraConfiguration*> allConfig = getAllCameraConfigurations(config);

    const unsigned int numCamerasDetected = cmanager->getNumCameras();
    const unsigned int numCameraConfigurations = allConfig.size();

    if (numCamerasDetected < numCameraConfigurations)
        LOG (INFO) << "No configuration found for at least one camera.";

    Dc1394Camera* camera = NULL;
    CameraConfiguration* cameraConfig = NULL;
    for (unsigned int i = 0; i < numCameraConfigurations; i++) {
        cameraConfig = allConfig[i];
        camera = cmanager->getCamera(cameraConfig->guid_);
        if (camera == NULL)
            continue;

        cameraConfig->setCameraConfiguration(camera);
    }
}

// ----------------------------------------------------------------------

void CameraConfiguration::setCameraConfiguration(Dc1394Camera* camera) {

    LOG (INFO) << "Opening configuration for camera " << camera->getCameraName() << " (" << camera->getCameraGuid() << ").";

    camera->setResolution(stringToDc1394Resolution(resolution_));
    camera->setFps(stringToDc1394Fps(fps_));
    camera->setGain(gain_);
    camera->setStdShutter(shutter_);
    camera->setBrightness(brightness_);
    camera->getAoi()->x_ = aoi_.x_;
    camera->getAoi()->y_ = aoi_.y_;
    camera->getAoi()->width_ = aoi_.width_;
    camera->getAoi()->height_ = aoi_.height_;
    camera->useAoi(useAoi_);

    if (useAoi_) {
        camera->setAoi(stringToDc1394Resolution("DC1394_VIDEO_MODE_FORMAT7_0"), camera->getAoi());
        camera->useAoi(true);
    }
}
