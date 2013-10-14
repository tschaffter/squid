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

#ifndef DC1394FRAME_H
#define DC1394FRAME_H

#include "dc1394/dc1394.h"

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Contains the information of one frame grabbed from a dc1349 camera.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Dc1394Frame {

public:

    /** dc1394 frame. */
    dc1394video_frame_t* frame_;
    /**
     * dc1394 frame width in pixels.
     * @deprecated
     */
    unsigned int width_;
    /**
     * dc1394 frame height in pixels.
     * @deprecated
     */
    unsigned int height_;

    /** Constructor */
    Dc1394Frame();
    /** Destructor */
    ~Dc1394Frame() {}
};

} // end namespace squid

#endif // DC1394FRAME_H
