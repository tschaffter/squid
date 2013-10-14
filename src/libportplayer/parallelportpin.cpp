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

#include "parallelportpin.h"
#include "myutility.h"
#include <exception>
#include <sys/io.h>
#include <glog/logging.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>

using namespace portplayer;

int ParallelPortPin::fd_ = 0;
unsigned int ParallelPortPin::ppPinCount_ = 0;

// ======================================================================
// PUBLIC METHODS

ParallelPortPin::ParallelPortPin(const std::string parport, const std::string name, const int port, const unsigned int pin, QObject *parent) : IOPin(name, port, pin, parent) {

    try {
        if (ppPinCount_ == 0)
            initializeParallelPort(parport);

        setLow();
        print();

        ppPinCount_++;

    } catch (MyException* e) {
        LOG(WARNING) << "Unable to initialize parallel port pin \"" << name_ << "\": " << e->getMessage();
    }
}

// ----------------------------------------------------------------------

ParallelPortPin::~ParallelPortPin() {

    try {
        setLow();

        if (ppPinCount_ == 1)
            finilizeParallelPort();

        ppPinCount_--;

    } catch (MyException* e) {
        LOG(WARNING) << "Unable to close parallel port pin \"" << name_ << "\": " << e->getMessage();
    }
}

// ----------------------------------------------------------------------

void ParallelPortPin::initializeParallelPort(const std::string parport) throw(MyException*) {

    // open the parallel port for reading and writing
    fd_ = open(parport.c_str(), O_RDWR);
    if (fd_ == -1) {
        std::string str = "Could not open parallel port " + parport + ".";
        throw new MyException(str);
    }

    // try to claim port (see http://www.linuxfocus.org/common/src/article205/ppdev.html)
    if (ioctl(fd_, PPCLAIM, NULL)) {
        close(fd_);
        throw new MyException("Could not claim parallel port.");
    }

    // and set the mode (SPP bi-directional, sometimes calls PS/2)
    int mode = IEEE1284_MODE_BYTE;
    if (ioctl(fd_, PPSETMODE, &mode)) {
        ioctl(fd_, PPRELEASE);
        close(fd_);
        throw new MyException("Unable to set paralllel port in SSP bi-directional mode.");
    }
}

// ----------------------------------------------------------------------

void ParallelPortPin::finilizeParallelPort() throw(MyException*) {

    // release and close the parallel port
    ioctl(fd_, PPRELEASE);
    close(fd_);
}

// ----------------------------------------------------------------------

void ParallelPortPin::setLow() {

    try {
        unsigned char value;
        readParallelPort(value);
        value &= ~(1 << pin_);
        writeParallelPort(value);
		
        emit stateChanged(false);

    } catch (MyException* e) {
        LOG(WARNING) << e->getMessage();
    }
}

// ----------------------------------------------------------------------

void ParallelPortPin::setHigh() {

    try {
        unsigned char value;
        readParallelPort(value);
        value |= (1 << pin_);
        writeParallelPort(value);

        emit stateChanged(true);

    } catch (MyException* e) {
        LOG(WARNING) << e->getMessage();
    }
}

// ----------------------------------------------------------------------

bool ParallelPortPin::isLow() throw(MyException*) {

    unsigned char value;
    readParallelPort(value);
    value >>= pin_;

    return (!(value & 1));
}

// ----------------------------------------------------------------------

bool ParallelPortPin::isHigh() throw(MyException*) {

    unsigned char value;
    readParallelPort(value);
    value >>= pin_;

    return (value & 1);
}

// ----------------------------------------------------------------------

void ParallelPortPin::print() {

    LOG(INFO) << "Parallel port pin: name = " << name_ << ", port = " << intToHexString(port_) << ", pin index = " << pin_ << ", state = " << getState();
}

// ---------------------------------------------------------------------- //

void ParallelPortPin::readParallelPort(unsigned char& value) throw(MyException*) {

//    setParallelPortDirectionToInput();

    if (ioctl(fd_, PPRDATA, &value) == 1)
        throw new MyException("Could not read parallel port.");
}

// ---------------------------------------------------------------------- //

void ParallelPortPin::writeParallelPort(unsigned char& value) throw(MyException*) {

//    setParallelPortDirectionToOutput();

    if (ioctl(fd_, PPWDATA, &value))
        throw new MyException("Could not write parallle port.");
}

// ---------------------------------------------------------------------- //

/** @deprecated */
void ParallelPortPin::setParallelPortDirectionToOutput() throw(MyException*) {

    // and direction (0=out) (1=in)
    int dir = 0x00;
    if (ioctl(fd_, PPDATADIR, &dir)) {
        ioctl(fd_, PPRELEASE);
        close(fd_);
        throw new MyException("Could not set parallel port direction to output.");
    }
}

// ---------------------------------------------------------------------- //

/** @deprecated */
void ParallelPortPin::setParallelPortDirectionToInput() throw(MyException*) {

    // and direction (0=out) (1=in)
    int dir = 0x01; // data_reverse
    if (ioctl(fd_, PPDATADIR, &dir)) {
        ioctl(fd_, PPRELEASE);
        close(fd_);
        throw new MyException("Could not set parallel port direction to intput.");
    }
}
