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

#include "squid.h"
#include <exception>
#include <iostream>
#include <qtsingleapplication.h>
#include <QMessageBox>
#include <glog/logging.h>

#ifdef Q_OS_LINUX
    #include <csignal>
#endif

/** Catches Ctrl+C event to exit the application properly (Unix only). */
void ctrl_c_exit (int signal);

// The destructor of dynamical instances must be called explicitely.
qsquid::Squid* squid_;

/**
 * Main method of sQuid.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
int main(int argc, char* argv[]) {

    try {
        // instantiate application
        QtSingleApplication application(argc, argv);

        if (application.isRunning()) {
            LOG(INFO) << "Only one instance of sQuid can be running at a time.";
            QMessageBox msgBox;
            msgBox.setWindowTitle("sQuid message");
            msgBox.setText("Unable to instantiate sQuid.");
            msgBox.setInformativeText("Only one instance of sQuid can be running at a time.\n\nTo kill any instance of sQuid:\n$ killall squid");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
            return 0;
        }

        qsquid::Squid squid(argc, argv);
        squid_ = &squid;
        application.setActivationWindow(&squid);
        squid.show();
        squid.setLocationOnDisplay();

        // CTRL+C handler
        #ifdef Q_OS_LINUX
            signal(SIGINT, ctrl_c_exit);
        #endif

        return application.exec();

    } catch (std::exception* e) {
        LOG(ERROR) << "Unable to start sQuid: " << e->what();
        QMessageBox msgBox;
        msgBox.setText("sQuid encountered an error.");
        msgBox.setInformativeText((char*)e->what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();

        squid_->close();

        return 1;
    }
}

// ---------------------------------------------------------------------- //

/** Catches the Ctrl+C action from the console. */
void ctrl_c_exit(int) {

    LOG(INFO) << "Ctrl+C action caught, preparing to exit." << std::endl;
    squid_->close();
}
