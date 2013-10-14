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

#ifndef FPSEVALUATOR_H
#define FPSEVALUATOR_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

/**
 * \brief Computes FPS (frames per second) or camera framerate in a dedicated thread.
 *
 * Here QThread doesn't need to be replaced by pthread.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class FpsEvaluator : public QThread {

    Q_OBJECT

private:

    /** Number of frames at the end of the previous FPS evaluation. */
    unsigned int prevNumFrames_;
    /** Current number of frames. */
    unsigned int currentNumFrames_;
    /** When the last FPS evaluation has been done. */
    unsigned int baseTimeInMilliseconds_;
    /** Time interval in ms between two FPS evaluations. */
    unsigned int intervalInMs_;
    /** Time support. */
    QTime time_;

    /** Mutex. */
    QMutex mutex_;
    /** Condition for waiting. */
    QWaitCondition condition_;

    /** If false, the thread is put to sleep. */
    bool restart_;
    /** If true, the thread is stopped. */
    bool abort_;

public:

    /** Constructor. */
    FpsEvaluator();
    /** Destructor. */
    ~FpsEvaluator();

    /** Initialization. */
    void initialize();

    /** Set the interval time between two refresh (use mutliples of 1s). */
    void setIntervalInMs(unsigned int interval);

public slots:

    /** Start thread. */
    void run();
    /** Stop thread. */
    void stop();
    /** Call setRestart(false) to put the thread to sleep. */
    void setRestart(bool b);
    /** Call setAbort(true) to stop the thread. */
    void setAbort(bool b);
    /** If the thread has been put to sleep with setRestart(false), call this method to wake it. */
    void wake();

    /** Do currentNumFrames_++. */
    void incrementNumFrames();

signals:

    /** Sent each time FPS is evaluated. */
    void fpsUpdated(const float fps);
};

} // end namespace squid

#endif // FPSEVALUATOR_H
