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

#include "highresolutiontime.h"
#include <glog/logging.h>

// ======================================================================
// PUBLIC METHODS

HighResolutionTime::HighResolutionTime() {

    initialize();
}

// ----------------------------------------------------------------------

HighResolutionTime::~HighResolutionTime() {}

// ----------------------------------------------------------------------

void HighResolutionTime::initialize() {

    startCount_.tv_sec = startCount_.tv_usec = 0;
    endCount_.tv_sec = endCount_.tv_usec = 0;
    stopped_ = true;
    startTimeInUs_ = 0;
    endTimeInUs_ = 0;
}

// ----------------------------------------------------------------------

void HighResolutionTime::start() {

    stopped_ = false;
    gettimeofday(&startCount_, NULL);
}

// ----------------------------------------------------------------------

void HighResolutionTime::stop() {

    stopped_ = true;
    gettimeofday(&endCount_, NULL);
}

// ----------------------------------------------------------------------

double HighResolutionTime::getElapsedTimeInUs() {

    if(!stopped_)
        gettimeofday(&endCount_, NULL);

    startTimeInUs_ = (startCount_.tv_sec * 1000000.0) + startCount_.tv_usec;
    endTimeInUs_ = (endCount_.tv_sec * 1000000.0) + endCount_.tv_usec;

    return endTimeInUs_ - startTimeInUs_;
}

// ----------------------------------------------------------------------

double HighResolutionTime::getElapsedTimeInMs() {

    return this->getElapsedTimeInUs() * 0.001;
}

// ----------------------------------------------------------------------

double HighResolutionTime::getElapsedTimeInSec() {

    return this->getElapsedTimeInUs() * 0.000001;
}

// ----------------------------------------------------------------------

double HighResolutionTime::getElapsedTime() {

    return this->getElapsedTimeInSec();
}

// ======================================================================
// GETTERS AND SETTERS

bool HighResolutionTime::isStopped() {

    return stopped_;
}
