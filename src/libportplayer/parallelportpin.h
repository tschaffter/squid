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

#ifndef PARALLELPORTPIN_H
#define PARALLELPORTPIN_H

#include "iopin.h"

//! Library to interact with the environment (valves, LEDs, robots, etc.).
namespace portplayer {

/**
 * \brief Represents the pin of a parallel port (IEEE1284).
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class ParallelPortPin : public IOPin {

    Q_OBJECT

protected:

    /** Reference to parport{fd_} */
    static int fd_;
    /** Number of pins accessible through the parallel port. */
    static unsigned int ppPinCount_;

public:

    /** Constructor takes the name of the pin, the address of the port and the index of the pin. */
    ParallelPortPin(const std::string parport, const std::string name, const int port, const unsigned int pin, QObject *parent = 0);
    /** Destructor. */
    virtual ~ParallelPortPin();

    /** Returns true if the state of the pin is low. */
    virtual bool isLow() throw(MyException*);
    /** Returns true if the state of the pin is high. */
    virtual bool isHigh() throw(MyException*);

public slots:

    /** Sets pin to low state. */
    virtual void setLow();
    /** Sets pin to high state. */
    virtual void setHigh();
    /** Prints the description of the pin (name, port, index and state). */
    virtual void print();

    /** Initializes the parallel port. */
    void initializeParallelPort(const std::string parport) throw(MyException*);
    /** Close the parallel port. */
    void finilizeParallelPort() throw(MyException*);
    /** Read the parallel port. */
    void readParallelPort(unsigned char& value) throw(MyException*);
    /** Write to the parallel port. */
    void writeParallelPort(unsigned char& value) throw(MyException*);

    /**
     * Set the direction of the parallel port to output.
     * @deprecated
     */
    void setParallelPortDirectionToOutput() throw(MyException*);
    /**
     * Set the direction of the parallel port to input.
     * @deprecated
     */
    void setParallelPortDirectionToInput() throw(MyException*);
};

}

#endif // PARALLELPORTPIN_H
