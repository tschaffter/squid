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

#ifndef CAMERACONFIGURATION_H
#define CAMERACONFIGURATION_H

#include "dc1394camera.h"
#include "cameramanager.h"
#include <vector>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Converts the configuration of a dc1394 camera from/to settings file.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class CameraConfiguration {

public:

    /** Guid of the camera. Declared as public for simplicity. */
    std::string guid_;
    /** Resolution of the camera. Declared as public for simplicity. */
    std::string resolution_;
    /** FPS of the camera. Declared as public for simplicity. */
    std::string fps_;
    /** Gain of the camera. Declared as public for simplicity. */
    unsigned int gain_;
    /** Shutter of the camera. Declared as public for simplicity. */
    unsigned int shutter_;
    /** Brightness of the camera. Declared as public for simplicity. */
    unsigned int brightness_;
    /** AOI (if any used). */
    Aoi aoi_;

    /** Set to true to use an AOI. */
    bool useAoi_;

    /** Constructor from camera settings. Parameter values separated by a space. */
    CameraConfiguration(std::string config);
    /** Constructor using the settings from an existing camera. */
    CameraConfiguration(Dc1394Camera* camera);
    /** Destructor. */
    ~CameraConfiguration() {}

    /** Returns the configuration of the camera. Parameter values separated by a space. */
    static CameraConfiguration* getCameraConfiguration(std::string config);
    /** Sets the configuration of the camera from the settings of the given camera. */
    void setCameraConfiguration(Dc1394Camera* camera);
    /** Returns the configuration of each camera (separated by a semi-colon ';'). */
    static std::vector<CameraConfiguration*> getAllCameraConfigurations(std::string config);
    /** Returns the configuration of each camera from the CameraManager */
    static std::string getAllCameraConfigurations(CameraManager* cmanager);

    /** toString() method. */
    std::string toString();

    /** Configures each camera through the CameraManager. */
    static void configureAllCameras(CameraManager* cmanager, std::string config);
};

} // end namespace squid

#endif // CAMERACONFIGURATION_H
