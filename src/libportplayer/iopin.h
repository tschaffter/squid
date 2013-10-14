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

#ifndef IOPIN_H
#define IOPIN_H

#include "myexception.h"
#include <QObject>

//! Library to interact with the environment (valves, LEDs, robots, etc.).
namespace portplayer {

/**
 * \brief Abstract representation of a port pin.
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class IOPin : public QObject {

    Q_OBJECT

protected:

    /** The name of the pin. */
    std::string name_;
    /** The base address of the port, e.g. a valid 8 bit I/O port for parallel port. */
    int port_;
    /** The index of the pin. */
    unsigned int pin_;

public:

    /** Constructor takes the name of the pin, the address of the port and the index of the pin. */
    IOPin(const std::string name, const int port, const unsigned int pin, QObject *parent = 0);
    /** Destructor. */
    virtual ~IOPin() {}

    /** Returns true if the state of the pin is low. */
    virtual bool isLow() throw(MyException*) = 0;
    /** Returns true if the state of the pin is high. */
    virtual bool isHigh() throw(MyException*) = 0;

    /** Sets the name of the pin. */
    std::string getName();
    /** Returns the address of the port. */
    int getPort();
    /** Returns the name of the pin. */
    unsigned int getPin();
    /** Returns the state of the pin. */
    bool getState() throw(MyException*);

public slots:

    /** Sets pin to low state. */
    virtual void setLow() = 0;
    /** Sets pin to high state. */
    virtual void setHigh() = 0;
    /** Prints the description of the pin (name, port, index and state). */
    virtual void print();

    /** Sets the name of the pin. */
    void setName(const std::string name);
    /** Sets the port address of the pin. */
    void setPort(const int port);
    /** Sets the index of the pin. */
    void setPin(const unsigned int pin);
    /** Sets the state of the pin. */
    void setState(const bool state) throw(MyException*);

signals:

    /** Sent when the state of the pin has changed. */
    void stateChanged(const bool state);
};

}

#endif // IOPIN_H
