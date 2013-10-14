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

#include "squidsettings.h"
#include "experiment.h"
#include "myutility.h"
#include "dc1394utility.h"
#include "dc1394framewriter.h"
#include "boost/filesystem.hpp"
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <glog/logging.h>

using namespace squid;
using namespace qsquid;

/** Singleton instance. */
SquidSettings* SquidSettings::instance_ = NULL;

// ======================================================================
// PRIVATE METHODS

SquidSettings::SquidSettings() {

    initialize();
}

// ----------------------------------------------------------------------

void SquidSettings::initialize() {

    dc1394_ = "a";
    settingsFile_ = "";
    workingDirectory_ = getenv("HOME");
//    workingDirectory_ += "/data";
    emailAddresses_ = "";
    cameraGuid_ = "";
    cameraConfigurations_ = "";
    triggerPeriod_ = 50;
    playerSettingsFilename_ = "";
    experimentName_ = "MyExperiment";
    experimentDurationMode_ = 1;
    experimentDuration_ = 1;
    experimentEmail_ = 1;
    experimentEmailSubjectPrefix_ = "sQuid message";
    outputFormat_ = Dc1394FrameWriter::IMAGE_TIFF;
    stderrLogging_ = 1;
    stderrLoggingSeverity_ = 0;
    fileLogging_ = 0;
    fileLoggingSeverity_ = 0;
    fileLoggingPrefix_ = "squid_log_";
    fileLoggingDirectory_ = getenv("HOME");
    emailLogging_ = 0;
    emailLoggingSeverity_ = 2; // INFO, WARNING, ERROR, and FATAL are 0, 1, 2, and 3, respectively.
    applicationWindowIcon_.addFile(":/ico");
    config_file_options_ = NULL;
}

// ======================================================================
// PUBLIC METHODS

SquidSettings* SquidSettings::getInstance() {

    if (instance_ == NULL)
        instance_ = new SquidSettings();

    return instance_;
}

// ----------------------------------------------------------------------

void SquidSettings::configureLogging() {

    // Initialize Google's logging library.
    google::InitGoogleLogging("");

    if (stderrLogging_)
        google::SetStderrLogging(stderrLoggingSeverity_);
//    else
//        google::SetStderrLogging(99999);

    if (fileLogging_) {
        std::stringstream loggingPath("");
        loggingPath << getFileLoggingDirectory() << "/";
        google::SetLogDestination(fileLoggingSeverity_, loggingPath.str().c_str());
        google::SetLogFilenameExtension(getFileLoggingPrefix().c_str());
    }
//    else
//        google::SetLogDestination(99999, "");

    configureEmailLogging();
}

// ----------------------------------------------------------------------

void SquidSettings::configureEmailLogging() {

    if (emailLogging_)
        google::SetEmailLogging(emailLoggingSeverity_, emailAddresses_.c_str());
//    else
//        google::SetEmailLogging(99999, "");
}

// ----------------------------------------------------------------------

void SquidSettings::parseArguments(int argc, char* argv[]) throw(MyException*) {

    try {
        // A group of options that will be allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "Print sQuid version")
            ("help,h", "Display this help")
        ;

        // A group of options that will be allowed both on command line
        // and in config file
        po::options_description config("Configuration");
        config.add_options()
            // ====================================================================================
            // COMMAND LINE OPTIONS
            ("settings-file,s", po::value<std::string>(&settingsFile_), "Specify settings file")
            ("dc1394", po::value<std::string>(&dc1394_), "DC1394 mode (a=FireWire400, b=Firewire800, default: a)")
        ;

        // A group of options that containt options that we will not display in help
        // Mainly option for config file
        po::options_description hidden("Hidden options");
        hidden.add_options()
            // ====================================================================================
            // GENERAL
            ("workingDirectory", po::value<std::string>(&workingDirectory_), "Working directory (default: USER_HOME)")
            ("emailAddresses", po::value<std::string>(&emailAddresses_), "List of comma-separated email addresses")
            // ====================================================================================
            // CAMERAS
            ("cameraGuid", po::value<std::string>(&cameraGuid_), "Guid of the camera to select (if detected)")
            ("cameraConfigurations", po::value<std::string>(&cameraConfigurations_), "Cameras configuration")
            ("triggerPeriod", po::value<unsigned int>(&triggerPeriod_), "Trigger period in milliseconds")
            // ====================================================================================
            // PARALLEL PORT CONTROLLER
            ("playerSettingsFilename", po::value<std::string>(&playerSettingsFilename_), "Absolute path to the player settings file")
            // ====================================================================================
            // EXPERIMENTS
            ("experimentName", po::value<std::string>(&experimentName_), "Experiment name")
            ("experimentDurationMode", po::value<int>(&experimentDurationMode_), "Experiment duration mode (0=MANUAL, 1=FIXED, 2=PLAYER)")
            ("experimentDuration", po::value<int>(&experimentDuration_), "Experiment duration in minutes")
            ("experimentEmail", po::value<int>(&experimentEmail_), "Send experiment report by email (1=yes, 0=no)")
            ("experimentEmailSubjectPrefix", po::value<std::string>(&experimentEmailSubjectPrefix_), "Email subject prefix")
            ("outputFormat", po::value<unsigned int>(&outputFormat_), "Image output format (0=IMAGE_PGM, 1=IMAGE_TIFF)")
            // ====================================================================================
            // LOGGING
            ("stderrLogging", po::value<int>(&stderrLogging_), "Enable stderr logging (1=on, 0=off)")
            ("stderrLoggingSeverity", po::value<int>(&stderrLoggingSeverity_), "Stderr logging severity (see Google logging documentation)")
            ("fileLogging", po::value<int>(&fileLogging_), "Enable file logging (1=on, 0=off)")
            ("fileLoggingSeverity", po::value<int>(&fileLoggingSeverity_), "File logging severity (see Google logging documentation)")
            ("fileLoggingDirectory", po::value<std::string>(&fileLoggingDirectory_), "Directory where log files are saved (default: USER_HOME)")
            ("fileLoggingPrefix", po::value<std::string>(&fileLoggingPrefix_), "String prefix for logging filenames")
            ("emailLogging", po::value<int>(&emailLogging_), "Enable email logging (1=on, 0=off)")
            ("emailLoggingSeverity", po::value<int>(&emailLoggingSeverity_), "Email logging severity (see Google logging documentation)")
        ;

        // Groups options
        // Both commandes line & config file
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        // Only config file
        if (config_file_options_ == NULL) {
            config_file_options_ = new po::options_description();
            config_file_options_->add(config).add(hidden);
        }

        // Visible options (not necessary idem that commande line options)
        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        // Map
        po::variables_map vm;
        store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);

        if (vm.count("help")) {
            std::cout << "sQuid " << SQUID_VERSION << " (" << SQUID_VERSION_DATE << ")" << std::endl;
            std::cout << std::endl;
            std::cout << "sQuid enables the supervision and manipulation of an environment using FireWire cameras (IEEE 1394) and communication with port interfaces." << std::endl;
            std::cout << "As an example, sQuid Player allows to control a hardware setup connected to a parallel port (IEEE 1284).";
            std::cout << std::endl;
            std::cout << visible;
            std::cout << std::endl;
            std::cout << "Project website: http://tschaffter.ch/projects/squid/" << std::endl;
            std::cout << "Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)" << std::endl;
            exit(0);
        }

        if (vm.count("version")) {
            std::cout << "sQuid " << SQUID_VERSION << " (" << SQUID_VERSION_DATE << ")" << std::endl;
            exit(0);
        }

        if (vm.count("settings-file"))
            load(vm["settings-file"].as<std::string>());
        else {
//            std::cout << "No settings file specified!" << std::endl << std::endl;
//            std::cout << visible << std::endl;
        }
    } catch (std::exception& e) {
        throw new MyException(e.what());
    }
}

// ----------------------------------------------------------------------

void SquidSettings::load(std::string filename) throw(MyException*) {

    try {
        // if specified settings file exists
        if (std::ifstream(filename.c_str())) {
            // If here, settings file exists -> open it
            std::ifstream ifs(filename.c_str());

            po::variables_map vm;
            store(parse_config_file(ifs, *config_file_options_), vm);
            notify(vm);

            // remove quotes from strings (if any)
            // this wasn't required with a previous version of boost
            stripLeadingAndEndingQuotes(workingDirectory_);
            stripLeadingAndEndingQuotes(emailAddresses_);
            stripLeadingAndEndingQuotes(dc1394_);
            stripLeadingAndEndingQuotes(cameraGuid_);
            stripLeadingAndEndingQuotes(cameraConfigurations_);
            stripLeadingAndEndingQuotes(playerSettingsFilename_);
            stripLeadingAndEndingQuotes(experimentName_);
            stripLeadingAndEndingQuotes(experimentEmailSubjectPrefix_);
            stripLeadingAndEndingQuotes(fileLoggingDirectory_);
            stripLeadingAndEndingQuotes(fileLoggingPrefix_);

            if (!boost::filesystem::exists(workingDirectory_) || !boost::filesystem::is_directory(workingDirectory_)) {
                LOG(WARNING) << "Invalid working directory " << workingDirectory_;
                workingDirectory_ = getenv("HOME");
                LOG(WARNING) << "Set working directory to " << workingDirectory_;
            }

            if (!boost::filesystem::exists(fileLoggingDirectory_) || !boost::filesystem::is_directory(fileLoggingDirectory_)) {
                LOG(WARNING) << "Invalid logging directory " << fileLoggingDirectory_;
                fileLoggingDirectory_ = getenv("HOME");
                LOG(WARNING) << "Set logging directory to " << fileLoggingDirectory_;
            }

            if (!boost::filesystem::exists(playerSettingsFilename_) && playerSettingsFilename_.length() > 0) {
                LOG (WARNING) << "Player settings file " << playerSettingsFilename_ << " doesn't exist.";
                playerSettingsFilename_ = "";
            }
            settingsFile_ = filename;
        }
        else
            throw new MyException("Unable to open settings file " + settingsFile_);

    } catch (boost::program_options::unknown_option& e) {
        throw new MyException(e.what());
    }
}

// ----------------------------------------------------------------------

void SquidSettings::save(std::string filename) throw(MyException*) {

    try {
        std::ofstream myfile;
        myfile.open(filename.c_str(), std::ios::out | std::ios::trunc);
        if (myfile.is_open()) {
            myfile << "#########################################################################" << std::endl;
            myfile << "# sQuid settings file" << std::endl;
            myfile << "# sQuid homepage: http://tschaffter.ch/projects/squid/" << std::endl;
            myfile << "#########################################################################" << std::endl;
            myfile << std::endl;
            myfile << "# Boolean values: 0 => false, 1 => true" << std::endl;
            myfile << std::endl;
            myfile << std::endl;
            myfile << "# ====================================================================================" << std::endl;
            myfile << "# GENERAL" << std::endl;
            myfile << std::endl;
            myfile << "# Working directory (default: USER_HOME if empty)." << std::endl;
            myfile << "# Example: /home/tschaffter" << std::endl;
            myfile << "workingDirectory = \"" << this->workingDirectory_ << "\"" << std::endl;
            myfile << "# List of comma-separated email addresses." << std::endl;
            myfile << "emailAddresses = \"" << this->emailAddresses_ << "\"" << std::endl;
            myfile << std::endl;
            myfile << "# ====================================================================================" << std::endl;
            myfile << "# CAMERAS" << std::endl;
            myfile << std::endl;
            myfile << "# Dc1394 mode (a=FireWire400, b=FireWire800, default: a)." << std::endl;
            myfile << "dc1394 = \"" << this->dc1394_ << "\"" << std::endl;
            myfile << "# Guid of the camera to select at startup (if detected)." << std::endl;
            myfile << "cameraGuid = \"" << this->cameraGuid_ << "\"" << std::endl;
            myfile << "# Configurations of all the cameras detected during the last session." << std::endl;
            myfile << "cameraConfigurations = \"" << this->cameraConfigurations_ << "\"" << std::endl;
            myfile << "# Trigger period in milliseconds." << std::endl;
            myfile << "triggerPeriod = " << this->triggerPeriod_ << std::endl;
            myfile << std::endl;
            myfile << "# ====================================================================================" << std::endl;
            myfile << "# PORT PLAYER" << std::endl;
            myfile << std::endl;
            myfile << "# Absolute path to the player settings file." << std::endl;
            myfile << "# Example: /home/tschaffter/squid/settings_player.txt" << std::endl;
            myfile << "playerSettingsFilename = \"" << this->playerSettingsFilename_ << "\"" << std::endl;
            myfile << std::endl;
            myfile << "# ====================================================================================" << std::endl;
            myfile << "# EXPERIMENT" << std::endl;
            myfile << std::endl;
            myfile << "# Experiment name." << std::endl;
            myfile << "experimentName = \"" << this->experimentName_ << "\"" << std::endl;
            myfile << "# Experiment duration mode (0=MANUAL, 1=FIXED, 2=PLAYER). In FIXED mode, the" << std::endl;
            myfile << "# experiment will stop after \"experimentDuration\" minutes (see next parameter)." << std::endl;
            myfile << "experimentDurationMode = " << this->experimentDurationMode_ << std::endl;
            myfile << "# Experiment duration in minutes (\"experimentDurationMode\" must be set to FIXED)." << std::endl;
            myfile << "experimentDuration = " << this->experimentDuration_ << std::endl;
            myfile << "# Send experiment report by email (1=yes, 0=no)." << std::endl;
            myfile << "experimentEmail = " << this->experimentEmail_ << std::endl;
            myfile << "# Subject prefix of the emails." << std::endl;
            myfile << "experimentEmailSubjectPrefix = \"" << this->experimentEmailSubjectPrefix_ << "\"" << std::endl;
            myfile << "# Output format (0=IMAGE_PGM, 1=IMAGE_TIFF)." << std::endl;
            myfile << "outputFormat = " << this->outputFormat_ << std::endl;
            myfile << std::endl;
            myfile << "# ====================================================================================" << std::endl;
            myfile << "# LOGGING" << std::endl;
            myfile << std::endl;
            myfile << "# Enable stderr logging (1=on, 0=off)." << std::endl;
            myfile << "stderrLogging = " << this->stderrLogging_ << std::endl;
            myfile << "# Stderr logging severity (0=INFO, 1=WARNING, 2=ERROR, 3=CRITICAL)." << std::endl;
            myfile << "stderrLoggingSeverity = " << this->stderrLoggingSeverity_ << std::endl;
            myfile << "# Enable file logging (1=on, 0=off)." << std::endl;
            myfile << "fileLogging = " << this->fileLogging_ << std::endl;
            myfile << "# File logging severity (0=INFO, 1=WARNING, 2=ERROR, 3=CRITICAL)." << std::endl;
            myfile << "fileLoggingSeverity = " << this->fileLoggingSeverity_ << std::endl;
            myfile << "# Directory where log files are saved (default: USER_HOME if empty)." << std::endl;
            myfile << "fileLoggingDirectory = \"" << this->fileLoggingDirectory_ << "\"" << std::endl;
            myfile << "# String prefix for logging filenames." << std::endl;
            myfile << "fileLoggingPrefix = \"" << this->fileLoggingPrefix_ << "\"" << std::endl;
            myfile << "# Enable email logging (1=on, 0=off)." << std::endl;
            myfile << "emailLogging = " << this->emailLogging_ << std::endl;
            myfile << "# Email logging severity (0=INFO, 1=WARNING, 2=ERROR, 3=CRITICAL)." << std::endl;
            myfile << "emailLoggingSeverity = " << this->emailLoggingSeverity_ << std::endl;
            myfile.close();

        } else
            throw new MyException("Unable to open settings file " + filename);
    } catch (std::exception* e) {
        throw new MyException(e->what());
    }
}

// ======================================================================
// GETTERS AND SETTERS

void SquidSettings::setWorkingDirectory(std::string directory) { workingDirectory_ = directory; }
std::string SquidSettings::getWorkingDirectory() { return workingDirectory_; }

void SquidSettings::setEmailAddress(std::string emailAddresses) { emailAddresses_ = emailAddresses; }
std::string SquidSettings::getEmailAddress() { return emailAddresses_; }

void SquidSettings::setExperimentName(std::string experimentName) { experimentName_ = experimentName; }
std::string SquidSettings::getExperimentName() { return experimentName_; }

void SquidSettings::setExperimentDuration(int duration) { experimentDuration_ = duration; }
int SquidSettings::getExperimentDuration() { return experimentDuration_; }

void SquidSettings::setSquid(Squid* squid) { squid_ = squid; }
Squid* SquidSettings::getSquid() { return squid_; }

void SquidSettings::setExperimentEmail(int experimentEmail) { experimentEmail_ = experimentEmail; }
int SquidSettings::getExperimentEmail() { return experimentEmail_; }

void SquidSettings::setCameraGuid(std::string str) { cameraGuid_ = str; }
std::string SquidSettings::getCameraGuid() { return cameraGuid_; }

void SquidSettings::setExperimentDurationMode(int mode) { experimentDurationMode_ = mode; }
int SquidSettings::getExperimentDurationMode() { return experimentDurationMode_; }

void SquidSettings::setExperimentEmailSubjectPrefix(std::string prefix) { experimentEmailSubjectPrefix_ = prefix; }
std::string SquidSettings::getExperimentEmailSubjectPrefix() { return experimentEmailSubjectPrefix_; }

void SquidSettings::setSettingsFile(std::string settingsFile) { settingsFile_ = settingsFile; }
std::string SquidSettings::getSettingsFile() { return settingsFile_; }

void SquidSettings::setStderrLogging(int stderrLogging) { stderrLogging_ = stderrLogging; }
int SquidSettings::getStderrLogging() { return stderrLogging_; }

void SquidSettings::setStderrLoggingSeverity(int severity) { stderrLoggingSeverity_ = severity; }
int SquidSettings::getStderrLoggingSeverity() { return stderrLoggingSeverity_; }

void SquidSettings::setFileLogging(int fileLogging) { fileLogging_ = fileLogging; }
int SquidSettings::getFileLogging() { return fileLogging_; }

void SquidSettings::setFileLoggingSeverity(int severity) { fileLoggingSeverity_ = severity; }
int SquidSettings::getFileLoggingSeverity() { return fileLoggingSeverity_; }

void SquidSettings::setFileLoggingPrefix(std::string prefix) { fileLoggingPrefix_ = prefix; }
std::string SquidSettings::getFileLoggingPrefix() { return fileLoggingPrefix_; }

void SquidSettings::setFileLoggingDirectory(std::string directory) { fileLoggingDirectory_ = directory; }
std::string SquidSettings::getFileLoggingDirectory() { return fileLoggingDirectory_; }

void SquidSettings::setEmailLogging(int emailLogging) { emailLogging_ = emailLogging; }
int SquidSettings::getEmailLogging() { return emailLogging_; }

void SquidSettings::setEmailLoggingSeverity(int severity) { emailLoggingSeverity_ = severity; }
int SquidSettings::getEmailLoggingSeverity() { return emailLoggingSeverity_; }

QIcon SquidSettings::getApplicationWindowIcon() { return applicationWindowIcon_; }

void SquidSettings::setOutputFormat(unsigned int format) { outputFormat_ = format; }
unsigned int SquidSettings::getOutputFormat() { return outputFormat_; }

void SquidSettings::setPlayerSettingsFilename(std::string filename) { playerSettingsFilename_ = filename; }
std::string SquidSettings::getPlayerSettingsFilename() { return playerSettingsFilename_; }

void SquidSettings::setTriggerPeriod(unsigned int period) { triggerPeriod_ = period; }
unsigned int SquidSettings::getTriggerPeriod() { return triggerPeriod_; }

void SquidSettings::setCameraConfigurations(std::string config) { cameraConfigurations_ = config; }
std::string SquidSettings::getCameraConfigurations() { return cameraConfigurations_; }

void SquidSettings::setDc1394(const std::string dc1394) { dc1394_ = dc1394; }
std::string SquidSettings::getDc1394() const { return dc1394_; }
