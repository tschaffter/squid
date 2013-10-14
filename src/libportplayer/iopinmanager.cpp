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

#include "iopinmanager.h"
#include "myutility.h"
#include <sstream>
#include <glog/logging.h>

using namespace portplayer;

// ======================================================================
// PUBLIC METHODS

IOPinManager::IOPinManager(QObject *parent) : QObject(parent) {

    portDevice_ = "/dev/parport0";
    mode_ = EDITION;
}

// ----------------------------------------------------------------------

IOPinManager::~IOPinManager() {

    deletePins();
    delete pinPlaylist_;
    pinPlaylist_ = NULL;
}

// ----------------------------------------------------------------------

void IOPinManager::deletePins() {

    const unsigned int numPins = pins_.size();
    IOPin* pin = NULL;
    for (unsigned int i = 0; i < numPins; i++) {
        pin = dynamic_cast<IOPin*>(pins_.at(i));
        delete pin;
        pin = NULL;
    }
    pins_.clear();
}

// ----------------------------------------------------------------------

std::vector<std::string> IOPinManager::getPinNames() {

    const unsigned int numPins = getNumPins();
    std::vector<std::string> list;
    for (unsigned int i = 0; i < numPins; i++)
        list.push_back(dynamic_cast<IOPin*>(pins_.at(i))->getName());

    return list;
}

// ----------------------------------------------------------------------

void IOPinManager::printPlaylist() {

    const unsigned int n = pinPlaylist_->getNumStates();
    for (unsigned int i = 0; i < n; i++) {
        std::vector<bool> state = pinPlaylist_->getPlaylist()->at(i);
        for (unsigned int j = 0; j < state.size(); j++)
            LOG (INFO) << "State " << i << ", pin " << j << ": " << state.at(j);
    }
}

// ----------------------------------------------------------------------

void IOPinManager::setAllPinsLow() {

    const unsigned int numPins = getNumPins();
    IOPin* pin = NULL;
    for (unsigned int i = 0; i < numPins; i++) {
        pin = getPins().at(i);
        pin->setLow();
    }
}

// ----------------------------------------------------------------------

void IOPinManager::applyState(const unsigned int state) throw(MyException*) {

    std::vector<bool> b = pinPlaylist_->getPlaylist()->at(state);
    IOPin* pin = NULL;
    for (unsigned int i = 0; i < getNumPins(); i++) {
        pin = dynamic_cast<IOPin*>(pins_.at(i));
        pin->setState(b.at(i));
    }
}

// ======================================================================
// GETTERS AND SETTERS

unsigned int IOPinManager::getNumPins() { return pins_.size(); }
std::vector<IOPin*>& IOPinManager::getPins() { return pins_; }
BooleanPlaylist* IOPinManager::getPinPlaylist() { return pinPlaylist_; }

void IOPinManager::setMode(const Mode mode) { mode_ = mode; }
IOPinManager::Mode IOPinManager::getMode() { return mode_; }
