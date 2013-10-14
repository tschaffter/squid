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

#include "dc1394framewriter.h"
#include "dc1394utility.h"
#include "fdtimer.h"
#include <cstdlib>
#include <glog/logging.h>

using namespace squid;

// ======================================================================
// PRIVATE METHODS

void* Dc1394FrameWriter::processThread(void* obj) {

    Dc1394FrameWriter* fwriter = reinterpret_cast<Dc1394FrameWriter*>(obj);

    FdTimer timer;
    timer.initialize(0, 50000 * 1000); // 50 ms

    timer.start();  // generates the timeouts
    int fd = timer.getTimer();

    while (!(fwriter->isAbort() && fwriter->frames_.size() == 0)) {
        pthread_mutex_lock(&fwriter->mutex_);
        if (!fwriter->pause_) {
            pthread_mutex_unlock(&fwriter->mutex_);

            uint64_t numTimeout;
            read(fd, &numTimeout, sizeof(numTimeout));
//            if (numTimeout > 1)
//                LOG(WARNING) << "Playlist timer missed " << (numTimeout - 1) << " events.";

            dc1394video_frame_t* frame = NULL;
            std::string filename;
            unsigned int format;
            pthread_mutex_lock(&fwriter->mutex_);
            while (fwriter->frames_.size() > 0) {
                // get the reference to the variables
                frame = fwriter->frames_.front();
                filename = fwriter->filenames_.front();
                format = fwriter->formats_.front();
                // release the lock while writing the frame to file
                pthread_mutex_unlock(&fwriter->mutex_);
                fwriter->write(frame, filename, format);
                // lock again before deleting the data
                pthread_mutex_lock(&fwriter->mutex_);
                fwriter->frames_.erase(fwriter->frames_.begin());
                fwriter->filenames_.erase(fwriter->filenames_.begin());
                fwriter->formats_.erase(fwriter->formats_.begin());
            }
        }

        // pause the frame writer ?
        while (fwriter->pause_) {
            if (pthread_cond_wait(&fwriter->cond_, &fwriter->mutex_)) // wait for resume signal
                throw new MyException("Unable to suspend frame writer: pthread_cond_wait() failed.");
            LOG(INFO) << "Resuming frame writer.";
        }
        pthread_mutex_unlock(&fwriter->mutex_);
    }

    timer.stop();

    pthread_mutex_lock(&fwriter->mutex_);
    fwriter->running_ = false;
    pthread_mutex_unlock(&fwriter->mutex_);

    return NULL;
}

// ----------------------------------------------------------------------

void Dc1394FrameWriter::write(dc1394video_frame_t* frame, std::string filename, unsigned int format) throw(MyException*) {

    if (frame == NULL)
        throw new MyException("frame is null.");

    const unsigned int w = frame->size[0];
    const unsigned int h = frame->size[1];

    if (format == Dc1394FrameWriter::IMAGE_PGM)
        squid::grayscale8bitsToPgm(filename.c_str(), frame->image, w, h);
    else if (format == IMAGE_TIFF)
        squid::grayscale8bitsToTiff(filename.c_str(), frame->image, w, h);
    else
        throw new MyException("ERROR: Unknown image format.");
}

// ======================================================================
// PUBLIC METHODS

Dc1394FrameWriter::Dc1394FrameWriter() {

    if (pthread_mutex_init(&mutex_, NULL) == -1)
        throw new MyException("Unable to pthread_mutex_init().");
    if (pthread_cond_init(&cond_, NULL) == -1)
        throw new MyException("Unable to pthread_cond_init().");

    initialize();
}

// ----------------------------------------------------------------------

Dc1394FrameWriter::~Dc1394FrameWriter() {

    if (pthread_cond_destroy(&cond_) == -1)
        throw new MyException("Unable to pthread_cond_destroy().");
    if (pthread_mutex_destroy(&mutex_) == -1)
        throw new MyException("Unable to pthread_mutex_destroy().");
}

// ----------------------------------------------------------------------

void Dc1394FrameWriter::initialize() {

    running_ = false;
    pause_ = false;
    abort_ = false;
}

// ----------------------------------------------------------------------

void Dc1394FrameWriter::push(dc1394video_frame_t* frame, std::string filename, unsigned int format) {

    pthread_mutex_lock(&mutex_);
    frames_.push_back(frame);
    filenames_.push_back(filename);
    formats_.push_back(format);
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void Dc1394FrameWriter::start() throw(MyException*) {

    if (running_)
        throw new MyException("Frame writer is already running.");

    pthread_mutex_lock(&mutex_);
    abort_ = false;
    pause_ = false;

    if (pthread_create(&thread_, 0, Dc1394FrameWriter::processThread, this))
        throw new MyException("Unable to start frame writer thread: pthread_create() failed.");

    running_ = true;
    pthread_mutex_unlock(&mutex_);
}

// ----------------------------------------------------------------------

void Dc1394FrameWriter::stop() throw(MyException*) {

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

void Dc1394FrameWriter::pause(bool pause) throw(MyException*) {

    if (pause == pause_)
        return;

    if (pause) LOG(INFO) << "Suspending frame writer.";
    else LOG(INFO) << "Resuming frame writer.";

    pthread_mutex_lock(&mutex_);
    pause_ = pause;
    if (!pause) {
        if (pthread_cond_signal(&cond_))
            throw new MyException("Unable to resume frame writer thread: pthread_cond_signal() failed.");
    }
    pthread_mutex_unlock(&mutex_);
}

// ======================================================================
// GETTERS AND SETTERS

bool Dc1394FrameWriter::isAbort() { return abort_; }
bool Dc1394FrameWriter::isRunning() { return running_; }
