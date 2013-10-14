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

#include "fdtimer.h"
#include <sys/time.h>
#include <glog/logging.h>

// ======================================================================
// PUBLIC METHODS

FdTimer::FdTimer() {

    timer_ = 0;
    running_ = false;
}

// ----------------------------------------------------------------------

FdTimer::~FdTimer() {

}

// ----------------------------------------------------------------------

void FdTimer::start() throw(MyException*) {

    if (running_)
        throw new MyException("Timer is already running.");

    if ((timer_ = timerfd_create(CLOCK_MONOTONIC, 0)) < 0)
        throw new MyException("Unable to start timer: timerfd_create() failed.");

    if (timerfd_settime(timer_, 0, &timerSpec_, NULL) < 0)
        throw new MyException("Unable to start timer: timerfd_settime() failed.");

    running_ = true;
}

// ----------------------------------------------------------------------

void FdTimer::stop() throw(MyException*) {

    if (!running_)
        return;

    struct itimerspec end;
    end.it_interval.tv_sec = 0;
    end.it_interval.tv_nsec = 0;
    end.it_value.tv_sec = 0;
    end.it_value.tv_nsec = 0;

    if (timerfd_settime(timer_, 0, &end, NULL) == -1)
        throw new MyException("Unable to stop timer.");

    running_ = false;
}

// ----------------------------------------------------------------------

void FdTimer::initialize(time_t sec, long nsec) throw(MyException*) {

    // set interval
    setInterval(sec, nsec);

    // set initial expiration
    setValue(sec, nsec);
//    struct timespec now;
//    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
//        throw new MyException("Unable to initialize timer.");
//    setValue(now.tv_sec + sec, now.tv_nsec + nsec);
}

// ----------------------------------------------------------------------

void FdTimer::setInterval(time_t sec, long nsec) {

    while (nsec >= 1000000000) {
        sec++;
        nsec -= 1000000000;
    }

    timerSpec_.it_interval.tv_sec = sec;
    timerSpec_.it_interval.tv_nsec = nsec;
}

// ----------------------------------------------------------------------

void FdTimer::setValue(time_t sec, long nsec) {

    while (nsec >= 1000000000) {
        sec++;
        nsec -= 1000000000;
    }

    timerSpec_.it_value.tv_sec = sec;
    timerSpec_.it_value.tv_nsec = nsec;
}

// ======================================================================
// GETTERS AND SETTERS

bool FdTimer::isRunning() { return running_; }

int FdTimer::getTimer() { return timer_; }
