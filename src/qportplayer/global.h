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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "myexception.h"
#include <string>
#include <vector>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
//! Graphical interface to control the environment (valves, LEDs, robots, etc.).
namespace qportplayer {

/**
 * \brief Global settings for this project.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Global {

private:

    /** The unique instance of Global. */
    static Global* instance_;

    /** Settings file options. */
    po::options_description* config_file_options_;

    /** Absolute path of the settings file. */
    std::string settingsFile_;

    /** Absolute path to the port device (e.g. "/dev/parport0"). */
    std::string portDeviceAbsPath_;
    /** Single string describing all the pins. */
    std::string pins_;
    /** Single string describing the content of the playlist. */
    std::string states_;
    /** Duration unit (0=min, 1=sec, 2=msec). */
    int durationUnit_;

public:

    /** Destructor. */
    ~Global();
    /** Returns the Singleton instance. */
    static Global* getInstance();

    /** Parses the options from list of arguments. */
    void parseArguments(int argc, char *argv[] = NULL) throw(MyException*);
    /** Configures the logging system. */
    void configureLogging();

    /** Sets the filename of the settings file (absolute path). */
    void setSettingsFile(std::string GlobalFile);
    /** Returns the filename of the settings file (absolute path). */
    std::string getSettingsFile();

    /** Sets the absolute path to the port device (e.g. "/dev/parport0"). */
    void setPortDeviceAbsPath(std::string portDeviceAbsPath);
    /** Returns the absolute path to the port device (e.g. "/dev/parport0"). */
    std::string getPortDeviceAbsPath();

    /** Sets the configuration of the pins described by a single string. */
    void setPins(std::string pins);
    /** Returns the configuration of the pins described by a single string. */
    std::string getPins();

    /** Sets playlist states from a string. */
    void setStates(std::string states);
    /** Returns a string describing the states of the playlist. */
    std::string getStates();

    /** Sets duration unit (0=min, 1=sec, 2=msec). */
    void setDurationUnit(int unit);
    /** Returns duration unit (0=min, 1=sec, 2=msec). */
    int getDurationUnit();

    /** Reads settings file. */
    void load(std::string filename) throw(MyException*);
    /** Writes settings file. */
    void save(std::string filename) throw(MyException*);

private:

    /** Constructor. */
    Global();
    /** Initialization. */
    void initialize();
};

}

#endif // GLOBAL_H
