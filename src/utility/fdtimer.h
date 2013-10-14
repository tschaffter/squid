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

#ifndef FDTIMER_H
#define FDTIMER_H

#include "myexception.h"

#include <sys/timerfd.h>

/**
 * \brief Creates a timer based on timerfd.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class FdTimer {

private:

    /** File descriptor returned by timerfd_create(). */
    int timer_;
    /** Configuration of the timer. */
    struct itimerspec timerSpec_;

    /** Is true if the timer is running. */
    bool running_;

public:

    /** Constructor. */
    FdTimer();
    /** Destructor. */
    ~FdTimer();

    /** Start the timer. */
    void start() throw(MyException*);
    /** Stops the timer. */
    void stop() throw(MyException*);

    /** Initializes the timer with sec and nsec. */
    void initialize(time_t sec, long nsec) throw(MyException*);

    /** Set the interval between two expiration in sec and nsec. */
    void setInterval(time_t sec, long nsec);
    /** Set the first experiation in sec and nsec. */
    void setValue(time_t sec, long nsec);

    /** Returns true if the timer is running. */
    bool isRunning();

    /** Get the file descriptor returned by timerfd_create(). */
    int getTimer();
};

#endif // FDTIMER_H
