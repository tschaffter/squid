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

#ifndef DC1394FRAMEWRITER_H
#define DC1394FRAMEWRITER_H

#include "dc1394/dc1394.h"
#include "myexception.h"
#include <vector>
#include <QObject>

//! Library to control multiple cameras and manage the experiments.
namespace squid {

#define IMAGE_PGM_EXTENSION ".pgm"
#define IMAGE_TIFF_EXTENSION ".tif"

/**
 * \brief Saves frames to image files (e.g. with low priority).
 *
 * Saves to files all frames put in a list. The time displayed in filenames has
 * been taken right after finishing grabbing the image. Thus when the image are
 * saved is not critical. The desired behavior is that if there are images in the
 * stack to be saved, the writer runs until no image are left in the stack. If
 * there are no images in the stack, the writer wait SLEEPTIMEINMS before checking
 * again if the stack is empty. When the writer is stopped, it ensure that all
 * images still present in the stack are saved.
 *
 * @version February 21, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */
class Dc1394FrameWriter : public QObject {

    Q_OBJECT

private:

    /** Mutex for the thread. */
    pthread_mutex_t mutex_;
    /** Condition variable to pause the thread. */
    pthread_cond_t cond_;
    /** Id returned by pthread_create(). */
    pthread_t thread_;

    /** Is true if the frame writer is running. */
    bool running_;
    /** Sets to true to abort. */
    bool abort_;
    /** Sets to true to pause the framewriter. */
    bool pause_;

    /** Frames left to be saved. */
    std::vector<dc1394video_frame_t*> frames_;
    /** Filenames. */
    std::vector<std::string> filenames_;
    /** Image format. */
    std::vector<unsigned int> formats_;

public:

    /** Image formats. */
    enum format {
        IMAGE_PGM = 0,
        IMAGE_TIFF = 1
    };

    /** Constructor. */
    Dc1394FrameWriter();
    /** Destructor. */
    ~Dc1394FrameWriter();

    /** Add the following frame to the list of frames still to be written. */
    void push(dc1394video_frame_t* frame, std::string filename, unsigned int format = Dc1394FrameWriter::IMAGE_TIFF);

public slots:

    /** Starts the frame writer. */
    void start() throw(MyException*);
    /** Stop the frame writer. */
    void stop() throw(MyException*);

    /** Pauses or resumes the frame writer. */
    void pause(bool pause) throw(MyException*);

    /** Returns true if the frame writer is running. */
    bool isRunning();
    /** Returns true if the frame writer has been aborted. */
    bool isAbort();

private:

    /** Initializes the frame writer. */
    void initialize();

    /** Write the image to file. */
    void write(dc1394video_frame_t* frame, std::string filename, unsigned int format) throw(MyException*);

    /**
     * This is the static class function that serves as a C style function pointer
     * for the pthread_create call.
     */
    static void* processThread(void* obj);
};

} // end namespace squid

#endif // DC1394FRAMEWRITER_H
