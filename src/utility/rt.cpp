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

#include <glog/logging.h>

//// RTKIT PART ....
#include <sys/types.h>
#include <dbus/dbus.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#define RTKIT_SERVICE_NAME "org.freedesktop.RealtimeKit1"
#define RTKIT_OBJECT_PATH "/org/freedesktop/RealtimeKit1"

static pid_t _gettid(void) {
        return (pid_t) syscall(SYS_gettid);
}

int rtkit_make_realtime(DBusConnection *connection, pid_t thread, int priority) {

        DBusMessage *m = NULL, *r = NULL;
        dbus_uint64_t u64;
        dbus_uint32_t u32;
        DBusError error;
        int ret;

        dbus_error_init(&error);

        if (thread == 0)
                thread = _gettid();

        if (!(m = dbus_message_new_method_call(
                              RTKIT_SERVICE_NAME,
                              RTKIT_OBJECT_PATH,
                              "org.freedesktop.RealtimeKit1",
                              "MakeThreadRealtime"))) {
                ret = -1;
                goto finish;
        }

        u64 = (dbus_uint64_t) thread;
        u32 = (dbus_uint32_t) priority;

        if (!dbus_message_append_args(
                            m,
                            DBUS_TYPE_UINT64, &u64,
                            DBUS_TYPE_UINT32, &u32,
                            DBUS_TYPE_INVALID)) {
                ret = -1;
                goto finish;
        }

        if (!(r = dbus_connection_send_with_reply_and_block(connection, m, -1, &error))) {
                ret = -1;
                goto finish;
        }


        if (dbus_set_error_from_message(&error, r)) {
                ret = -1;
                goto finish;
        }

        ret = 0;

finish:

        if (m)
                dbus_message_unref(m);

        if (r)
                dbus_message_unref(r);

        dbus_error_free(&error);

        return ret;
}

/// End of rtkit part


#define barrier() do{ asm volatile ("":::"memory"); } while(0)

// ----------------------------------------------------------------------

void promoteRT() {

        DBusError error;
        DBusConnection* dbus;

        // Rtkit wants our process to have a RLIMIT_RTTIME set, so let's do it !
        // RLIMIT_RTTIME is in us
        struct rlimit r;
        r.rlim_cur = 1000000; // 1 s
        r.rlim_max = 2000000; // 2 s

        if (setrlimit(RLIMIT_RTTIME, &r))
            LOG(WARNING) << "Unable to promote playlist to RT priority: Unable to set RLIMIT_RTTIME.";

        dbus_error_init(&error);
        dbus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (!dbus) {
            LOG(WARNING) << "Unable to promote playlist to RT priority: Unable to connect to dbus.";
            goto error_free;
        }

        int s;
        if ((s = rtkit_make_realtime(dbus, 0, 10)) < 0) {
            LOG(WARNING) << "Unable to promote playlist to RT priority: Unable to ask rtkit for realtime (status " << s << ").";
        } else
            LOG(INFO) << "Realtime successfully granted!";

        dbus_connection_unref(dbus);

error_free:
        dbus_error_free(&error);
}
