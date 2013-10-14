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

#ifndef MYUTILITY_H
#define MYUTILITY_H

#include "myexception.h"
#include <string>
#include <QListWidget>

/**
 * \brief Implements useful methods (conversion, Qt, time, etc.).
 *
 * @version February 26, 2012
 * @author Thomas Schaffter (thomas.schaff...@gmail.com)
 */

/**
 * STD UTILITY
 */
/** Decomposes a string whose elements are separated by space. */
std::vector<std::string> getTokensSeparatedBySpace(std::string str);

/** Removes the leading and ending quotes from the given string (if any). */
void stripLeadingAndEndingQuotes(std::string& str);


/**
 * CONVERSION UTILITY
 */
int intStringToInt(const std::string s) throw(MyException*);
std::string intToIntString(const int i) throw(MyException*);
std::string longToLongString(const long i) throw(MyException*);

int hexStringToInt(const std::string s);
std::string intToHexString(const int i);

std::string unsignedCharToString(const unsigned char uc);


/**
 * QT UTILITY
 */
/** Convert the content of a QListWidget to a vector of string */
std::vector<std::string> convertQListWidgetToStringVector(QListWidget* list);


/**
 * TIME UTILITY
 */
/** Transforms time in [ms] to [h], [min], [s], [ms] */
void formatTimeInMs(const unsigned int timeMs, unsigned int& h, unsigned int& min, unsigned int& s, unsigned int& ms);
/** Transforms time in [us] to [h], [min], [s], [ms], [us] */
void formatTimeInUs(const unsigned int t_us, unsigned int& h, unsigned int& min, unsigned int& s, unsigned int& ms, unsigned int& us);

/** Get current time in format YYYYMMDD (year month day) */
std::string getCurrentLocalYyyyMmDd(const char* separator);

/** Get current time in format HHMMSS (h min s) */
std::string getCurrentLocalHhMmSs(const char* separator);

#endif // MYUTILITY_H
