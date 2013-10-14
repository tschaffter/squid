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

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "dc1394utility.h"
#include "dc1394/dc1394.h"
#include "myexception.h"
#include "dc1394framewriter.h"
#include <vector>
#include <sstream>
#include <cstring>
#include <QObject>
#include <boost/filesystem.hpp>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

#define REPORT_FILENAME "squid_report.txt"

/**
 * \brief Takes care of supervising the experiment including exporting frames to files.
 *
 * @version January 13, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Experiment : public QObject {

    Q_OBJECT

public:

        /** Experiment duration mode */
        enum durationMode {
            MANUAL = 0,
            FIXED = 1,
            PLAYER = 2
        };

        /** Experiment time format */
        enum timeFormat {
            ELAPSED = 0,
            REMAINING = 1
        };

private:

    /** Mutex for the thread. */
    pthread_mutex_t mutex_;
    /** Condition variable to pause the thread. */
    pthread_cond_t cond_;
    /** Id returned by pthread_create(). */
    pthread_t thread_;

    /** Is true if the experiment is running. */
    bool running_;
    /** Sets to true to abort. */
    bool abort_;
    /** Sets to true to pause the experiment. */
    bool pause_;

    /** Name of the experiment */
    std::string name_;
    /** Notes taken during the experiment */
    std::string description_;
    /** Sub-experiments ids */
    std::vector<std::string> subExperimentIds_;
    /** Working directory */
    std::string workingDirectory_;
    /** Absolute path towards experiment folder */
    std::string folder_;
    /** Absolute path towards each sub-experiment folders */
    std::vector<std::string> subExperimentFolders_;
    /** Output format */
    unsigned int outputFormat_;
    /** Experiment duration mode (MANUAL or SPECIFIED)  */
    durationMode durationMode_;
    /** Duration of experiment in seconds (> 0 if SPECIFIED, -1 if MANUAL) */
    unsigned int durationInUs_;
    /** String representing when experiment starts */
    std::string start_;
    /** String representing when experiment ends */
    std::string end_;
    /** Set if whether the elapsed or remaining time must be used */
    timeFormat format_;
    /** String suffix for image filenames */
    std::string frameSuffix_;
    /** Dedicated thread to save frames to file. */
    Dc1394FrameWriter* frameWriter_;

    /** Tells if the frames must be saved since the very beginning of the experiment. */
    bool saveFirstFrames_;

public:

    /** Constructor */
    Experiment();
    /** Destructor */
    ~Experiment();

    /** Initialization */
    void initialize();

    /** Setup (to be call just before start() or run()) */
    void setup() throw(MyException*, boost::filesystem::filesystem_error&);

    /** Set experiment identifier */
    void setName(std::string name);
    /** Get experiment identifier */
    std::string getName();

    /** Set sub-experiment identifiers */
    void setSubExperimentIds(std::vector<std::string> subExperimentIds);
    /** Get sub.experiment identifiers */
    std::vector<std::string> getSubExperimentIds();

    /** Set experiment description */
    void setDescription(std::string description);
    /** Get experiment description */
    std::string getDescription();

    /** Set duration mode */
    void setDurationMode(durationMode mode);
    /** Get duration mode */
    durationMode getDurationMode();

    /** Set experiment duration in us */
    void setDurationInUs(const unsigned int durationInUs);
    /** Get experiment duration in us */
    unsigned int getDurationInUs();

    /** Set time format (REMAINING or ELAPSED) */
    void setTimeFormat(timeFormat format);
    /** Get time format (REMAINING or ELAPSED) */
    timeFormat getTimeFormat();

    /** Set working directory */
    void setWorkingDirectory(std::string workingDirectory);
    /** Get working directory */
    std::string getWorkingDirectory();
    
    /** Get experiment folder */
    std::string getExperimentFolder();

    /** Set output format */
    void setOutputFormat(unsigned int format);
    /** Get output format */
    unsigned int getOutputFormat();

    /** Returns the absolute path to the experiment folder. */
    std::string getFolder();

    /** Sets if yes or no the first frames of the experiment must be saved. */
    void setSaveFirstFrames(bool saveFirstFrames);

public slots:

    /** Starts playing the experiment. */
    void start() throw(MyException*);
    /** Stop the experiment. */
    void stop() throw(MyException*);

    /** Returns true if the experiment is running. */
    bool isRunning();
    /** Returns true if the experiment has been aborted. */
    bool isAbort();

    /** Pauses or resumes the experiment. */
    void pause(bool pause) throw(MyException*);

    /** Save received frame as image */
    void saveFrame(dc1394video_frame_t* frame, unsigned int cameraIndex, unsigned int us);
    /** Save current experiment description to file */
    std::string saveDescription();
    /** Set string suffix for image filenames */
    void setFrameSuffix(std::string suffix);

signals:

    /** Sent when experiment finished (abort = true) */
    void finished();
    /** Sent to give the fraction of the experiment done (only id durationMode_ == FIXED) */
    void progress(float fraction);
    /** Sent total time since the experiment began in us (was ms) */
    void timeElapsedInUs(unsigned int timeElapsedInUs);
    /**
     * Sent to give the elapsed time since the beginning of the experiment OR
     * can also give the remaining time if durationMode_ == FIXED
     */
    void experimentTime(int hour, int min, int sec, int ms, int us);

private:

    /**
     * This is the static class function that serves as a C style function pointer
     * for the pthread_create call.
     */
    static void* processThread(void* obj);

};

} // end namespace squid

#endif // EXPERIMENT_H
