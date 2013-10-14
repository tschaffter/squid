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

#include "iopin.h"
#include "myutility.h"
#include <sys/io.h>
#include <glog/logging.h>

using namespace portplayer;

// ======================================================================
// PUBLIC METHODS

IOPin::IOPin(const std::string name, const int port, const unsigned int pin, QObject *parent) : QObject(parent) {

    name_ = name;
    port_ = port;
    pin_ = pin;
}

// ----------------------------------------------------------------------

void IOPin::setState(const bool state) throw(MyException*) {

    if (state)
        setHigh();
    else
        setLow();
}

// ----------------------------------------------------------------------

bool IOPin::getState() throw(MyException*) {

    return isHigh();
}

// ----------------------------------------------------------------------

void IOPin::print() {

    LOG(INFO) << "IOPin: name = " << name_ << ", port = " << intToHexString(port_) << ", pin = " << pin_;
}

// ======================================================================
// GETTERS AND SETTERS

void IOPin::setName(const std::string name) { name_ = name; }
std::string IOPin::getName() { return name_; }

void IOPin::setPort(const int port) { port_ = port; }
int IOPin::getPort() { return port_; }

void IOPin::setPin(const unsigned int pin) { pin_ = pin; }
unsigned int IOPin::getPin() { return pin_; }
