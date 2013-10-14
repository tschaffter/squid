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

#include "experimenttime.h"
#include <glog/logging.h>

using namespace squid;

/** Singleton instance */
static ExperimentTime* instance_ = NULL;

// ======================================================================
// PRIVATE METHODS

ExperimentTime::ExperimentTime() {

    time_ = new HighResolutionTime();
}

// ======================================================================
// PUBLIC METHODS

ExperimentTime* ExperimentTime::getInstance() {

    if (instance_ == NULL)
        instance_ = new ExperimentTime();

    return instance_;
}

// ----------------------------------------------------------------------

void ExperimentTime::initialize() {

    if (time_ != NULL)
        time_->initialize();
}

// ======================================================================
// GETTERS AND SETTERS

void ExperimentTime::setTimeReference(HighResolutionTime* time) { time_ = time; }
HighResolutionTime* ExperimentTime::getTimeReference() { return time_; }
