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

#include "qportplayerdialog.h"
#include "parallelportmanager.h"
#include "global.h"
#include <iostream>
#include <QApplication>
#include <QMessageBox>
#include <glog/logging.h>

#ifdef Q_OS_LINUX
    #include <csignal>
#endif

/**
 * Catch CTRL+C event to exit the application properly.
 * WARNING: It is plateform dependent, here it will only work properly on UNIX systems.
 */
void ctrl_c_exit (int signal);

/** Shows a dialog to show the error message. */
void displayErrorMessage(char* message);

// If it's a dynamical instance, destructor must be called explicitely.
qportplayer::QPortPlayerDialog* qppplayer_;

/**
 * Main method of QPortPlayer.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
int main(int argc, char* argv[]) {

    try {
        QApplication application(argc, argv);
        
        qportplayer::Global* settings = qportplayer::Global::getInstance();
        settings->configureLogging();
        settings->parseArguments(argc, argv);

        portplayer::ParallelPortManager* ppManager = NULL;

        // create a default ppManager
        if (settings->getSettingsFile().empty() && argc == 1)
            ppManager =  portplayer::ParallelPortManager::generateParallelPortExample();
        // create a ppManager from settings
        else {
            ppManager = new portplayer::ParallelPortManager();
            ppManager->load(settings->getPortDeviceAbsPath(), settings->getPins());

            std::string text = settings->getStates();
            std::string pinPlaylist = "";
            std::string stateDurations = "";
            std::string buffer = "";
            unsigned int numPins = ppManager->getNumPins();
            std::string::size_type prev_pos = 0, pos = 0;
            while((pos = text.find(';', pos)) != std::string::npos) {
                std::string substring(text.substr(prev_pos, pos-prev_pos));
                std::stringstream ss(substring);
                for (unsigned int i = 0; i < numPins; i++) {
                    ss >> buffer;
                    pinPlaylist += buffer;
                }
                ss >> buffer;
                stateDurations += buffer;

                pinPlaylist += " ";
                stateDurations += " ";

                prev_pos = ++pos;
            }
            std::string substring(text.substr(prev_pos, pos-prev_pos)); // Last word
            std::stringstream ss(substring);
            for (unsigned int i = 0; i < numPins; i++) {
                ss >> buffer;
                pinPlaylist += buffer;
            }
            ss >> buffer;
            stateDurations += buffer;

            ppManager->getPinPlaylist()->loadPlaylist(pinPlaylist);
            ppManager->getPinPlaylist()->loadStateDurations(stateDurations);
        }

        qportplayer::QPortPlayerDialog qppplayer;
        qppplayer.setPortManager(ppManager);
        qppplayer_ = &qppplayer;
        qppplayer.show();
        
        // CTRL+C handler
        #ifdef Q_OS_LINUX
            signal(SIGINT, ctrl_c_exit);
        #endif

        int value = application.exec();
        LOG(INFO) << std::endl;
        LOG(INFO) << "QPortPlayer is part of sQuid: http://tschaffter.ch/projects/squid/" << std::endl;
        LOG(INFO) << "Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)" << std::endl;
        return value;
    }
    catch (MyException* e) {
        displayErrorMessage((char*)e->what());

        if (qppplayer_ != NULL)
            qppplayer_->close();

        return 1;
    }
    catch (std::exception* e) {
        displayErrorMessage((char*)e->what());

        if (qppplayer_ != NULL)
            qppplayer_->close();

        return 1;
    }
}

// ---------------------------------------------------------------------- //

/**
 * Catches the CTRL+C action from the console.
 * @param int signal
 */
void ctrl_c_exit(int) {

    LOG(INFO) << "CTRL+C action caught, prepare to exit..." << std::endl;
    qppplayer_->close();
}

// ---------------------------------------------------------------------- //

/** Shows a dialog to show the error message. */
void displayErrorMessage(char* message) {

        LOG(ERROR) << "main(): " << message;

//        QMessageBox msgBox;
//        msgBox.setText("QPortControl encountered an error.");
//        msgBox.setInformativeText(message);
//        msgBox.setIcon(QMessageBox::Critical);
//        msgBox.exec();
}
