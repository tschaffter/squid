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

#include "booleanplaylist.h"
#include "myutility.h"
#include "highresolutiontime.h"
#include "fdtimer.h"
#include "rt.h"
#include <sstream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <glog/logging.h>

using namespace portplayer;

// ======================================================================
// PRIVATE METHODS

void* BooleanPlaylist::processThread(void* obj) {

    BooleanPlaylist* playlist = reinterpret_cast<BooleanPlaylist*>(obj);

    LOG(INFO) << "Promoting playlist to RT priority.";
    promoteRT();

    double playListBaseInMs = 0.;
    double stateBaseInMs = 0.;
    double pauseOffsetInMs = 0.;

    HighResolutionTime time;
    FdTimer timer;
    timer.initialize(0, playlist->getUpdateIntervalInUs() * 1000);

    time.start();   // time is used to get the elapsed time since starting the player
    timer.start();  // generates the timeouts to refresh the interface
    int fd = timer.getTimer();

    double tInMs = 0.;
    while (!playlist->isAbort()) {
        pthread_mutex_lock(&playlist->mutex_);
        if (!playlist->pause_) {
            pthread_mutex_unlock(&playlist->mutex_);

            uint64_t numTimeout;
            read(fd, &numTimeout, sizeof(numTimeout));
//            if (numTimeout > 1)
//                LOG(WARNING) << "Playlist timer missed " << (numTimeout - 1) << " events.";

            tInMs = time.getElapsedTimeInMs() - pauseOffsetInMs;

            // refresh the interface
            emit playlist->updatePlaylistTimeInMs(tInMs - playListBaseInMs);
            emit playlist->updateStateTimeInMs(tInMs - stateBaseInMs);

            pthread_mutex_lock(&playlist->mutex_);
            if (tInMs - stateBaseInMs > playlist->stateDurations_.at(playlist->currentState_)) {
                // move and apply next state
                if (playlist->currentState_ < playlist->getNumStates() - 1) {
                    stateBaseInMs = tInMs;
                    playlist->next(playlist->currentState_ + 1);
//                    playlist->currentState_++;
//                    emit playlist->stateChanged(playlist->currentState_);
                }
                // end of the playlist ? go to state 0 or stop
                else if (playlist->repeatPlaylist_) {
                    playListBaseInMs = tInMs;
                    stateBaseInMs = tInMs;
                    playlist->next(0);
//                    playlist->currentState_ = 0;
//                    emit playlist->stateChanged(playlist->currentState_);
                }
                else
                    playlist->abort_ = true;
            }
        }

        // pause the player ?
        while (playlist->pause_) {
            double tInMsBkp = time.getElapsedTimeInMs();
            if (pthread_cond_wait(&playlist->cond_, &playlist->mutex_)) // wait for resume signal
                throw new MyException("Unable to suspend playlist: pthread_cond_wait() failed.");
            pauseOffsetInMs += time.getElapsedTimeInMs() - tInMsBkp;
            LOG(INFO) << "Resuming playlist after a break of " << pauseOffsetInMs/1000. << " seconds.";
        }
        pthread_mutex_unlock(&playlist->mutex_);
    }

    // abortion
    emit playlist->updatePlaylistTimeInMs(tInMs - playListBaseInMs);
    emit playlist->updateStateTimeInMs(tInMs - stateBaseInMs);
    emit playlist->done();

    time.stop();
    timer.stop();

    pthread_mutex_lock(&playlist->mutex_);
    playlist->running_ = false;
    pthread_mutex_unlock(&playlist->mutex_);

    return NULL;
}

// ----------------------------------------------------------------------

void BooleanPlaylist::next(const unsigned int stateIndex) {

    currentState_ = stateIndex;

//    (this->*preNextStateAction_)(stateIndex);
    // there is no warranty that the slot(s) responding to this signal
    // will be executed before the post action.
    emit stateChanged(stateIndex);
//    (this->*postNextStateAction_)(stateIndex);
}

// ======================================================================
// PUBLIC METHODS

BooleanPlaylist::BooleanPlaylist(const unsigned int numItems) {

    if (pthread_mutex_init(&mutex_, NULL) == -1)
        throw new MyException("Unable to pthread_mutex_init().");
    if (pthread_cond_init(&cond_, NULL) == -1)
        throw new MyException("Unable to pthread_cond_init().");

    numItems_ = numItems;
    initialize();
}

// ----------------------------------------------------------------------

BooleanPlaylist::~BooleanPlaylist() {

    if (pthread_cond_destroy(&cond_) == -1)
        throw new MyException("Unable to pthread_cond_destroy().");
    if (pthread_mutex_destroy(&mutex_) == -1)
        throw new MyException("Unable to pthread_mutex_destroy().");
}

// ----------------------------------------------------------------------

void BooleanPlaylist::initialize() {

    // create a sequence made with one state
    std::vector<bool> state;
    for (unsigned int i = 0; i < numItems_; i++)
        state.push_back(false);
    playlist_.clear();
    playlist_.push_back(state);
    stateDurations_.clear();
    stateDurations_.push_back(1);

    currentState_ = 0;
    updateIntervalInUs_ = 10000;
    repeatPlaylist_ = false;
    running_ = false;
    pause_ = false;
    abort_ = false;
}

// ----------------------------------------------------------------------

void BooleanPlaylist::loadPlaylist(const std::string playlist/*, const unsigned int numItems*/) throw(MyException*) {

    std::stringstream ss(playlist);
    std::string buffer;
    std::vector<std::string> tokens;

    while (ss >> buffer)
        tokens.push_back(buffer);

    const unsigned int n = tokens.size();
    std::string temp = "";

    playlist_.clear();

    std::string a = "1";
    std::string b = "";

    for (unsigned int i = 0; i < n; i++) {
        temp = tokens.at(i);
        std::vector<bool> state;
        
        if (temp.length() != numItems_)
            throw new MyException("A state must have a multiple of " + intToIntString(numItems_) +  " binary elements.");

        for (unsigned int j = 0; j < numItems_; j++) {
            b = temp.at(j);
            if (a.compare(b) == 0)
                state.push_back(true);
            else
                state.push_back(false);
        }
        playlist_.push_back(state);
    }

    // init default duration for each state
    stateDurations_.clear();
    for (unsigned int i = 0; i < playlist_.size(); i++)
        stateDurations_.push_back(1);

    currentState_ = 0;
}

// ----------------------------------------------------------------------

std::string BooleanPlaylist::exportPlaylist() {

    const unsigned int numStates = getNumStates();

    if (numItems_ == 0 || numStates == 0)
        return "";

    std::ostringstream output;

    for (unsigned int i = 0; i < numStates; i++) {
        std::vector<bool> state = playlist_.at(i);

        for (unsigned int j = 0; j < numItems_; j++) {
            if (state.at(j))
                output << "1";
            else
                output << "0";
        }
        output << " ";
    }
    return output.str();
}

// ----------------------------------------------------------------------

/** WARNING: Be sure to first set the playlist before the duration. */
void BooleanPlaylist::loadStateDurations(const std::string durations) throw(MyException*) {

    std::stringstream ss(durations);
    std::string buffer;

    stateDurations_.clear();

    while (ss >> buffer)
        stateDurations_.push_back(intStringToInt(buffer));

    if (stateDurations_.size() == 0)
        LOG (WARNING) << "No state duration specified.";

    if (playlist_.size() != getNumStates())
        throw new MyException("Number of duration fields is not equal to the number of states in the playlist.");
}

// ----------------------------------------------------------------------

std::string BooleanPlaylist::exportStateDurations() {

    if (stateDurations_.size() == 0)
        return "";

    std::ostringstream output;

    for (unsigned int i = 0; i < stateDurations_.size(); i++)
        output << stateDurations_.at(i) << " ";

    return output.str();
}

// ----------------------------------------------------------------------

void BooleanPlaylist::start() throw(MyException*) {

    if (running_)
        throw new MyException("Playlist is already running.");

    pthread_mutex_lock(&mutex_);
    abort_ = false;
    pause_ = false;

    if (pthread_create(&thread_, 0, BooleanPlaylist::processThread, this))
        throw new MyException("Unable to start playlist thread: pthread_create() failed.");

    running_ = true;
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void BooleanPlaylist::stop() throw(MyException*) {

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

void BooleanPlaylist::pause(bool pause) throw(MyException*) {

    if (pause == pause_)
        return;

    if (pause) LOG(INFO) << "Suspending playlist.";
    else LOG(INFO) << "Resuming playlist.";

    pthread_mutex_lock(&mutex_);
    pause_ = pause;
    if (!pause) {
        if (pthread_cond_signal(&cond_))
            throw new MyException("Unable to resume playlist thread: pthread_cond_signal() failed.");
    }
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

unsigned int BooleanPlaylist::getPlaylistTotalTime() {

    unsigned int total = 0;
    const unsigned int numStates = getNumStates();

    for (unsigned int i = 0; i < numStates; i++)
        total += stateDurations_.at(i);

    return total;
}

// ======================================================================
// GETTERS AND SETTERS

unsigned int BooleanPlaylist::getNumItems() { return numItems_; }
unsigned int BooleanPlaylist::getNumStates() { return playlist_.size(); }
std::vector< std::vector<bool> >* BooleanPlaylist::getPlaylist() { return &playlist_; }
std::vector<unsigned int>* BooleanPlaylist::getStateDurations() { return &stateDurations_; }

void BooleanPlaylist::setCurrentState(const unsigned int state) { currentState_ = state; }
unsigned int BooleanPlaylist::getCurrentState() { return currentState_; }

void BooleanPlaylist::repeatPlaylist(int b) { repeatPlaylist_ = b; }
bool BooleanPlaylist::repeatPlaylist() { return repeatPlaylist_; }

bool BooleanPlaylist::isAbort() { return abort_; }
bool BooleanPlaylist::isRunning() { return running_; }

void BooleanPlaylist::setUpdateIntervalInUs(long updateIntervalInUs) { updateIntervalInUs_ = updateIntervalInUs; }
long BooleanPlaylist::getUpdateIntervalInUs() { return updateIntervalInUs_; }
