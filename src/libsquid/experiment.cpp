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

#include "experiment.h"
#include "experimenttime.h"
#include "myutility.h"
#include "cameramanager.h"
#include "fdtimer.h"
#include "rt.h"
#include <fstream>
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PRIVATE METHODS

void* Experiment::processThread(void* obj) {

    Experiment* experiment = reinterpret_cast<Experiment*>(obj);

    LOG(INFO) << "Promoting experiment to RT priority.";
    promoteRT();

    double pauseOffsetInMs = 0.;
    bool prematureAbortion = true;

    // experiment time
    ExperimentTime* time = ExperimentTime::getInstance();
    time->initialize();
    CameraManager::getInstance()->grabReferenceTimer_ = time->getTimeReference();

    // timer for update interface
    FdTimer timer;
    timer.initialize(0, 10000 * 1000); // 10 ms

    time->getTimeReference()->start();
    timer.start();  // generates the timeouts to refresh the interface
    int fd = timer.getTimer();

    // takes care of writing frames to files
    experiment->frameWriter_->start();

    // start saving the frames if required
    CameraManager::getInstance()->setSaveFrame(experiment->saveFirstFrames_);

    double tInMs = 0.;
    while (!experiment->isAbort()) {
        pthread_mutex_lock(&experiment->mutex_);
        if (!experiment->pause_) {
            pthread_mutex_unlock(&experiment->mutex_);

            uint64_t numTimeout;
            read(fd, &numTimeout, sizeof(numTimeout));
//            if (numTimeout > 1)
//                LOG(WARNING) << "Playlist timer missed " << (numTimeout - 1) << " events.";

            tInMs = time->getTimeReference()->getElapsedTimeInMs() - pauseOffsetInMs;
            emit experiment->timeElapsedInUs(tInMs * 1000);

            // timeout
            if (experiment->durationMode_ == FIXED || experiment->durationMode_ == PLAYER) {
                if ((tInMs * 1000) > experiment->durationInUs_) {
                    experiment->abort_ = true;
                    prematureAbortion = false;
                }
            }
        }

        // pause the experiment ?
        while (experiment->pause_) {
            double tInMsBkp = time->getTimeReference()->getElapsedTimeInMs();
            if (pthread_cond_wait(&experiment->cond_, &experiment->mutex_)) // wait for resume signal
                throw new MyException("Unable to suspend playlist: pthread_cond_wait() failed.");
            pauseOffsetInMs += time->getTimeReference()->getElapsedTimeInMs() - tInMsBkp;
            LOG(INFO) << "Resuming experiment after a break of " << pauseOffsetInMs/1000. << " seconds.";
        }
        pthread_mutex_unlock(&experiment->mutex_);
    }

    time->getTimeReference()->stop();
    timer.stop();

    pthread_mutex_lock(&experiment->mutex_);
    experiment->running_ = false;
    CameraManager::getInstance()->setSaveFrame(false);
    CameraManager::getInstance()->grabReferenceTimer_ = NULL;
    experiment->frameWriter_->stop();
    experiment->end_ = getCurrentLocalYyyyMmDd("-") + " " + getCurrentLocalHhMmSs("-"); // absolute end time

    if (!prematureAbortion)
        emit experiment->timeElapsedInUs(experiment->durationInUs_);
    emit experiment->finished();
    pthread_mutex_unlock(&experiment->mutex_);

    return NULL;
}

// ======================================================================
// PUBLIC METHODS

Experiment::Experiment() {

    if (pthread_mutex_init(&mutex_, NULL) == -1)
        throw new MyException("Unable to pthread_mutex_init().");
    if (pthread_cond_init(&cond_, NULL) == -1)
        throw new MyException("Unable to pthread_cond_init().");

    initialize();
}

// ----------------------------------------------------------------------

Experiment::~Experiment() {

    if (pthread_cond_destroy(&cond_) == -1)
        throw new MyException("Unable to pthread_cond_destroy().");
    if (pthread_mutex_destroy(&mutex_) == -1)
        throw new MyException("Unable to pthread_mutex_destroy().");
}

// ----------------------------------------------------------------------

void Experiment::initialize() {

    name_ = "myExperiment";
    description_ = "";
    subExperimentIds_.clear();
    workingDirectory_ = "";
    folder_ = "";
    subExperimentFolders_.clear();
    durationMode_ = MANUAL;
    durationInUs_ = -1;
    start_ = "";
    end_ = "";
    format_ = ELAPSED;
    running_ = false;
    abort_ = false;
    pause_ = false;
    frameSuffix_ = "";
    frameWriter_ = new Dc1394FrameWriter();
}

// ----------------------------------------------------------------------

/**
 * Prepare anything that should be done before running an exepriment.
 * Be sure that workingDirectory_ is correctly set before calling run().
 * Be sure to call setup() just before calling run().
 */
void Experiment::setup() throw(MyException*, boost::filesystem::filesystem_error&) {

    // verify that at least one sub-experiment has been specified
    if (subExperimentIds_.empty())
        throw new MyException("At least one sub-experiment is required.");

    pthread_mutex_lock(&mutex_);
    start_ = getCurrentLocalYyyyMmDd("-") + " " + getCurrentLocalHhMmSs("-"); // save start time

    std::string folderName = name_ + "_" + getCurrentLocalYyyyMmDd("-") + "_" + getCurrentLocalHhMmSs("-");
    folder_ = workingDirectory_ + "/" + folderName;

    if (boost::filesystem::exists(folder_) && boost::filesystem::is_directory(folder_))
        throw new MyException("Folder " + folder_ + " already exists!");

    // create the folder for the current experiment
    if (!boost::filesystem::create_directory(folder_))
        throw new MyException("Failed to create folder " + folder_);

    // create the folders for the sub experiments
    for (unsigned int i = 0; i < subExperimentIds_.size(); i++) {
        subExperimentFolders_.push_back(folder_ + "/" + subExperimentIds_.at(i) + "_" + getCurrentLocalYyyyMmDd("-") + "_" + getCurrentLocalHhMmSs("-"));
        if (!boost::filesystem::create_directory(subExperimentFolders_.at(i))) {
            std::string msg = "Failed to create sub-folder " + subExperimentFolders_.at(i) + "\n";
            msg += "Are you sure that ALL sub-experiments have a distinct identifier ?";
            throw new MyException(msg);
        }
    }
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void Experiment::start() throw(MyException*) {

    if (running_)
        throw new MyException("Experiment is already running.");

    pthread_mutex_lock(&mutex_);
    abort_ = false;
    pause_ = false;

    if (pthread_create(&thread_, 0, Experiment::processThread, this))
        throw new MyException("Unable to start experiment thread: pthread_create() failed.");

    running_ = true;
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

std::string Experiment::saveDescription() {

    std::stringstream ssDescription("");

    try {
        std::string filename = folder_ + "/" + REPORT_FILENAME;
        std::ofstream myfile;
        myfile.open(filename.c_str(), std::ios::out | std::ios::trunc);
        if (myfile.is_open()) {
            ssDescription << "Experiment name: " << name_ << std::endl;
            ssDescription << "Experiment directory: " << folder_ << std::endl;
            for (unsigned int i = 0; i < subExperimentIds_.size(); i++)
                ssDescription << "Camera " << i << " sub-directory: " << (std::string)(subExperimentIds_.at(i)) << std::endl;
            ssDescription << std::endl;
            ssDescription << "Start time: " << start_ << std::endl;
            ssDescription << "End time: " << end_ << std::endl;
            ssDescription << std::endl;
            ssDescription << "Notes:" << std::endl;
            ssDescription << description_ << std::endl;
            ssDescription << std::endl;
            ssDescription << "http://tschaffter.ch/projects/squid/" << std::endl;

            myfile << ssDescription.str();
            myfile.close();
        }
        else
            throw new MyException("Unable to open settings file " + filename);

    } catch (MyException* e) {
        LOG(ERROR) << "Unable to save experiment report to file: " << e->getMessage();
    }
    return ssDescription.str();
}

// ----------------------------------------------------------------------

/**
 * Save a dc1394 frame to an image file in the correct sub-experiment folder.
 * This implementation only supports dc1394 MONO8 frames.
 */
void Experiment::saveFrame(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int tInUs) {

    // XXX hack to avoid that frames are saved before the experiment timer is running
    if (tInUs == 0)
        return;

    unsigned int hour, min, sec, msec, us;
    char timestamp[20];

    formatTimeInUs(tInUs, hour, min, sec, msec, us);
    sprintf(timestamp, "%02d-%02d-%02d-%03d", hour, min, sec, msec);

    std::string filename = "";
    unsigned int format = -1;

    if (outputFormat_ == Dc1394FrameWriter::IMAGE_PGM) {
        filename = subExperimentFolders_.at(cameraIndex) + "/" + subExperimentIds_.at(cameraIndex) + "_" + timestamp + frameSuffix_ + IMAGE_PGM_EXTENSION;
        format = Dc1394FrameWriter::IMAGE_PGM;
    } else if (outputFormat_ == Dc1394FrameWriter::IMAGE_TIFF) {
        filename = subExperimentFolders_.at(cameraIndex) + "/" + subExperimentIds_.at(cameraIndex) + "_" + timestamp + frameSuffix_ + IMAGE_TIFF_EXTENSION;
        format = Dc1394FrameWriter::IMAGE_TIFF;
    } else
        LOG(WARNING) << "Unable to save frame: Unknowm image format.";

    // add the current frame to the list of frames still left to be saved
    frameWriter_->push(frame, filename, format);
}

// ----------------------------------------------------------------------

void Experiment::stop() throw(MyException*) {

    if (!running_)
        return;

    if (pause_)
        pause(false);
    pause_ = false;
    abort_ = true;
    pthread_join(thread_, NULL);
    running_ = false;
}

// ----------------------------------------------------------------------

void Experiment::pause(bool pause) throw(MyException*) {

    if (pause == pause_)
        return;

    if (pause) LOG(INFO) << "Suspending experiment.";
    else LOG(INFO) << "Resuming experiment.";

    pthread_mutex_lock(&mutex_);
    pause_ = pause;
    if (!pause) {
        if (pthread_cond_signal(&cond_))
            throw new MyException("Unable to resume experiment thread: pthread_cond_signal() failed.");
    }
    pthread_mutex_unlock(&mutex_);
}

// ======================================================================
// GETTERS AND SETTERS

bool Experiment::isAbort() { return abort_; }
bool Experiment::isRunning() { return running_; }

void Experiment::setName(std::string name) { name_ = name; }
std::string Experiment::getName() { return name_; }

void Experiment::setDescription(std::string description) { description_ = description; }
std::string Experiment::getDescription() { return description_; }

void Experiment::setDurationMode(durationMode mode) { durationMode_ = mode; }
Experiment::durationMode Experiment::getDurationMode() { return durationMode_; }

void Experiment::setDurationInUs(const unsigned int durationInUs) { durationInUs_ = durationInUs; }
unsigned int Experiment::getDurationInUs() { return durationInUs_; }

void Experiment::setTimeFormat(timeFormat format) { format_ = format; }
Experiment::timeFormat Experiment::getTimeFormat() { return format_; }

void Experiment::setSubExperimentIds(std::vector<std::string> subExperimentIds) { subExperimentIds_ = subExperimentIds; }
std::vector<std::string> Experiment::getSubExperimentIds() { return subExperimentIds_; }

void Experiment::setWorkingDirectory(std::string workingDirectory) { workingDirectory_ = workingDirectory; }
std::string Experiment::getWorkingDirectory() { return workingDirectory_; }

std::string Experiment::getExperimentFolder() { return folder_; }

void Experiment::setOutputFormat(unsigned int format) { outputFormat_ = format; }
unsigned int Experiment::getOutputFormat() { return outputFormat_; }

void Experiment::setFrameSuffix(std::string suffix) { frameSuffix_ = suffix; }

std::string Experiment::getFolder() { return folder_; }

void Experiment::setSaveFirstFrames(bool saveFirstFrames) { saveFirstFrames_ = saveFirstFrames; }
