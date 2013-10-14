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

#include "parallelportmanager.h"
#include "myutility.h"
#include <sstream>
#include <glog/logging.h>

using namespace portplayer;

// ======================================================================
// PUBLIC METHODS

ParallelPortManager::ParallelPortManager(QObject *parent) : IOPinManager(parent) {

    pinPlaylist_ = NULL;
}

// ----------------------------------------------------------------------

ParallelPortManager::~ParallelPortManager() {

    deletePins();
}

// ----------------------------------------------------------------------

void ParallelPortManager::deletePins() {

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

void ParallelPortManager::load(const std::string parport, const std::string pinList) throw(MyException*) {

    portDevice_ = parport;

    std::string buffer;
    std::stringstream ss(pinList);
    std::vector<std::string> tokens;

    while (ss >> buffer)
        tokens.push_back(buffer);

    int n = tokens.size();

    if (n%3 != 0)
        throw new MyException("Invalid LED list format.");

    n /= 3;

    std::string id = "";
    int port = 0;
    unsigned char pin = 0;
    ParallelPortPin* p = NULL;
    deletePins();

    for (int i = 0; i < n; i++) {
        try {
            id = tokens.at(3 * i);
            port = hexStringToInt(tokens.at((3 * i) + 1));
            pin = intStringToInt(tokens.at((3 * i) + 2));

            p = new ParallelPortPin(portDevice_, id, port, pin);
            pins_.push_back(p);
        } catch (MyException* e) {
            LOG(WARNING) << "ParallelPortPinManager::load(): " << e->getMessage();
        }
    }
    
    delete pinPlaylist_;
    pinPlaylist_ = new BooleanPlaylist(n);
}

// ----------------------------------------------------------------------

std::string ParallelPortManager::save() {

    if (getNumPins() == 0)
        return "";

    std::ostringstream output;
    ParallelPortPin* p = NULL;
    const unsigned int numPins = getNumPins();

    for (unsigned int i = 0; i < numPins; i++) {
        p = dynamic_cast<ParallelPortPin*>(pins_.at(i));
        output << p->getName() << " ";
        output << intToHexString(p->getPort()) << " ";
        output << intToIntString(p->getPin()) << " ";
    }

    return output.str();
}

// ----------------------------------------------------------------------

ParallelPortManager* ParallelPortManager::generateParallelPortExample() {

    LOG(INFO) << "Generating parallel port example.";
    portplayer::ParallelPortManager* ppManager = new portplayer::ParallelPortManager();
    ppManager->load("/dev/parport0", "pin0 0x327 0 pin1 0x327 1 pin2 0x327 2 pin3 0x327 3 pin4 0x327 4 pin5 0x327 5 pin6 0x327 6 pin7 0x327 7");
    ppManager->getPinPlaylist()->loadPlaylist("10000000 01000000");
    ppManager->getPinPlaylist()->loadStateDurations("5 5");

    return ppManager;
}

// ----------------------------------------------------------------------

std::string ParallelPortManager::getParallelPortState() {

    std::ostringstream buffer;
    ParallelPortPin* p = dynamic_cast<ParallelPortPin*>(pins_.at(0)); // use the first pin as reference

    unsigned char value;
    p->readParallelPort(value);

    return unsignedCharToString(value);
}

// ----------------------------------------------------------------------

void ParallelPortManager::saveParallelPortState(std::ofstream* os, std::string prefix) {

    *os << prefix << getParallelPortState() << "\n";
}
