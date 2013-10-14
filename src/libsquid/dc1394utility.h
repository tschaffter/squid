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

#ifndef DC1394UTILITY_H
#define DC1394UTILITY_H

#include "myexception.h"
#include "dc1394/dc1394.h"
#include <vector>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Implements useful methods to manipulate dc1394 format.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
// ======================================================================
// VIDEO MODE UTILITY METHODS

/** Returns the camera resolution as string. */
std::string dc1394ToSringResolution(const dc1394video_mode_t resolution) throw(MyException*);
/** Returns dc1394video_mode_t object from string resolution. */
dc1394video_mode_t stringToDc1394Resolution(const std::string strResolution) throw(MyException*);

/** Gets all resolutions supported by the camera. */
dc1394video_modes_t getSupportedResolutions(dc1394camera_t* camera) throw(MyException*);

/** Gets all resolutions supported by the camera as string. */
std::vector<std::string> getSupportedResolutionsLabels(dc1394camera_t* camera) throw(MyException*);

/** Gets highest MONO8 resolution supported by the camera.*/
dc1394video_mode_t getHighestSupportedResolution(dc1394camera_t* camera, dc1394video_modes_t* supportedResolutions, dc1394color_coding_t coding) throw(MyException*);

/** Checks if the given resolution is support by the camera. */
bool isResolutionSupported(dc1394video_modes_t* supportedResolutions, dc1394video_mode_t resolution);

// ======================================================================
// FRAMERATE UTILITY METHODS

/** Returns the camera FPS as string. */
std::string dc1394ToStringFps(const dc1394framerate_t fps) throw(MyException*);
/** Returns dc1394framerate_t object from string FPS. */
dc1394framerate_t stringToDc1394Fps(const std::string strFps) throw(MyException*);

/** Converts FPS to period in us. */
unsigned int dc1394FpsToPeriodInUs(dc1394framerate_t fps);

/** Gets all FPS supported by the camera. */
dc1394framerates_t getSupportedFps(dc1394camera_t* camera, dc1394video_mode_t resolution) throw(MyException*);

/** Gets all FPS supported by the camera as string. */
std::vector<std::string> getSupportedFpsLabels(dc1394camera_t* camera, dc1394video_mode_t resolution) throw(MyException*);

/** Gets highest FPS supported by the camera and current resolution. */
dc1394framerate_t getHighestSupportedFps(dc1394framerates_t* supportedFps) throw(MyException*);

/** Checks if the given FPS is supported by the camera (depending on current resolution). */
bool isFpsSupported(dc1394framerates_t* supportedFps, dc1394framerate_t fps);

// ======================================================================
// IMAGE CODING UTILITY METHODS

/** Returns the image coding as string. */
std::string dc1394ToSringCoding(const dc1394color_coding_t coding) throw(MyException*);
/** Returns a dc1394color_coding_t object from string image coding. */
dc1394color_coding_t stringToDc1394Coding(const std::string coding) throw(MyException*);

// ======================================================================
// FRAME UTILITY METHODS

/** Converts a 8-bit grayscale image to PGM format. */
void grayscale8bitsToPgm(const char* filename, unsigned char* image, const unsigned int width, const unsigned int height) throw(MyException*);
/** Converts a 8-bit grayscale image to TIFF format. */
void grayscale8bitsToTiff(const char * filename, unsigned char* image, const unsigned int width, const unsigned int height) throw(MyException*);

} // end namespace squid

#endif // DC1394UTILITY_H
