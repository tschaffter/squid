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

#ifndef AOI_H
#define AOI_H

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Defines an area of interest (AOI) for the camera.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Aoi {

public:

    /** X coordinate of the top-left coordinate of the AOI. Declared as public for simplicity. */
    unsigned int x_;
    /** Y coordinate of the top-left coordinate of the AOI. Declared as public for simplicity. */
    unsigned int y_;
    /** Width of the AOI. Declared as public for simplicity. */
    unsigned int width_;
    /** Height of the AOI. Declared as public for simplicity. */
    unsigned int height_;

    /** Constructor. */
    Aoi();
    /** Destructor. */
    ~Aoi() {}
};

} // end namespace squid

#endif // AOI_H
