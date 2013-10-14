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

#ifndef PARALLELPORTMANAGER_H
#define PARALLELPORTMANAGER_H

#include "iopinmanager.h"
#include "parallelportpin.h"
#include "myexception.h"
#include <vector>
#include <iostream>
#include <fstream>

//! Library to interact with the environment (valves, LEDs, robots, etc.).
namespace portplayer {

/**
 * \brief Parallel port (IEEE1284) manager.
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class ParallelPortManager : public IOPinManager {

    Q_OBJECT

protected:

    /** Closes and deletes the pins. */
    virtual void deletePins();

public:

    /** Constructor. */
    ParallelPortManager(QObject *parent = 0);
    /** Destructor. */
    ~ParallelPortManager();

    /** Returns a string describing the settings of the pins. */
    virtual void load(const std::string parport, const std::string pins) throw(MyException*);

    /** Generates an example using all the pins of a standard parallel port (IEEE1284).*/
    static ParallelPortManager* generateParallelPortExample();

    /** Returns a string descrbing the current state of the parallel port. */
    std::string getParallelPortState();

public slots:

    /** Returns a string describing the settings of the pins. */
    virtual std::string save();
    /** Saves the current states of the parallel port to file. */
    void saveParallelPortState(std::ofstream* os, std::string prefix);
};

}

#endif // PARALLELPORTMANAGER_H
