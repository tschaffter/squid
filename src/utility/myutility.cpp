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

#include "myutility.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <glog/logging.h>

#define BASE_TIME_YEAR_SHIFT 1900
#define BASE_TIME_MONTH_SHIFT 1
#define BASE_TIME_DAY_SHIFT 0

// ======================================================================
// STRING METHODS

std::vector<std::string> getTokensSeparatedBySpace(std::string str) {

    std::string buf; // Have a buffer string
    std::stringstream ss(str); // Insert the string into a stream

    std::vector<std::string> tokens; // Create vector to hold our words

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

// ----------------------------------------------------------------------

void stripLeadingAndEndingQuotes(std::string& str) {

    if (str.compare(0,1,"\"") == 0)
        str.erase(str.begin());
    if (str.compare(str.length()-1,str.length(),"\"") == 0)
        str.erase(str.end()-1);
}

// ======================================================================
// CONVERSION METHODS

int intStringToInt(const std::string s) throw(MyException*) {

    std::istringstream buffer(s);
    int output;
    buffer >> output;

    return output;
}

// ----------------------------------------------------------------------

std::string intToIntString(const int i) throw(MyException*) {

    std::ostringstream buffer;
    buffer << i;

   return buffer.str();
}

// ----------------------------------------------------------------------

std::string longToLongString(const long i) throw(MyException*) {

    std::ostringstream buffer;
    buffer << i;

   return buffer.str();
}

// ----------------------------------------------------------------------

int hexStringToInt(const std::string s) {

    return strtol(s.c_str() + 2, NULL, 16);
}

// ----------------------------------------------------------------------

std::string intToHexString(const int i) {

    char* buffer = new char[0];
    sprintf(buffer,"0x%x", i);

    return std::string(buffer);
}

// ----------------------------------------------------------------------

std::string unsignedCharToString(const unsigned char uc) {

    unsigned char c = uc;
    std::ostringstream buffer;

    while (c != 0) {
        if (c & 1)
            buffer << 1;
        else
            buffer << 0;

        c >>= 1;
    }

    if (buffer.str() == "")
        buffer << 0;

    std::string output = buffer.str();
    std::reverse(output.begin(), output.end());
    return output;
}

// ======================================================================
// QT METHODS

/**
 * Take all the items contained in a QListWidget, extract their text and
 * return a vector of string containing the item texts.
 * @param QListWidget* list
 * @returns vector<string> item texts
 */
std::vector<std::string> convertQListWidgetToStringVector(QListWidget* list) {

    int count = list->count();
    std::vector<std::string> output;

    for (int i=0; i<count; i++)
        output.push_back(list->item(i)->text().toStdString());

    return output;
}

// ======================================================================
// TIME METHODS

/**
 * Convert time in [ms] to [h], [min], [s], and [ms]
 * @param const int timeMs time in [ms]
 * @param int& number of [h]
 * @param int& number of [min]
 * @param int& number of [s]
 * @param int& number of [ms]
 */
void formatTimeInMs(const unsigned int timeMs, unsigned int& h, unsigned int& min, unsigned int& s, unsigned int& ms) {

    unsigned int time = timeMs;

    h = floor(time / 3600000);
    time -= h * 3600000;

    min = floor(time / 60000);
    time -= min * 60000;

    s = floor(time / 1000);
    time -= s * 1000;

    ms = time;
}

// ----------------------------------------------------------------------

/**
 * Convert time in [us] to [h], [min], [s], [ms] and [us]
 * @param const int timeMs time in [ms]
 * @param int& number of [h]
 * @param int& number of [min]
 * @param int& number of [s]
 * @param int& number of [ms]
 * @param int& number of [us]
 */
void formatTimeInUs(const unsigned int t_us, unsigned int& h, unsigned int& min, unsigned int& s, unsigned int& ms, unsigned int& us) {

    unsigned int time = t_us;

    h = floor(time / 3600000000u);
    time -= h * 3600000000u;

    min = floor(time / 60000000);
    time -= min * 60000000;

    s = floor(time / 1000000);
    time -= s * 1000000;

    ms = floor(time / 1000);
    time -= s * 1000;

    us = time;
}

// ----------------------------------------------------------------------

/**
 * Returns current date as a string with the format (year month day)
 * @param const char* string separator
 * @returns std::string formated date
 */
std::string getCurrentLocalYyyyMmDd(const char* separator) {

    time_t rawtime;
    tm* today;

    time(&rawtime);
    today = localtime(&rawtime);

    char timestamp[20];
    sprintf(timestamp, "%04d%s%02d%s%02d", (today->tm_year+BASE_TIME_YEAR_SHIFT),
            separator, (today->tm_mon+BASE_TIME_MONTH_SHIFT), separator,
            (today->tm_mday+BASE_TIME_DAY_SHIFT));

    return std::string(timestamp);
}

// ----------------------------------------------------------------------

/**
 * Returns current time as a string with the format (hour min s)
 * @param const char* string separator
 * @returns std::string formated date
 */
std::string getCurrentLocalHhMmSs(const char* separator) {

    time_t rawtime;
    tm* today;

    time(&rawtime);
    today = localtime(&rawtime); // rather than (gmtime(&rawtime))

    char timestamp[20];
    sprintf(timestamp, "%02d%s%02d%s%02d", today->tm_hour, separator,
            today->tm_min, separator, today->tm_sec);

    return std::string(timestamp);
}
