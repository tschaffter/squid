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

#include "global.h"
#include "qportplayerdialog.h"
#include "myutility.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <glog/logging.h>

using namespace qportplayer;

Global* Global::instance_ = NULL;

// ======================================================================
// PRIVATE METHODS

Global::Global() {

    initialize();
}

// ----------------------------------------------------------------------

void Global::initialize() {

    settingsFile_ = "";
    pins_ = "";
    states_ = "";
    durationUnit_ = 1;
    config_file_options_ = NULL;
}

// ======================================================================
// PUBLIC METHODS

/** Returns the unique instance of Global. */
Global* Global::getInstance() {

    if (instance_ == NULL)
        instance_ = new Global();

    return instance_;
}

// ----------------------------------------------------------------------

Global::~Global() {}

// ----------------------------------------------------------------------

void Global::configureLogging() {

    // initialize Google's logging library
    google::InitGoogleLogging("");
    google::SetStderrLogging(0); // INFO
}

// ----------------------------------------------------------------------

void Global::parseArguments(int argc, char* argv[]) throw(MyException*) {

    try {
        // A group of options that will be allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "Print QPortPlayer version")
            ("help,h", "Display this help")
        ;

        // A group of options that will be allowed both on command line
        // and in config file
        po::options_description config("Settings");
        config.add_options()
            // ====================================================================================
            // COMMAND LINE OPTIONS
            ("settings-file,s", po::value<std::string>(&settingsFile_), "Specify a settings file")
            ("portDevice", po::value<std::string>(&portDeviceAbsPath_), "Absolute path to the port device (e.g. \"/dev/parport0\").")
            ("pins,p", po::value<std::string>(&pins_), "List of pins formatted as \"pin1_name pin1_port pin1_index pin2_name ... pinN_index\"")
            ("states", po::value<std::string>(&states_), "Semicolon-separated list of states (playlist)")
            ("durationUnit,u", po::value<int>(&durationUnit_), "Duration unit (0=min, 1=sec, 2=msec)")
        ;

        // A group of options that containt options that we will not display in help
        // Mainly option for config file
        po::options_description hidden("Hidden options");
        hidden.add_options()
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
        if (argv != NULL) {
            po::variables_map vm;
            int commandStyle =  po::command_line_style::default_style;// & ~po::command_line_style::allow_guessing;
            store(po::command_line_parser(argc, argv).options(cmdline_options).style(commandStyle).run(), vm);
            notify(vm);

            if (vm.count("help")) {
                std::cout << "QPortPlayer " << QPORTPLAYER_VERSION << " (" << QPORTPLAYER_VERSION_DATE << ")" << std::endl;
                std::cout << std::endl;
                std::cout << "QPortPlayer provides a framework to communicate with port interfaces." << std::endl;
                std::cout << "As an example, QPortPlayer allows here to control the parallel port (IEEE 1284)." << std::endl;
                std::cout << std::endl;
                std::cout << visible << std::endl;
                std::cout << std::endl;
                std::cout << "QPortPlayer is part of sQuid: http://tschaffter.ch/projects/squid/" << std::endl;
                std::cout << "Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)" << std::endl;
                exit(0);
            }

            if (vm.count("version")) {
                std::cout << "QPortPlayer " << QPORTPLAYER_VERSION << " (" << QPORTPLAYER_VERSION_DATE << ")" << std::endl;
                exit(0);
            }

            if (vm.count("settings-file"))
                load(vm["settings-file"].as<std::string>());
            else {
    //                        cout << "No Global file specified!" << std::endl << std::endl;
    //                        cout << visible << std::endl;
            }
        }
    } catch (std::exception& e) {
        throw new MyException(e.what());
    }
}

// ----------------------------------------------------------------------

void Global::load(std::string filename) throw(MyException*) {

    if (filename.length() == 0)
        throw new MyException("Invalid settings filename.");

    try {
        // if specified Global file exists
        if (std::ifstream(filename.c_str())) {
            // If here, Global file exists -> open it
            std::ifstream ifs(filename.c_str());

            // Build config_file_options_
            parseArguments(0, NULL);

            po::variables_map vm;
            store(parse_config_file(ifs, *config_file_options_), vm);
            notify(vm);

            // remove quotes from strings (if any)
            // this wasn't required with a previous version of boost
            stripLeadingAndEndingQuotes(portDeviceAbsPath_);
            stripLeadingAndEndingQuotes(pins_);
            stripLeadingAndEndingQuotes(states_);

            settingsFile_ = filename;

        } else
            throw new MyException("Unable to open settings file " + settingsFile_);

    } catch (boost::program_options::unknown_option& e) {
        throw new MyException(e.what());
    }
}

// ----------------------------------------------------------------------

void Global::save(std::string filename) throw(MyException*) {

    try {
        std::ofstream myfile;
        myfile.open(filename.c_str(), std::ios::out | std::ios::trunc);
        if (myfile.is_open()) {
            myfile << "#########################################################################" << std::endl;
            myfile << "# QPortPlayer settings file" << std::endl;
            myfile << "# QPortPlayer is part of sQuid: http://tschaffter.ch/projects/squid/" << std::endl;
            myfile << "#########################################################################" << std::endl;
            myfile << std::endl;
            myfile << "# Boolean values: 0 => false, 1 => true" << std::endl;
            myfile << std::endl;
            myfile << std::endl;
            myfile << "# Absolute path to the port device (e.g. \"/dev/parport0\")." << std::endl;
            myfile << "portDevice = \"" << this->portDeviceAbsPath_ << "\"" << std::endl;
            myfile << "# List of pins formatted as \"pin1_name pin1_port pin1_index pin2_name ... pinN_index\" (no whitespace in the elements)" << std::endl;
            myfile << "pins = \"" << this->pins_ << "\"" << std::endl;
            myfile << "# Semicolon-separated list of states (playlist)" << std::endl;
            myfile << "states = \"" << states_ << "\"" << std::endl;
            myfile << "# Duration unit (0=min, 1=sec, 2=msec)" << std::endl;
            myfile << "durationUnit = " << durationUnit_ << std::endl;
            myfile.close();

        } else
            throw new MyException("Unable to open settings file " + filename);
    }
    catch (std::exception* e) {
        throw new MyException(e->what());
    }
}

// ======================================================================
// GETTERS AND SETTERS

void Global::setSettingsFile(std::string settingsFile) { settingsFile_ = settingsFile; }
std::string Global::getSettingsFile() { return settingsFile_; }

void Global::setPortDeviceAbsPath(std::string portDeviceAbsPath) { portDeviceAbsPath_ = portDeviceAbsPath; }
std::string Global::getPortDeviceAbsPath() { return portDeviceAbsPath_; }

void Global::setPins(std::string pins) { pins_ = pins; }
std::string Global::getPins() { return pins_; }

void Global::setDurationUnit(int unit) { durationUnit_ = unit; }
int Global::getDurationUnit() { return durationUnit_; }

void Global::setStates(std::string states) { states_ = states; }
std::string Global::getStates() { return states_; }
