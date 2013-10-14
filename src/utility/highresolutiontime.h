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

#ifndef HIGHRESOLUTIONTIME_H
#define HIGHRESOLUTIONTIME_H

#ifdef WIN32   // Windows system specific
    #include <windows.h>
#else          // Unix based system specific
    #include <sys/time.h>
#endif

/**
 * \brief High resolution time based on gettimeofday() (Unix only).
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class HighResolutionTime {

private:

    /** Starting time in us. */
    double startTimeInUs_;
    /** Ending time in us. */
    double endTimeInUs_;
    /** True if the timer is stopped. */
    bool stopped_;

    /** Internal variable. */
    timeval startCount_;
    /** Internal variable. */
    timeval endCount_;

public:

    /** Constructor. */
    HighResolutionTime();
    /** Desctructor. */
    ~HighResolutionTime();

    /** Initializes the timer. */
    void initialize();

    /** Starts the timer. */
    void start();
    /** Stops the timer. */
    void stop();

    /** Returns true if the timer is stopped. */
    bool isStopped();

    /** Returns elapsed time in seconds. */
    double getElapsedTime();
    /** Returns elapsed time in seconds. */
    double getElapsedTimeInSec();
    /** Returns elapsed time in milliseconds. */
    double getElapsedTimeInMs();
    /** Returns elapsed time in microseconds. */
    double getElapsedTimeInUs();
};

#endif // HIGHRESOLUTIONTIME_H
