/**
 * Copyright (c) 2010-2012 Thomas Schaffter
 *
 * We release this software open source under an MIT license (see below). If this
 * software was useful for your scientific work, please cite our paper(s) listed
 * on http://tschaffter.ch/projects/squid/.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#include "triggermanager.h"
#include "experimenttime.h"
#include "cameramanager.h"

#include <dc1394/vendor/avt.h>
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PUBLIC METHODS

TriggerManager::TriggerManager() {

    timer_ = NULL;
    periodInUs_ = 0;

    initialize();
}

// ----------------------------------------------------------------------

TriggerManager::~TriggerManager() {

    mutex_.lock();
    abort_ = true;
    if (timer_ != NULL)
        timer_->stop();
    // to wake up the thread if it's sleeping
    // (the thread is put to sleep when it has nothing to do)
    condition_.wakeOne();
    mutex_.unlock();
    // to wait until run() has exited before the base class destructor is invoked
    wait();
}

// ----------------------------------------------------------------------

void TriggerManager::initialize() {

    timer_ = new HighResolutionTime();
    triggerId_ = 0;
    preTriggerAction_ = (pfv) &TriggerManager::defaultPreTriggerAction;
    postTriggerAction_ = (pfv) &TriggerManager::defaultPostTriggerAction;
//    restart_ = false;
    abort_ = false;
}

// ----------------------------------------------------------------------

void TriggerManager::run() {

    triggerId_ = 0;

    double tInUs = 0;
    double tBaseInUs = 0;
    double tOffsetUs = 0;

    timer_->start();

    forever {
        if (abort_) {
            emit done();
            return;
        }

        tInUs = timer_->getElapsedTimeInUs() - tOffsetUs;
        if (tInUs - tBaseInUs > periodInUs_) {
            tBaseInUs = tInUs;
            LOG(INFO) << tInUs/1000;
            trigger(triggerId_++);
        }

        mutex_.lock();
        // we call QWaitCondition::wait() to put the thread to sleep by calling, unless restart is true.
        if (!restart_) {
            double tBkpUs = timer_->getElapsedTimeInUs();
            condition_.wait(&mutex_);
            tOffsetUs += timer_->getElapsedTimeInUs() - tBkpUs;
        }
        mutex_.unlock();
    }
}

// ----------------------------------------------------------------------

void TriggerManager::trigger(unsigned int triggerId) {

//    ExperimentTime* time = ExperimentTime::getInstance();

//    dc1394bool_t polarity;
//    unsigned int mode;
    dc1394bool_t pinstate;
    
    try {
        // checks GPOut2 if the camera can now accept a trigger
        // WARNING: be sure to have configured GPOu2 to generate "WaitingForTrigger" signal
        // done in dc1394camera.cpp

        // the nex line is only to get values polarity, mode and pinstate
//         dc1394_avt_get_io(CameraManager::getInstance()->getCamera(0)->getCamera(), 0x324, &polarity, &mode, &pinstate);

        // whether the camera can accept a trigger
        if (true or pinstate) { // remove true to use the value of pinstate

            (this->*preTriggerAction_)(triggerId);
            emit triggered(triggerId);
            (this->*postTriggerAction_)(triggerId);

        } else
            LOG(WARNING) << "===== CAMERA TRIGGER " << triggerId << " REFUSED =====";

    } catch (MyException* e) {
        LOG(WARNING) << "Trigger failed: " << e->getMessage();
    }
}

// ----------------------------------------------------------------------

void TriggerManager::stop() {

        setAbort(true);
}

// ----------------------------------------------------------------------

void TriggerManager::wake() {

    condition_.wakeOne();
}

// ----------------------------------------------------------------------

void TriggerManager::defaultPreTriggerAction(int /*triggerId*/) {

//    LOG(INFO) << "Calling TriggerManager::defaultPreTriggerAction(): id = " << triggerId;
}

// ----------------------------------------------------------------------

void TriggerManager::defaultPostTriggerAction(int /*triggerId*/) {

//    LOG(INFO) << "Calling TriggerManager::defaultPostTriggerAction(): id = " << triggerId;
}

// ======================================================================
// SETTERS AND GETTERS

void TriggerManager::setPeriodInUs(unsigned int periodInUs) { periodInUs_ = periodInUs; }
unsigned int TriggerManager::getPeriodInUs() { return periodInUs_; }

void TriggerManager::setPreTriggerAction(pfv functionPtr) { preTriggerAction_ = functionPtr; }
pfv TriggerManager::getPreTriggerAction() { return preTriggerAction_; }

void TriggerManager::setPostTriggerAction(pfv functionPtr) { postTriggerAction_ = functionPtr; }
pfv TriggerManager::getPostTriggerAction() { return postTriggerAction_; }

void TriggerManager::setRestart(bool b) { restart_ = b; }
void TriggerManager::setAbort(bool b) { abort_ = b; }

void TriggerManager::setTriggerId(unsigned int triggerId) { triggerId_ = triggerId; }
unsigned int TriggerManager::getTriggerId() { return triggerId_; }
