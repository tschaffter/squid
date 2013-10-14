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

#ifndef BOOLEANPLAYLIST_H
#define BOOLEANPLAYLIST_H

#include "myexception.h"
#include <pthread.h>
#include <vector>
#include <QObject>

//! Library to interact with the environment (valves, LEDs, robots, etc.).
namespace portplayer {

/**
 * \brief Represents a playlist of different states composed of boolean items, e.g. the parallel port pins.
 *
 * A duration in minutes, seconds or milliseconds can be associated to each state/line
 * in the player list. When starting the player, a timer is executed in a dedicated pthread. If
 * rtkit is correctly setup, the thread is set with a real-time priority. At each timeout
 * of the timer, the player moves and applied the next state. It it possible to configure
 * the player to repeat again and again the playlist without interruption. A SIGNAL
 * is sent at each timeout of the timer to indicate that the effective state in the player
 * has changed.
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class BooleanPlaylist : public QObject {

    Q_OBJECT

private:

    /** Mutex for the thread. */
    pthread_mutex_t mutex_;
    /** Condition variable to pause the thread. */
    pthread_cond_t cond_;
    /** Id returned by pthread_create(). */
    pthread_t thread_;

    /** Is true if the player is running. */
    bool running_;
    /** Sets to true to abort. */
    bool abort_;
    /** Sets to true to pause the player. */
    bool pause_;

    /** Interval in us between two timeouts to refresh the interface (default: 10ms). */
    long updateIntervalInUs_;

    /** The number of boolean items composing one state. */
    unsigned int numItems_;
    /** The index of the current and effective state in the playlist. */
    unsigned int currentState_;
    /** Table describing the playlist. */
    std::vector< std::vector<bool> > playlist_;
    /** Durations in ms of each state in the playlist. */
    std::vector<unsigned int> stateDurations_;
    /** If true, the playlist is played again and again. */
    bool repeatPlaylist_;

public:

    /** Constructor initialized with a given number of state/line. */
    BooleanPlaylist(const unsigned int numItems);
    /** Destructor. */
    ~BooleanPlaylist();

    /** Returns a string describing all the states in the playlist. */
    std::string exportPlaylist();
    /** Returns a string containing only the duration associated to each state. */
    std::string exportStateDurations();
    /** Returns the number of (boolean) items composing one state. */
    unsigned int getNumItems();
    /** Returns the number of states in the playlist. */
    unsigned int getNumStates();
    /** Returns the index of the current and effective state in the playlist. */
    unsigned int getCurrentState();
    /** Returns a table of boolean items describing all the sequences in the playlist. */
    std::vector< std::vector<bool> >* getPlaylist();
    /** Returns the duration associated to each state in the playlist. */
    std::vector<unsigned int>* getStateDurations();
    /** Returns the total duration of the playlist, i.e. the sum of all state durations. */
    unsigned int getPlaylistTotalTime();
    /** Returns true if the player is configured to play the playlist again and again. */
    bool repeatPlaylist();

public slots:

    /** Loads a playlist from a single string. */
    void loadPlaylist(const std::string playlist) throw(MyException*);
    /** Loads the state durations from a single string. */
    void loadStateDurations(const std::string durations) throw(MyException*);
    /** Sets the current and effective state in the playlist. */
    void setCurrentState(const unsigned int state);

    /** Starts playing the playlist. */
    void start() throw(MyException*);
    /** Stop the player. */
    void stop() throw(MyException*);

    /** Returns true if the player is running. */
    bool isRunning();
    /** Returns true if the player has been aborted. */
    bool isAbort();

    /** Sets the update interval in um between two refreshments of the interface. */
    void setUpdateIntervalInUs(long intervalInUs);
    /** Returns the update interval in um between two refreshments of the interface. */
    long getUpdateIntervalInUs();

    /** Pauses or resumes the playlist. */
    void pause(bool pause) throw(MyException*);

    /** Sets to true to play the playlist again and again. */
    void repeatPlaylist(int b);

signals:

    /** Sends the time in ms elapsed (or remaining) in the playlist. */
    void updatePlaylistTimeInMs(const unsigned int tInMs);
    /** Sends the time in ms elapsed (or remaining) in the current state. */
    void updateStateTimeInMs(const unsigned int tInMs);
    /** Sent when the current state has changed. */
    void stateChanged(const unsigned int currentState);
    /** Sent when the playlist is done, i.e. the player is stopped. */
    void done();

private:

    /** Initializes the playlist. */
    void initialize();

    /**
     * This is the static class function that serves as a C style function pointer
     * for the pthread_create call.
     */
    static void* processThread(void* obj);

    /** Function executed each time the playlist moves to the next state. */
    void next(const unsigned int nextStateIndex);
};

}

#endif // BOOLEANPLAYLIST_H
