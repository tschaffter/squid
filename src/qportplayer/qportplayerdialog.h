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

#ifndef QPORTPLAYERDIALOG_H
#define QPORTPLAYERDIALOG_H

#include "parallelportmanager.h"
#include "enhancedprogressbar.h"
#include "myexception.h"
#include <QDialog>
#include <QSpinBox>

#define QPORTPLAYER_VERSION "1.0.10 Beta"
#define QPORTPLAYER_VERSION_DATE "February 2012"

//! Elements of the graphical interface.
namespace Ui {
    class QPortPlayerDialog;
}
//! Graphical interface to control the environment (valves, LEDs, robots, etc.).
namespace qportplayer {

class QPortPlayerDialog;

/** Declares a typedef for pointer to a function returning void and taking an int argument (next state index). */
typedef void (QPortPlayerDialog::*pfv)(int);

/**
 * \brief Implements the main dialog of the <qportcontrol> application.
 *
 * @version February 27, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class QPortPlayerDialog : public QDialog {

    Q_OBJECT

protected:

    /** Port manager. */
    portplayer::IOPinManager* pManager_;
    /** Reference of the GUI. */
    Ui::QPortPlayerDialog* ui;
    /** Is true if the duration column is enabled. */
    bool durationColumnEnabled_;
    /** Progress bar for the state of the playlist. */
    EnhancedProgressBar* sequenceProgressBar_;
    /** Progress bar for the current state. */
    EnhancedProgressBar* stateProgressBar_;

    /** Function pointer to call just before changing state. */
    pfv preNextStateAction_;
    /** Function pointer to call just after changing state. */
    pfv postNextStateAction_;

public:

    /** Constructor. */
    QPortPlayerDialog(QWidget *parent = 0);
    /** Destructor. */
    ~QPortPlayerDialog();

    /** Returns the reference to the port manager. */
    portplayer::IOPinManager* getPortManager();

    /** Returns the name of the boolean items checked for the given state. */
    std::string getStateKeys(const unsigned int index);

public slots:

    /** Adds a state/line to the playlist. */
    void addState();
    /** Removes a state/line from the playlist. */
    void removeState();
    /** Applied the changes made to the playlist when closing EDITION mode (uses saveTableContent()). */
    void saveEditionAndClose();
    /** Refreshes the GUI. */
    void updateGui();
    /** Changes the mode of the application (EDITION, MANUAL, PLAYLIST or REMOTE). */
    void changeMode();
    /** Called when selecting a new state/line in the playlist. */
    void tableSelectionChanged();
    /** Saves the content of the table to playlist. */
    void saveTableContent();

    /** Starts playing the playlist. */
    void startPlaylist();
    /** Stops playing the playlist. */
    void stopPlaylist();
    /** Pauses playing the playlist. */
    void pausePlaylist(bool pause);
    /** Called when the playlist is done. */
    void playlistDone();

    /** Called when the state selected changed. */
    void stateChanged(const unsigned int currentState);

    /** Loads the application settngs from file. */
    virtual void loadGlobal(std::string filename);
    /** Loads the application settngs from file (opens a dialog to select the file). */
    virtual void loadGlobal();
    /** Saves the application settings to file. */
    virtual void saveGlobal();
    /** Sets the reference of the port manager. */
    virtual void setPortManager(portplayer::IOPinManager* ppManager);
    /** Called when a trigger has been send from a remote part of the code to move to the next state of the playlist. */
    void externalTriggerCatched(unsigned int index);
    /** Turns off the active elements of the application (e.g. the parallel port). */
    void turnOff();
    /** Applies the settings stored in Global. */
    virtual void importSettings();
    /** Stores the current settings to Global. */
    virtual void exportSettings();

    /** Sets pre next state function. */
    void setPreNextStateAction(pfv functionPtr);
    /** Returns pre next state function. */
    pfv getPreNextStateAction();

    /** Sets post next state function. */
    void setPostNextStateAction(pfv functionPtr);
    /** Returns post next state function. */
    pfv getPostNextStateAction();

    /** Default function called before changing state (do nothing by default). */
    void defaultPreNextStateAction(int /*nextStateIndex*/) {}
    /** Default function called after changing state (do nothing by default). */
    void defaultPostNextStateAction(int /*nextStateIndex*/) {}

signals:

    /** Sent when the selected state in the playlist changed. */
    void tableStateChanged(const unsigned int currentState);
    /** Sent when the playlist is done playing including normal and user-made stop. */
    void endOfSequence();

protected:

    void changeEvent(QEvent* event);
    /** Called when the user closes the window with the cross button. */
    void closeEvent(QCloseEvent* event);
    void retranslate();

    /** Initialization. */
    void initialize();

    /** Setup the GUI. */
    virtual void setup();

    /** Initializes the cells content of the table (playlist). */
    void setPinCells();
    /** Initializes the duration cells. */
    void setDurationCells();

    /** Creates a spin box to be placed into duration cells. */
    QSpinBox* createDurationQSpinBox();
    /** Initializes the output LEDs displayed in the GUI. */
    void setOutputLeds();
    /** Creates checkbox widget to be placed into cells. */
    void createCellWidget(const int row, const int column, QWidget* widget);
    /** Initializes playlist controls. */
    void setPlaylistPlayer();
    /** Enables or disables duration column. */
    void setDurationColumnEnabled(const bool state);
    /** Enables or disables cells (does include duration cells). */
    void setWidgetsInTableEnabled(const bool state);
    /** Makes connections. */
    void makeConnections();

public:

    /** Initializes the parallel port using the given configuration string. */
    static void getParallelPortConfiguration(const std::string config, const unsigned int numPins, std::string& sequence, std::string& durations);
};

}

#endif // QPORTPLAYERDIALOG_H
