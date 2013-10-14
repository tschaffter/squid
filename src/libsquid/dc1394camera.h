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

#ifndef DC1394CAMERA_H
#define DC1394CAMERA_H

#include "dc1394frame.h"
#include "dc1394/dc1394.h"
#include "aoi.h"
#include "myexception.h"
#include "fpsevaluator.h"
#include <QObject>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Wrapper to allow an easier high-level control of one dc1394 camera.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Dc1394Camera : public QObject {

    Q_OBJECT

private:

    /** Reference to DC1394 camera. */
    dc1394camera_t* camera_;
    /** Error support. */
    dc1394error_t err_;

    /** Camera resolution. */
    dc1394video_mode_t resolution_;
    /** Camera FPS (framerate). */
    dc1394framerate_t fps_;
    /** Camera gain backup. */
    unsigned int gainBkp_;
    /** Camera standard shutter backup. */
    unsigned int stdShutterBkp_;
    /** Camera brightness backup. */
    unsigned int brightnessBkp_;

    /** Structure filled with the resolutions supported by the camera. */
    dc1394video_modes_t supportedResolutions_;
    /** Structure filled with the FPS supported by the camera and the current resolution. */
    dc1394framerates_t supportedFps_;

    /** Compute the camera FPS. */
    FpsEvaluator* fpsEvaluator_;

    /** Is true if AOI is currently used. */
    bool useAoi_;
    /** Area of interest (AOI). */
    Aoi aoi_;

public:

    /** Camera frame. */
    Dc1394Frame frame_;

    /** Constructor. */
    Dc1394Camera(dc1394camera_t* camera);
    /** Destructor. */
    ~Dc1394Camera();

    /** Initialization. */
    void initialize() throw(MyException*);
    /** Stop the camera transmission and cleanup everything (triggers, etc.). */
    void cleanup(bool verbose = true);
    /** Get DC1394 camera instance. */
    dc1394camera_t* getCamera();

    /** Logical operator == (equality). */
    bool operator==(const Dc1394Camera& c) const;

    /** Get the name of the camera. */
    std::string getCameraName();
    /** Get the guid of the camera. */
    std::string getCameraGuid() const;
    /** Get a string in the format "camera_name (camera_guid)". */
    std::string getCameraNameAndGuid();

    /** Get gain range [min, max] directly from camera. */
    void getGainRange(unsigned int &min, unsigned int &max) throw(MyException*);
    /** Get standard shutter range [min, max] directly from camera. */
    void getStdShutterRange(unsigned int &min, unsigned int &max) throw(MyException*);
    /** Get brightness range [min, max] directly from camera. */
    void getBrightnessRange(unsigned int &min, unsigned int &max) throw(MyException*);

    /** Get gain value directly from camera. */
    unsigned int getGain() throw(MyException*);
    /** Get standard shutter value directly from camera. */
    unsigned int getStdShutter() throw(MyException*);
    /** Get brightness value directly from camera. */
    unsigned int getBrightness() throw(MyException*);

    /** Get FPS evaluator. */
    FpsEvaluator* getFpsEvaluator();

    /** Set AOI. */
    void setAoi(dc1394video_mode_t mode, Aoi* aoi) throw(MyException*);

public slots:

    /** Reset camera (required to apply the modifications before running it). */
    void setupCamera(std::string dc1394) throw(MyException*);
    /** Print some information about this camera. */
    void printCameraInfo();

    /** Set the camera to use the software trigger. */
    void setSoftwareTrigger();
    /** Reset camera bus. */
    void resetBus();
    /** Free camera. */
    void freeCamera();

    /** Set gain value directly to camera. */
    void setGain(int gain) throw(MyException*);
    /** Set standard shutter value directly to camera. */
    void setStdShutter(int shutter) throw(MyException*);
    /** Set brightness value directly to camera. */
    void setBrightness(int brightness) throw(MyException*);

    /** Set camera resolution. */
    void setResolution(dc1394video_mode_t resolution) throw(MyException*);
    /** Get camera resolution. */
    dc1394video_mode_t getResolution();

    /** Set camera FPS. */
    void setFps(dc1394framerate_t fps) throw(MyException*);
    /** Get camera FPS. */
    dc1394framerate_t getFps();

    /** Set AOI. */
    void setAoi(Aoi aoi);
    /** Get AOI. */
    Aoi* getAoi();

    /** Set to true to use AOI. */
    void useAoi(bool b);
    /** Return true if AOI used. */
    bool useAoi();

private:

    /** Setup camera in DC1394A mode (FireWire400). */
    void setDc1394a();
    /** Setup camera in DC1394B mode (FireWire800). */
    void setDc1394b();
};

} // end namespace squid

#endif // DC1394CAMERA_H
