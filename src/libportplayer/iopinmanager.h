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

#ifndef IOPINMANAGER_H
#define IOPINMANAGER_H

#include "iopin.h"
#include "booleanplaylist.h"
#include "myexception.h"
#include <vector>
#include <QObject>

//! Library to interact with the environment (valves, LEDs, robots, etc.).
namespace portplayer {

/**
 * \brief Abstract manager of a set of pins.
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class IOPinManager : public QObject {

    Q_OBJECT

public:

    /** Manager mode. */
    enum Mode {
        EDITION = 0,    // user can modify the sequences
        MANUAL,         // user can apply the selected sequence
        PLAYLIST,       // waiting for the user to start the sequence player
        REMOTE          // move and apply the next sequence from remote code
    };

protected:

    /** Absolute path to the port device (e.g. "/dev/parport0"). */
    std::string portDevice_;
    /** Set of pins. */
    std::vector<IOPin*> pins_;
    /** Represents a list of combinations of pin states. */
    BooleanPlaylist* pinPlaylist_;
    /** Current mode of the manager. */
    Mode mode_;

    /** Delete the pins. */
    virtual void deletePins();

public:

    /** Constructor. */
    IOPinManager(QObject *parent);
    /** Destructor. */
    ~IOPinManager();

    /** Returns a string describing the settings of the pins. */
    virtual std::string save() = 0;

    /** Returns a set of pins. */
    std::vector<IOPin*>& getPins();
    /** Returns the number of pins. */
    unsigned int getNumPins();
    /** Returns a list containing the names of the pins. */
    std::vector<std::string> getPinNames();
    /** Returns the pins sequence. */
    BooleanPlaylist* getPinPlaylist();
    /** Returns the current mode of the manager. */
    Mode getMode();

 public slots:

    /** Configures the manager from the given string. */
    virtual void load(const std::string portDevice, const std::string str) throw(MyException*) = 0;
    /** Turns off all pins. */
    virtual void setAllPinsLow();

    /** Applies the specified state in the sequence. */
    void applyState(const unsigned int state) throw(MyException*);

    /** Sets the mode of the manager. */
    void setMode(const Mode mode);
    /** Prints a description of the sequence. */
    void printPlaylist();
};

}

#endif // IOPINMANAGER_H
