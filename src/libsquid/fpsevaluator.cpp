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

#include "cameramanager.h"
#include "fpsevaluator.h"
#include "fdtimer.h"
#include "myexception.h"
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PUBLIC METHODS

FpsEvaluator::FpsEvaluator() {

    initialize();
}

// ----------------------------------------------------------------------

FpsEvaluator::~FpsEvaluator() {

    mutex_.lock();
    abort_ = true;
    // to wake up the thread if it's sleeping
    // (the thread is put to sleep when it has nothing to do)
    condition_.wakeOne();
    mutex_.unlock();
    // to wait until run() has exited before the base class destructor is invoked
    wait();
}

// ----------------------------------------------------------------------

void FpsEvaluator::initialize() {

    prevNumFrames_ = 0;
    currentNumFrames_ = 0;
    baseTimeInMilliseconds_ = 0;
    intervalInMs_ = 2000;
    restart_ = false;
    abort_ = false;
}

// ----------------------------------------------------------------------

void FpsEvaluator::run() {

    try {
        FdTimer timer;
        timer.initialize(0, intervalInMs_ * 1000000);
        timer.start();
        int fd = timer.getTimer();

        while (!abort_) {
            uint64_t numTimeout;
            read(fd, &numTimeout, sizeof(numTimeout));
            if (numTimeout > 1)
                LOG(WARNING) << "Timer missed " << (numTimeout - 1) << " events.";

            emit fpsUpdated((1000.*((float)currentNumFrames_-(float)prevNumFrames_))/(float)intervalInMs_);

            mutex_.lock();
                // save variable for next time
                this->prevNumFrames_ = currentNumFrames_;
            mutex_.unlock();
        }

        timer.stop();
        emit fpsUpdated(0);

    } catch (MyException* e) {
        LOG(WARNING) << "FpsEvaluator failed: " << e->getMessage();
    }
}

// ----------------------------------------------------------------------

void FpsEvaluator::stop() {

        setAbort(true);
}

// ----------------------------------------------------------------------

void FpsEvaluator::wake() {

    condition_.wakeOne();
}

// ----------------------------------------------------------------------

void FpsEvaluator::incrementNumFrames() {

    mutex_.lock();
    currentNumFrames_++;
    mutex_.unlock();
}

// ======================================================================
// GETTERS AND SETTERS

void FpsEvaluator::setIntervalInMs(unsigned int intervalInMs) { intervalInMs_ = intervalInMs; }
void FpsEvaluator::setRestart(bool b) { restart_ = b; }
void FpsEvaluator::setAbort(bool b) { abort_ = b; }
