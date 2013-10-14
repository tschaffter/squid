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

#ifndef SQUIDSETTINGS_H
#define SQUIDSETTINGS_H

#include "dc1394/dc1394.h"
#include "squid.h"
#include <cstring>
#include <vector>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
//! Graphical interface of sQuid.
namespace qsquid {

/**
 * \brief sQuid settings.
 *
 * @version February 28, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class SquidSettings {

private:

    /** Reference of the main application. */
    Squid* squid_;

    /** Dc1394 mode (a = FireWire400, b = FireWire800). */
    std::string dc1394_;

    /** Settings file option. */
    po::options_description* config_file_options_;

    /** The absolute path to the settings file. */
    std::string settingsFile_;
    /** The absolute path to the output directory. */
    std::string workingDirectory_;
    /** Email addresses (separated by a space). */
    std::string emailAddresses_;

    /** Guid of the camera to be selected by default. */
    std::string cameraGuid_;
    /** Cameras configuration. */
    std::string cameraConfigurations_;
    /** Trigger period in ms. */
    unsigned int triggerPeriod_;

    /** The name of the experiment. */
    std::string experimentName_;
    /** Experiment duration mode (0 = MANUAL, 1 = FIXED, 2 = PLAYER). */
    int experimentDurationMode_;
    /** Experiment duration time in minutes (only for FIXED duration mode). */
    int experimentDuration_;
    /** If different from 0, experiment report is sent by email. */
    int experimentEmail_;
    /** The subject prefix of the email. */
    std::string experimentEmailSubjectPrefix_;
    /** The format in which images must be saved (0 = IMAGE_PGM, 1 = IMAGE_TIFF). */
    unsigned int outputFormat_;

    /** Settings file of the player. */
    std::string playerSettingsFilename_;

    /** Enables stderr logging. */
    int stderrLogging_;
    /** Stderr logging severity (0 = INFO, 1 = WARNING, 2 = ERROR, 3 = FATAL). */
    int stderrLoggingSeverity_;
    /** Enables file logging. */
    int fileLogging_;
    /** File logging severity (0 = INFO, 1 = WARNING, 2 = ERROR, 3 = FATAL). */
    int fileLoggingSeverity_;
    /** File logging prefix. */
    std::string fileLoggingPrefix_;
    /** File logging director.y */
    std::string fileLoggingDirectory_;
    /** Enables email logging. */
    int emailLogging_;
    /** Email logging severity. */
    int emailLoggingSeverity_;

    /** Returns the icon of the application window. */
    QIcon applicationWindowIcon_;

public:

    /** Destructor. */
    ~SquidSettings() {}

    /** Parses commmand-line arguments. */
    void parseArguments(int argc, char *argv[]) throw(MyException*);

    /** Configures logging using Google logging library (glog). */
    void configureLogging();
    /** Configures email logging. */
    void configureEmailLogging();

    /** Returns the unique instance of Settings. */
    static SquidSettings* getInstance();

    /** Sets sQuid reference. */
    void setSquid(Squid* squid);
    /** Returns sQuid reference. */
    Squid* getSquid();

    /**
     * VARIOUS
     */
    /** Sets the working directory. */
    void setWorkingDirectory(std::string directory);
    /** Returns the working directory. */
    std::string getWorkingDirectory();

    /** Sets the email addresses (separated by a space). */
    void setEmailAddress(std::string emailAddresses);
    /** Returns the email addresses. */
    std::string getEmailAddress();

    /**
     * CAMERAS
     */
    /** Sets the guid of the selected camera. */
    void setCameraGuid(std::string str);
    /** Returns the guid of the selected camera. */
    std::string getCameraGuid();

    /** Sets the cameras configuration. */
    void setCameraConfigurations(std::string config);
    /** Returns the cameras configuration. */
    std::string getCameraConfigurations();

    /** Sets the trigger period in milliseconds. */
    void setTriggerPeriod(unsigned int period);
    /** Returns the trigger period in milliseconds. */
    unsigned int getTriggerPeriod();

    /**
     * EXPERIMENT
     */
    /** Sets the name of the experiment. */
    void setExperimentName(std::string experimentName);
    /** Returns the name of the experiment. */
    std::string getExperimentName();

    /** Sets experiment duration mode. */
    void setExperimentDurationMode(int mode);
    /** Returns experiment duration mode. */
    int getExperimentDurationMode();

    /** Sets default experiment duration in minutes. */
    void setExperimentDuration(int duration);
    /** Returns default experiment duration in minutes. */
    int getExperimentDuration();

    /** Sets email addresses. */
    void setExperimentEmail(int experimentEmail);
    /** Returns email addresses. */
    int getExperimentEmail();

    /** Sets the subject prefix of the experiment report sent by email. */
    void setExperimentEmailSubjectPrefix(std::string prefix);
    /** Returns the subject prefix of the experiment report sent by email. */
    std::string getExperimentEmailSubjectPrefix();

    /** Sets the image output format. */
    void setOutputFormat(unsigned int format);
    /** Returns the image output format. */
    unsigned int getOutputFormat();

    /**
     * (PORT) PLAYER
     */
    /** Sets the absolute path to the player settings file. */
    void setPlayerSettingsFilename(std::string filename);
    /** Returns the absolute path to the player settings file. */
    std::string getPlayerSettingsFilename();

    /**
     * LOGGING
     */
    /** Sets stderr logging. */
    void setStderrLogging(int stderrLogging);
    /** Returns stderr logging. */
    int getStderrLogging();

    /** Sets stderr logging severity. */
    void setStderrLoggingSeverity(int severity);
    /** Returns stderr logging severity. */
    int getStderrLoggingSeverity();

    /** Sets file logging. */
    void setFileLogging(int fileLogging);
    /** Returns file logging. */
    int getFileLogging();

    /** Sets file logging severity. */
    void setFileLoggingSeverity(int severity);
    /** Returns file logging severity. */
    int getFileLoggingSeverity();

    /** Sets file logging prefix. */
    void setFileLoggingPrefix(std::string prefix);
    /** Returns file logging prefix. */
    std::string getFileLoggingPrefix();

    /** Sets file logging directory. */
    void setFileLoggingDirectory(std::string directory);
    /** Returns file logging directory. */
    std::string getFileLoggingDirectory();

    /** Sets email logging */
    void setEmailLogging(int emailLogging);
    /** Returns email logging. */
    int getEmailLogging();

    /** Sets email logging severity. */
    void setEmailLoggingSeverity(int severity);
    /** Returns email logging severity. */
    int getEmailLoggingSeverity();

    /** Sets path to the settings file. */
    void setSettingsFile(std::string settingsFile);
    /** Returns path to the settings file. */
    std::string getSettingsFile();

    /** Reads settings from file. */
    void load(std::string filename) throw(MyException*);
    /** Writes settings to file. */
    void save(std::string filename) throw(MyException*);

    /** Returns the icon of the application window. */
    QIcon getApplicationWindowIcon();

    /** Sets the absolute path to the player settings file. */
    void setParallelPortLogFilename(std::string filename);
    /** Returns the absolute path to the player settings file. */
    std::string getParallelPortLogFilename();

    /** Sets the stream to the player log file. */
    void setParallelPortLog(std::ofstream* stream);
    /** Returns the stream to the player log file. */
    std::ofstream* getParallelPortLog();

    /** Sets DC1394 mode. */
    void setDc1394(const std::string dc1394);
    /** Returns DC1394 mode. */
    std::string getDc1394() const;

private:

    /** Constructor. */
    SquidSettings();

    /** Initialization. */
    void initialize();

    /** The unique reference of Settings. */
    static SquidSettings* instance_;
};

}

#endif // SQUIDSETTINGS_H
