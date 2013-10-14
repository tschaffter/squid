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
#include "ui_qportplayerdialog.h"
#include "booleanledwidget.h"
#include "global.h"
#include <sstream>
#include <QFileDialog>
#include <QMessageBox>
#include <glog/logging.h>

using namespace qportplayer;

// ======================================================================
// PUBLIC METHODS

QPortPlayerDialog::QPortPlayerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::QPortPlayerDialog) {

    ui->setupUi(this);
    setWindowTitle("QPortPlayer");
    pManager_ = NULL;
    sequenceProgressBar_ = NULL;
    stateProgressBar_ = NULL;
    makeConnections();
}

// ----------------------------------------------------------------------

QPortPlayerDialog::~QPortPlayerDialog() {

    delete ui;

    pManager_->setAllPinsLow();

    delete pManager_;
    delete sequenceProgressBar_;
    delete stateProgressBar_;

    ui = NULL;
    pManager_ = NULL;
    sequenceProgressBar_ = NULL;
    stateProgressBar_ = NULL;
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setPortManager(portplayer::IOPinManager* pManager) {

    delete pManager_;
    pManager_ = pManager;

    if (pManager_ != NULL) {
        initialize();
        setup();
        connect(ui->repeatSequenceCheckBox, SIGNAL(stateChanged(int)), pManager_->getPinPlaylist(), SLOT(repeatPlaylist(int)));
    }
    // applies the content of the playlist to the port manager
    saveTableContent();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::makeConnections() {

    connect(ui->addState, SIGNAL(clicked()), this, SLOT(addState()));
    connect(ui->removeState, SIGNAL(clicked()), this, SLOT(removeState()));
    connect(ui->editionRadioButton, SIGNAL(clicked()), this, SLOT(changeMode()));
    connect(ui->manualSelectionRadioButton, SIGNAL(clicked()), this, SLOT(changeMode()));
    connect(ui->sequenceRadioButton, SIGNAL(clicked()), this, SLOT(changeMode()));
    connect(ui->externalTriggerRadioButton, SIGNAL(clicked()), this, SLOT(changeMode()));
    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(tableSelectionChanged()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(saveEditionAndClose()));
    connect(ui->startSequenceButton, SIGNAL(clicked()), this, SLOT(startPlaylist()));
    connect(ui->stopSequenceButton, SIGNAL(clicked()), this, SLOT(stopPlaylist()));
    connect(ui->pauseSequenceButton, SIGNAL(toggled(bool)), this, SLOT(pausePlaylist(bool)));
    connect(ui->loadButton, SIGNAL(clicked()), this, SLOT(loadGlobal()));
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveGlobal()));
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::changeEvent(QEvent* event) {

    QDialog::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslate();
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::initialize() {

    durationColumnEnabled_ = true;
    ui->externalTriggerRadioButton->setVisible(false);
    preNextStateAction_ = (pfv) &QPortPlayerDialog::defaultPreNextStateAction;
    postNextStateAction_ = (pfv) &QPortPlayerDialog::defaultPostNextStateAction;
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setup() {

    setPinCells();
    setDurationCells();
    setOutputLeds();
    setPlaylistPlayer();

    ui->editionRadioButton->setChecked(true);

    const int unit = Global::getInstance()->getDurationUnit();
    if (unit == 0)
        ui->minRadioButton->setChecked(true);
    else if (unit == 1)
        ui->secRadioButton->setChecked(true);
    else if (unit == 2)
        ui->msecRadioButton->setChecked(true);

    pManager_->setAllPinsLow();

    // adpat dialog size to number of columns
    const double customWidth = 90 * ui->tableWidget->columnCount();
    const QSize qs(customWidth, this->minimumHeight());
    this->resize(qs);

    updateGui();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setPinCells() {

    // create the rows
    const unsigned int numRows = pManager_->getPinPlaylist()->getNumStates();
    ui->tableWidget->setRowCount(numRows);

    // create the columns
    const unsigned int numColumns = pManager_->getNumPins();
    ui->tableWidget->setColumnCount(numColumns);
    QHeaderView* header = ui->tableWidget->horizontalHeader();
    header->setResizeMode(QHeaderView::Stretch);

    // set table content
    QCheckBox* cb = NULL;
    for (unsigned int i = 0; i < numRows; i++) {
        // get ith state from sequence
        const std::vector<bool> state = pManager_->getPinPlaylist()->getPlaylist()->at(i);
        for (unsigned int j = 0; j < numColumns; j++) {
            cb = new QCheckBox();
            cb->setChecked(state.at(j));
            createCellWidget(i, j, cb);
        }
    }

    // set header
    std::vector<std::string> list = pManager_->getPinNames();
    QStringList qlist;
    for (unsigned int i = 0; i < list.size(); i++)
        qlist << QString(list.at(i).c_str());
    ui->tableWidget->setHorizontalHeaderLabels(qlist);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::updateGui() {

    const portplayer::IOPinManager::Mode mode = pManager_->getMode();
    bool b = (mode == portplayer::IOPinManager::EDITION);
    setWidgetsInTableEnabled(b);
    ui->addState->setEnabled(b);
    ui->removeState->setEnabled(b);   

    b = (mode == portplayer::IOPinManager::PLAYLIST || mode == portplayer::IOPinManager::REMOTE);
    ui->tableWidget->setEnabled(!b);

    if (mode == portplayer::IOPinManager::EDITION) {
        // set default table style sheet
        ui->tableWidget->setStyleSheet("");
    } else if (mode == portplayer::IOPinManager::MANUAL) {
        ui->tableWidget->clearSelection();
        ui->tableWidget->setStyleSheet("selection-background-color: #A00000; selection-color: #FFFFFF");
    } else if (mode == portplayer::IOPinManager::PLAYLIST) {
        ui->tableWidget->setStyleSheet("selection-background-color: #00A000; selection-color: #FFFFFF");
    } else if (mode == portplayer::IOPinManager::REMOTE) {
        ui->tableWidget->setStyleSheet("selection-background-color: #A0A0A0; selection-color: #FFFFFF");
    } else
        LOG(WARNING) << "IOPinManagerDialog::updateGui(): Must never arrive .";

    b = (pManager_->getMode() == portplayer::IOPinManager::PLAYLIST);
    ui->sequenceControlGroupBox->setEnabled(b);

    b = pManager_->getPinPlaylist()->repeatPlaylist() && (mode == portplayer::IOPinManager::PLAYLIST);
    ui->repeatSequenceCheckBox->setChecked(b);

    b = pManager_->getPinPlaylist()->isRunning() && (mode == portplayer::IOPinManager::PLAYLIST);
    ui->startSequenceButton->setEnabled(!b);
    ui->stopSequenceButton->setEnabled(b);
    ui->pauseSequenceButton->setEnabled(b);
    ui->modeGroupBox->setEnabled(!b);
    ui->loadButton->setEnabled(!b);
    ui->saveButton->setEnabled(!b);

    if (!b)
        ui->pauseSequenceButton->setChecked(false);

    this->repaint();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setDurationCells() {

    QSpinBox* sb = NULL;
    const unsigned int numStates = pManager_->getPinPlaylist()->getNumStates();
    const std::vector<unsigned int>* durations = pManager_->getPinPlaylist()->getStateDurations();

    // add one column to the table
    ui->tableWidget->setColumnCount(ui->tableWidget->columnCount() + 1);

    // set header
    QTableWidgetItem* header = new QTableWidgetItem();
    header->setText("Duration");
    ui->tableWidget->setHorizontalHeaderItem(ui->tableWidget->columnCount() - 1, header);

    // set content
    const unsigned int n = ui->tableWidget->columnCount() - 1;
    for (unsigned int i = 0; i < numStates; i++) {
        sb = createDurationQSpinBox();
        sb->setValue(durations->at(i));
        createCellWidget(i, n, sb);
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::createCellWidget(const int row, const int column, QWidget* widget) {

    QWidget* wdg = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(wdg);
    layout->addWidget(widget);
    layout->setAlignment(Qt::AlignCenter);
    wdg->setLayout(layout);
    ui->tableWidget->setCellWidget(row, column, wdg);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setDurationColumnEnabled(const bool state) {

    durationColumnEnabled_ = state;
    const unsigned int j = ui->tableWidget->columnCount() - 1;
    const unsigned int numStates = pManager_->getPinPlaylist()->getNumStates();

    for (unsigned int i = 0; i < numStates; i++)
        ui->tableWidget->cellWidget(i, j)->setEnabled(state);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setWidgetsInTableEnabled(const bool state) {

    const unsigned int numRows = ui->tableWidget->rowCount();
    const unsigned int numColumns = ui->tableWidget->columnCount();

    for (unsigned int i = 0; i < numRows; i++) {
        for (unsigned int j = 0; j < numColumns; j++)
            ui->tableWidget->cellWidget(i, j)->setEnabled(state);
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::addState() {

    QModelIndexList indexes = ui->tableWidget->selectionModel()->selection().indexes();
    int row = -1;

    if (indexes.empty()) // no selection
        row = ui->tableWidget->rowCount(); // add row to the bottom
    else
        row = indexes.at(0).row() + 1; // insert row after 1st row selected

    ui->tableWidget->insertRow(row);
    const unsigned int numPins = pManager_->getNumPins();

    for (unsigned int i = 0; i < numPins; i++)
        createCellWidget(row, i, new QCheckBox());

    createCellWidget(row, ui->tableWidget->columnCount()-2, createDurationQSpinBox());
    createCellWidget(row, ui->tableWidget->columnCount()-1, new QCheckBox());
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::removeState() {

    // get all selections
    QModelIndexList indexes = ui->tableWidget->selectionModel()->selection().indexes();
    int numColumns = ui->tableWidget->columnCount();
    int numDeletions = indexes.count() / numColumns;
    int row = -1;

    // removes rows from bottom to top
    for (int i = 0; i < numDeletions; i++) {
        row = indexes.at(numColumns * i).row();
        ui->tableWidget->removeRow(row-i);
        ui->tableWidget->selectRow(row);
    }
}

// ----------------------------------------------------------------------

QSpinBox* QPortPlayerDialog::createDurationQSpinBox() {

    QSpinBox* sb = new QSpinBox();
    sb->setRange(1, 9999999);
    sb->setValue(1000);
    sb->setAlignment(Qt::AlignRight);
    sb->setEnabled(durationColumnEnabled_);

    return sb;
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::saveEditionAndClose() {

    if (pManager_->getMode() == portplayer::IOPinManager::EDITION)
        saveTableContent();

    accept();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::closeEvent(QCloseEvent*) {

    saveEditionAndClose();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::saveTableContent() {

    std::vector< std::vector<bool> >* pinSequence = pManager_->getPinPlaylist()->getPlaylist();
    std::vector<unsigned int>* pinSequenceDurations = pManager_->getPinPlaylist()->getStateDurations();

    pinSequence->clear();
    pinSequenceDurations->clear();

    const unsigned int numStates = ui->tableWidget->rowCount();
    const unsigned int numPins = pManager_->getNumPins();

    QWidget* w = NULL;
    QCheckBox* c = NULL;
    QSpinBox* s = NULL;

    for (unsigned int i = 0; i < numStates; i++) {
        std::vector<bool> state;
        // save pin states
        for (unsigned int j = 0; j < numPins; j++) {
            w = ui->tableWidget->cellWidget(i, j);
            c = (QCheckBox*)w->layout()->itemAt(0)->widget();
            state.push_back(c->isChecked());
        }
        pinSequence->push_back(state);

        // save state duration
        w = ui->tableWidget->cellWidget(i, numPins);
        s = (QSpinBox*)w->layout()->itemAt(0)->widget();

        if (ui->minRadioButton->isChecked())
            pinSequenceDurations->push_back(s->value() * 1000 * 60);
        else if (ui->secRadioButton->isChecked())
            pinSequenceDurations->push_back(s->value() * 1000);
        else if (ui->msecRadioButton)
            pinSequenceDurations->push_back(s->value());
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::changeMode() {

    if (ui->editionRadioButton->isChecked()) {
        pManager_->setAllPinsLow();
        pManager_->setMode(portplayer::IOPinManager::EDITION);
    } else if (ui->manualSelectionRadioButton->isChecked()) {
        saveTableContent();
        pManager_->setMode(portplayer::IOPinManager::MANUAL);
    } else if (ui->sequenceRadioButton->isChecked()) {
        ui->tableWidget->clearSelection();
        saveTableContent();
        sequenceProgressBar_->setMaxDurationInMs(pManager_->getPinPlaylist()->getPlaylistTotalTime());
        stateProgressBar_->setMaxDurationInMs(pManager_->getPinPlaylist()->getStateDurations()->at(0));
        pManager_->setMode(portplayer::IOPinManager::PLAYLIST);
    } else if (ui->externalTriggerRadioButton->isChecked()) {
        ui->tableWidget->clearSelection();
        saveTableContent();
        pManager_->setMode(portplayer::IOPinManager::REMOTE);
    } else
        LOG(WARNING) << "Invalid QPortPlayer mode.";

    updateGui();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setOutputLeds() {

    const unsigned int numPins = pManager_->getNumPins();
    QGroupBox* gp = ui->effectiveOutputGroupBox;
    portplayer::IOPin* pin = NULL;

    if (gp->layout() == NULL) {
        QHBoxLayout* layout = new QHBoxLayout(gp);
        gp->setLayout(layout);
    }

    QLayoutItem *child = NULL;
    while ((child = gp->layout()->takeAt(0)) != 0) {
        delete child->widget();
    }

    for (unsigned int i = 0; i < numPins; i++) {
        pin = pManager_->getPins().at(i);

        BooleanLedWidget* led = new BooleanLedWidget();
        led->setBooleanItem(pin);
        led->setBooleanItemName(pin->getName());
        connect(pManager_->getPins().at(i), SIGNAL(stateChanged(bool)), led, SLOT(setState(bool)));

        gp->layout()->addWidget(led);
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::tableSelectionChanged() {

    const portplayer::IOPinManager::Mode mode = pManager_->getMode();

    if (mode == portplayer::IOPinManager::EDITION)
        return;

    QModelIndexList indexes = ui->tableWidget->selectionModel()->selection().indexes();

    if (indexes.count() == 0) {
        pManager_->setAllPinsLow();
        return;
    }

    const unsigned int numRowsSelected = indexes.count() / ui->tableWidget->columnCount();

    if (numRowsSelected < 1 && mode == portplayer::IOPinManager::MANUAL) {
        pManager_->setAllPinsLow();
        return;
    }

    const unsigned int rowIndex = indexes.at(0).row();
    LOG (INFO) << "Applying state " << rowIndex + 1 << ".";

    if (numRowsSelected > 1)
        LOG(WARNING) << "Multiple states selected: Only the state with the lowest index is taken into account ( " << rowIndex + 1 << ")";

    if (mode == portplayer::IOPinManager::MANUAL) {
        try {
            // XXX
            (this->*preNextStateAction_)(rowIndex);
            pManager_->applyState(rowIndex);
            (this->*postNextStateAction_)(rowIndex);

        } catch (MyException* e) {
            LOG(WARNING) << "Player::tableSelectionChanged(): " << e->getMessage();
        }
    } else if (mode == portplayer::IOPinManager::PLAYLIST) {
        // do nothing
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::setPlaylistPlayer() {

    if (sequenceProgressBar_ == NULL) {
        sequenceProgressBar_ = new EnhancedProgressBar();
        stateProgressBar_ = new EnhancedProgressBar();

        ui->stateGLayout->addWidget(stateProgressBar_, 0, 1);
        ui->stateGLayout->addWidget(sequenceProgressBar_, 1, 1);
    }

    sequenceProgressBar_->setTimeInMs(0);
    stateProgressBar_->setTimeInMs(0);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::startPlaylist() {

    try {
        // applies the content of the playlist to the port manager
        saveTableContent();

        pManager_->getPinPlaylist()->disconnect();

        connect(pManager_->getPinPlaylist(), SIGNAL(updatePlaylistTimeInMs(const unsigned int)), sequenceProgressBar_, SLOT(setTimeInMs(const unsigned int)));
        connect(pManager_->getPinPlaylist(), SIGNAL(updateStateTimeInMs(const unsigned int)), stateProgressBar_, SLOT(setTimeInMs(const unsigned int)));
        connect(pManager_->getPinPlaylist(), SIGNAL(stateChanged(const unsigned int)), this, SLOT(stateChanged(const unsigned int)));
        connect(pManager_->getPinPlaylist(), SIGNAL(done()), this, SLOT(playlistDone()));

        sequenceProgressBar_->setMaxDurationInMs(pManager_->getPinPlaylist()->getPlaylistTotalTime());
        stateProgressBar_->setMaxDurationInMs(pManager_->getPinPlaylist()->getStateDurations()->at(0));

        pManager_->getPinPlaylist()->setCurrentState(0);
        stateChanged(0);

        pManager_->getPinPlaylist()->start();

    } catch (MyException* e) {
        LOG(WARNING) << "Unable to play the playlist: " << e->getMessage();
    }
    updateGui();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::stopPlaylist() {

    try {
        pManager_->getPinPlaylist()->stop();

    } catch (MyException* e) {
        LOG (WARNING) << "Unable to stop the playlist: " << e->getMessage();
    }
    updateGui();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::pausePlaylist(bool pause) {

    try {
        pManager_->getPinPlaylist()->pause(pause);

    } catch (MyException* e) {
        LOG (WARNING) << "Unable to pause the playlist: " << e->getMessage();
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::playlistDone() {

    LOG (INFO) << "Playlist done.";
    emit endOfSequence(); // TODO rename

    // clearSelection() also calls tableSelectionChanged(), which switch off
    // all the pins of the port if no row are selected
    ui->tableWidget->clearSelection();

    updateGui();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::turnOff() {

    const portplayer::IOPinManager::Mode mode = pManager_->getMode();

    if (mode == portplayer::IOPinManager::PLAYLIST) {
        stopPlaylist();
        return;
    }

    if (mode == portplayer::IOPinManager::MANUAL || mode == portplayer::IOPinManager::REMOTE) {
        ui->tableWidget->clearSelection();
        updateGui();
        return;
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::stateChanged(const unsigned int currentState) {

    stateProgressBar_->setMaxDurationInMs(pManager_->getPinPlaylist()->getStateDurations()->at(currentState));

    // select the state in the table
    ui->tableWidget->setEnabled(true);
    ui->tableWidget->selectRow(currentState);
    ui->tableWidget->setEnabled(false);

    // XXX
    (this->*preNextStateAction_)(currentState);
    pManager_->applyState(currentState);
    (this->*postNextStateAction_)(currentState);

    emit tableStateChanged(currentState);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::loadGlobal(std::string filename) {

    LOG(INFO) << "Opening settings file " << filename;
    Global::getInstance()->load(filename);
    importSettings();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::loadGlobal() {

    QString file = QFileDialog::getOpenFileName(this, tr("Open Settings"));

    if (!file.isEmpty()) {
        try {
            loadGlobal(file.toStdString());

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to open player settings: " << e->what();
            QMessageBox msgBox;
            msgBox.setWindowTitle("QPortPlayer message");
            msgBox.setText("Unable to load settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::saveGlobal() {

    Global* global = Global::getInstance();
    QString file = QFileDialog::getSaveFileName(this, tr("Save Settings"));

    if (!file.isEmpty()) {
        try {
            LOG(INFO) << "Writing settings file " << global->getSettingsFile();
            exportSettings();
            global->setSettingsFile(file.toStdString());
            global->save(global->getSettingsFile());

        } catch (MyException* e) {
            LOG(WARNING) << "Unable to save player settings: " << e->what();
            QMessageBox msgBox;
            msgBox.setWindowTitle("QPortPlayer message");
            msgBox.setText("Unable to save settings file.");
            msgBox.setInformativeText((char*) e->what());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::importSettings() {

    Global* global = Global::getInstance();

    pManager_->load(global->getPortDeviceAbsPath(), global->getPins());
    // because the sequence is recreated inside load(), we have to reconnect it.
    connect(ui->repeatSequenceCheckBox, SIGNAL(stateChanged(int)), pManager_->getPinPlaylist(), SLOT(repeatPlaylist(int)));

    std::string pins = "";
    std::string sequence = "";
    std::string durations = "";
    QPortPlayerDialog::getParallelPortConfiguration(global->getStates(), pManager_->getNumPins(), sequence, durations);

    pManager_->getPinPlaylist()->loadPlaylist(sequence);
    pManager_->getPinPlaylist()->loadStateDurations(durations);

    const int unit = global->getDurationUnit();
    if (unit == 0)
        ui->minRadioButton->setChecked(true);
    else if (unit == 1)
        ui->secRadioButton->setChecked(true);
    else if (unit == 2)
        ui->msecRadioButton->setChecked(true);

    setup();
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::exportSettings() {

    Global* global = Global::getInstance();

    saveTableContent();

    global->setPins(pManager_->save());

    std::stringstream text;
    const unsigned int numStates = ui->tableWidget->rowCount();
    const unsigned int numPins = pManager_->getNumPins();
    for (unsigned int i = 0; i < numStates; i++) {
        for (unsigned int j = 0; j < numPins; j++) {
            QCheckBox* box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(i, j)->layout()->itemAt(0)->widget());
            text << box->isChecked();
            text << " ";
        }

        QSpinBox* box = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(i, numPins)->layout()->itemAt(0)->widget());
        text << box->value();

        if (i < numStates - 1)
            text << ";";
    }
    global->setStates(text.str());

    if (ui->minRadioButton->isChecked())
        global->setDurationUnit(0);
    else if (ui->secRadioButton->isChecked())
        global->setDurationUnit(1);
    else if (ui->msecRadioButton)
        global->setDurationUnit(2);
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::externalTriggerCatched(unsigned int index) {

    if (pManager_->getMode() != portplayer::IOPinManager::REMOTE) {
        LOG (WARNING) << "\"" << windowTitle().toStdString() << "\" received an external trigger but is not configured to respond to it";
        return;
    }
    pManager_->applyState(index % ui->tableWidget->rowCount());
}

// ----------------------------------------------------------------------

std::string QPortPlayerDialog::getStateKeys(const unsigned int index) {

    std::string output;
    QCheckBox* box = NULL;

    const unsigned int n = pManager_->getNumPins();
    std::vector<std::string> list = pManager_->getPinNames();
    for (unsigned int i = 0; i < n; i++) {
        box = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(index, i)->layout()->itemAt(0)->widget());
        if (box->isChecked())
            output += "_" + list[i];
    }
    return output;
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::getParallelPortConfiguration(const std::string config, const unsigned int numPins, std::string& sequence, std::string& durations) {

    std::string buffer = "";
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = config.find(';', pos)) != std::string::npos) {
        std::string substring(config.substr(prev_pos, pos-prev_pos));
        std::stringstream ss(substring);
        for (unsigned int i = 0; i < numPins; i++) {
            ss >> buffer;
            sequence += buffer;
        }
        ss >> buffer;
        durations += buffer;

        sequence += " ";
        durations += " ";

        prev_pos = ++pos;
    }
    std::string substring(config.substr(prev_pos, pos-prev_pos)); // Last word
    std::stringstream ss(substring);
    for (unsigned int i = 0; i < numPins; i++) {
        ss >> buffer;
        sequence += buffer;
    }
    ss >> buffer;
    durations += buffer;
}

// ----------------------------------------------------------------------

void QPortPlayerDialog::retranslate() {

    //ui->retranslateUi(this);
    //setWindowTitle(tr("Player"));
}

// ======================================================================
// GETTERS AND SETTERS

portplayer::IOPinManager* QPortPlayerDialog::getPortManager() { return pManager_; }

void QPortPlayerDialog::setPreNextStateAction(pfv functionPtr) { preNextStateAction_ = functionPtr; }
pfv QPortPlayerDialog::getPreNextStateAction() { return preNextStateAction_; }

void QPortPlayerDialog::setPostNextStateAction(pfv functionPtr) { postNextStateAction_ = functionPtr; }
pfv QPortPlayerDialog::getPostNextStateAction() { return postNextStateAction_; }
