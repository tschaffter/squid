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

#include "fdtriggermanager.h"
#include "fdtimer.h"
#include "rt.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PRIVATE METHODS

void* FdTriggerManager::processThread(void* obj) {

    FdTriggerManager* tmanager = reinterpret_cast<FdTriggerManager*>(obj);

    LOG(INFO) << "Promoting trigger manager to RT priority.";
    promoteRT();

    FdTimer timer;
    timer.initialize(0, tmanager->getIntervalInUs() * 1000);
    timer.start();  // generates the timeouts
    int fd = timer.getTimer();

    int triggerId = 0;
    while (!tmanager->isAbort()) {
        pthread_mutex_lock(&tmanager->mutex_);
        if (!tmanager->pause_) {
            pthread_mutex_unlock(&tmanager->mutex_);

            uint64_t numTimeout;
            read(fd, &numTimeout, sizeof(numTimeout));
//            if (numTimeout > 1)
//                LOG(WARNING) << "Trigger manager timer missed " << (numTimeout - 1) << " events.";

            // call the trigger function
            pthread_mutex_lock(&tmanager->mutex_);
            tmanager->trigger(triggerId++);
        }

        // pause the trigger manager ?
        while (tmanager->pause_) {
            if (pthread_cond_wait(&tmanager->cond_, &tmanager->mutex_)) // wait for resume signal
                throw new MyException("Unable to suspend trigger manager: pthread_cond_wait() failed.");
            LOG(INFO) << "Resuming trigger manager.";
        }
        pthread_mutex_unlock(&tmanager->mutex_);
    }

    // abortion
    emit tmanager->done();

    timer.stop();

    pthread_mutex_lock(&tmanager->mutex_);
    tmanager->running_ = false;
    pthread_mutex_unlock(&tmanager->mutex_);

    return NULL;
}

// ----------------------------------------------------------------------

void FdTriggerManager::trigger(const unsigned int triggerId) {

    (this->*preTriggerAction_)(triggerId);
    // there is no warranty that the slot(s) responding to this signal
    // will be executed before the post trigger method.
    emit triggered(triggerId);
    (this->*postTriggerAction_)(triggerId);
}

// ======================================================================
// PUBLIC METHODS

FdTriggerManager::FdTriggerManager() {

    if (pthread_mutex_init(&mutex_, NULL) == -1)
        throw new MyException("Unable to pthread_mutex_init().");
    if (pthread_cond_init(&cond_, NULL) == -1)
        throw new MyException("Unable to pthread_cond_init().");

    initialize();
}

// ----------------------------------------------------------------------

FdTriggerManager::~FdTriggerManager() {

    if (pthread_cond_destroy(&cond_) == -1)
        throw new MyException("Unable to pthread_cond_destroy().");
    if (pthread_mutex_destroy(&mutex_) == -1)
        throw new MyException("Unable to pthread_mutex_destroy().");
}

// ----------------------------------------------------------------------

void FdTriggerManager::initialize() {

    intervalInUs_ = 50000;
    running_ = false;
    pause_ = false;
    abort_ = false;

    preTriggerAction_ = (pfv) &FdTriggerManager::defaultPreTriggerAction;
    postTriggerAction_ = (pfv) &FdTriggerManager::defaultPostTriggerAction;
}

// ----------------------------------------------------------------------

void FdTriggerManager::start() throw(MyException*) {

    if (running_)
        throw new MyException("Trigger manager is already running.");

    pthread_mutex_lock(&mutex_);
    abort_ = false;
    pause_ = false;

    if (pthread_create(&thread_, 0, FdTriggerManager::processThread, this))
        throw new MyException("Unable to start trigger manager thread: pthread_create() failed.");

    running_ = true;
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void FdTriggerManager::stop() throw(MyException*) {

    if (!running_)
        return;

    if (pause_)
        pause(false);
    pause_ = false;
    abort_ = true;
    pthread_join(thread_, NULL);
    running_ = false;
}

// ----------------------------------------------------------------------

void FdTriggerManager::pause(bool pause) throw(MyException*) {

    if (pause == pause_)
        return;

    if (pause) LOG(INFO) << "Suspending trigger manager.";
    else LOG(INFO) << "Resuming trigger manager.";

    pthread_mutex_lock(&mutex_);
    pause_ = pause;
    if (!pause) {
        if (pthread_cond_signal(&cond_))
            throw new MyException("Unable to resume trigger manager thread: pthread_cond_signal() failed.");
    }
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void FdTriggerManager::defaultPreTriggerAction(int /*triggerId*/) {}

// ----------------------------------------------------------------------

void FdTriggerManager::defaultPostTriggerAction(int /*triggerId*/) {}

// ======================================================================
// GETTERS AND SETTERS

bool FdTriggerManager::isAbort() { return abort_; }
bool FdTriggerManager::isRunning() { return running_; }

void FdTriggerManager::setIntervalInUs(long intervalInUs) { intervalInUs_ = intervalInUs; }
long FdTriggerManager::getIntervalInUs() { return intervalInUs_; }

void FdTriggerManager::setPreTriggerAction(pfv functionPtr) { preTriggerAction_ = functionPtr; }
pfv FdTriggerManager::getPreTriggerAction() { return preTriggerAction_; }

void FdTriggerManager::setPostTriggerAction(pfv functionPtr) { postTriggerAction_ = functionPtr; }
pfv FdTriggerManager::getPostTriggerAction() { return postTriggerAction_; }
