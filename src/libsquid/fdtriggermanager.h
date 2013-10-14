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

#ifndef FDTRIGGERMANAGER_H
#define FDTRIGGERMANAGER_H

#include "myexception.h"
#include <pthread.h>
#include <QObject>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

class FdTriggerManager;

/** Declares a typedef for pointer to a function returning void and taking an int argument (frame index). */
typedef void (FdTriggerManager::*pfv)(int);

/**
 * \brief Implements a software trigger manager for dc1394 cameras.
 *
 * The implementation uses a dedicated pthread using rtkit to promote the thread
 * to realtime (RT) priority.
 *
 * @version January 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class FdTriggerManager : public QObject {

    Q_OBJECT

private:

    /** Mutex for the thread. */
    pthread_mutex_t mutex_;
    /** Condition variable to pause the thread. */
    pthread_cond_t cond_;
    /** Id returned by pthread_create(). */
    pthread_t thread_;

    /** Is true if the trigger manager is running. */
    bool running_;
    /** Sets to true to abort. */
    bool abort_;
    /** Sets to true to pause the trigger manager. */
    bool pause_;

    /** Interval in us between two triggers. */
    long intervalInUs_;

    /** Function pointer to call just before sending a trigger. */
    pfv preTriggerAction_;
    /** Function pointer to call just after sending a trigger. */
    pfv postTriggerAction_;

public:

    /** Constructor. */
    FdTriggerManager();
    /** Destructor. */
    ~FdTriggerManager();

    /** Sets the interval in um between two refreshments of the interface. */
    void setIntervalInUs(long intervalInUs);
    /** Returns the interval in um between two refreshments of the interface. */
    long getIntervalInUs();

    /** Sets pre-trigger function. */
    void setPreTriggerAction(pfv functionPtr);
    /** Returns pre-trigger function. */
    pfv getPreTriggerAction();

    /** Sets post-trigger function. */
    void setPostTriggerAction(pfv functionPtr);
    /** Returns post-trigger function. */
    pfv getPostTriggerAction();

    /** Default function called before sending a trigger (do nothing by default). */
    void defaultPreTriggerAction(int triggerId);
    /** Defautl function called after sending a trigger (do nothing by default). */
    void defaultPostTriggerAction(int triggerId);

public slots:

    /** Starts generating triggers. */
    void start() throw(MyException*);
    /** Stop generating triggers. */
    void stop() throw(MyException*);

    /** Pauses or resumes the trigger manager. */
    void pause(bool pause) throw(MyException*);

    /** Returns true if the trigger manager is running. */
    bool isRunning();
    /** Returns true if the trigger manager has been aborted. */
    bool isAbort();

signals:

    /** Sent when a trigger is generated. */
    void triggered(unsigned int triggerId);
    /** Emits a signal when the trigger manager is aborted. */
    void done();

private:

    /** Initializes the playlist. */
    void initialize();

    /**
     * This is the static class function that serves as a C style function pointer
     * for the pthread_create call.
     */
    static void* processThread(void* obj);

    /** Function executed at every trigger. */
    void trigger(const unsigned int triggerId);
};

} // end namespace squid

#endif // FDTRIGGERMANAGER_H
