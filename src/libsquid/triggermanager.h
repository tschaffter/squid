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

#ifndef TRIGGERMANAGER_H
#define TRIGGERMANAGER_H

#include "myexception.h"
#include "highresolutiontime.h"
#include <vector>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

class TriggerManager;

/** Declares a typedef for pointer to a function returning void and taking an int argument (frame index). */
typedef void (TriggerManager::*pfv)(int);

/**
 * \brief Generates a trigger every interval_ in us using a dedicated thread.
 *
 * @version January 14, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class TriggerManager : public QThread {

    Q_OBJECT

public:

    /** Constructor */
    TriggerManager();
    /** Destructor */
    ~TriggerManager();

    /** Initialization */
    void initialize();

    /** Set trigger period in us (duration between two triggers). */
    void setPeriodInUs(unsigned int period);
    /** Get trigger period in us (duration between two triggers). */
    unsigned int getPeriodInUs();

    /** Set pre-trigger function. */
    void setPreTriggerAction(pfv functionPtr);
    /** Get pre-trigger function. */
    pfv getPreTriggerAction();

    /** Set post-trigger function. */
    void setPostTriggerAction(pfv functionPtr);
    /** Get post-trigger function. */
    pfv getPostTriggerAction();

    /** Default function to call to perform BEFORE sending a trigger (do nothing by default). */
    void defaultPreTriggerAction(int triggerId);
    /** Action to perform AFTER sending a trigger (do nothing by default). */
    void defaultPostTriggerAction(int triggerId);

    /** Set trigger id. */
    void setTriggerId(unsigned int triggerId);
    /** Get trigger id. */
    unsigned int getTriggerId();

public slots:

    /** Start to generate triggers */
    void run();
    /** Stop to generation of triggers */
    void stop();
    /** Call setRestart(false) to put the thread to sleep */
    void setRestart(bool b);
    /** Call setAbort(true) to stop the thread */
    void setAbort(bool b);
    /** If the thread has been put to sleep with setRestart(false), call this method to wake it */
    void wake();

signals:

    /** Sent when a trigger is generated */
    void triggered(unsigned int triggerId);
    /** Emits a signal when the trigger manager is aborted. */
    void done();

private:    

    /** Called each time t - base > periodInUs_. */
    void trigger(unsigned int triggerId);

    /** High resolution timer. */
    HighResolutionTime* timer_;
    /** Interval in us between two triggers */
    unsigned int periodInUs_;

    /** Trigger id */
    unsigned int triggerId_;

    /** Function pointer to call just before sending a trigger */
    pfv preTriggerAction_;
    /** Function pointer to call just after sending a trigger */
    pfv postTriggerAction_;

    /** Mutex */
    QMutex mutex_;
    /** Condition for waiting */
    QWaitCondition condition_;

    /** If false, the thread is put to sleep */
    bool restart_;
    /** If true, the thread is stopped */
    bool abort_;
};

} // end namespace squid

#endif // TRIGGERMANAGER_H
