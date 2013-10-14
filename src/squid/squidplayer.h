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

#ifndef SQUIDPLAYER_H
#define SQUIDPLAYER_H

#include "qportplayerdialog.h"
#include "cameramanager.h"

//! Graphical interface of sQuid.
namespace qsquid {

/**
 * \brief Overrides qportplayer::QPortPlayerDialog to provide an interface to control the parallel port.
 *
 * @version February 29, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class SquidPlayer : public qportplayer::QPortPlayerDialog {

    Q_OBJECT

protected:

    /** Override qportcontrol::QPortControlDialog::setup() */
    virtual void setup();

 public:

    /** Destructor. */
    ~SquidPlayer() {}

    /** Returns the unique reference to the SquidPlayer. */
    static SquidPlayer* getInstance();

    /** Returns true if the checkbox "save" of the selected state (line of the playlist) is checked. */
    bool getSave(const unsigned int row);

public slots:

    /** Overrides qportplayer::QPortPlayerDialog::setPortManager() */
    virtual void setPortManager(portplayer::IOPinManager* ppManager);

    /**
     * Sets the "camera" comboboxes.
     * @deprecated
     */
    void setCameras(squid::CameraManager* cameras);

    /** Overrides qportcontrol::QPortControlDialog::loadGlobal(std::string). */
    virtual void loadGlobal(std::string filename);
    /** Overrides qportcontrol::QPortControlDialog::loadGlobal(). */
    virtual void loadGlobal();
    /** Override qportcontrol::QPortControlDialog::saveGlobal(). */
    virtual void saveGlobal();

    /** Overrides qportplayer::QPortPlayerDialog::importSettings(). */
    virtual void importSettings();
    /** Overrides qportplayer::QPortPlayerDialog::exportSettings(). */
    virtual void exportSettings();

    /** Shows the SquidPlayer. */
    virtual void show();

private:

    /** Constructor. */
    SquidPlayer(QWidget* parent = 0);

    /** Sets the "save" checkboxes of the playlist. */
    void setSaveCells();
    /**
     * Sets the "camera" comboboxes of the playlist.
     * @deprecated
     */
    void setCameraCells();

    /** Sets the "save" and "camera" columns of the playlist. */
    void loadSaveAndCameraConfiguration(const std::string config);
};

}

#endif // SQUIDPLAYER_H
