/* The MIT License (MIT)
 *
 * Copyright (c) 2021 Stefano Trettel
 *
 * Software repository: MoonUSB, https://github.com/stetre/moonusb
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

static void sectotv(struct timeval *tv, double seconds)
    {
    tv->tv_sec=(time_t)seconds;
    tv->tv_usec=(long)((seconds-((double)tv->tv_sec))*1.0e6);
    }

static double tvtosec(const struct timeval *tv)
    {
    return tv->tv_sec*1.0+tv->tv_usec*1.0e-6;
    }

static int Handle_events(lua_State *L)
    {
    int ec;
    double seconds;
    struct timeval tv;
    context_t *context = checkcontext(L, 1, NULL);
    if(lua_isnoneornil(L, 2)) /* blocking */
        ec = libusb_handle_events(context);
    else
        {
        seconds = luaL_checknumber(L, 2);
        sectotv(&tv, seconds);
        ec = libusb_handle_events_timeout(context, &tv);
        }
    CheckError(L, ec);
    return 0;
    }

static int Get_next_timeout(lua_State *L)
    {
    struct timeval tv;
    context_t *context = checkcontext(L, 1, NULL);
    int ec = libusb_get_next_timeout(context, &tv);
    if(ec==0) { lua_pushnil(L); return 1; } // no pending timeouts
    if(ec==1) { lua_pushnumber(L, tvtosec(&tv)); return 1; }
    CheckError(L, ec);
    return 0;
    }

static const struct luaL_Reg Methods[] = 
    {
        { "handle_events", Handle_events },
        { "get_next_timeout", Get_next_timeout },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_polling(lua_State *L)
    {
    udata_addmethods(L, CONTEXT_MT, Methods);
    luaL_setfuncs(L, Functions, 0);
    }

#if 0 //
/* File descriptor for polling */
struct libusb_pollfd {
    int fd;
    short events;
};
typedef void (*libusb_pollfd_added_cb)(int fd, short events, void *user_data);
typedef void (*libusb_pollfd_removed_cb)(int fd, void *user_data);
//int libusb_pollfds_handle_timeouts(context_t *context);
//const struct libusb_pollfd ** libusb_get_pollfds(context_t *context);
//void libusb_free_pollfds(const struct libusb_pollfd **pollfds);
//void libusb_set_pollfd_notifiers(context_t *context, libusb_pollfd_added_cb added_cb, libusb_pollfd_removed_cb removed_cb, void *user_data);
//int libusb_try_lock_events(context_t *context);
//void libusb_lock_events(context_t *context);
//void libusb_unlock_events(context_t *context);
//void libusb_interrupt_event_handler(context_t *context);
//void libusb_lock_event_waiters(context_t *context);
//void libusb_unlock_event_waiters(context_t *context);
//int libusb_event_handling_ok(context_t *context);
//int libusb_event_handler_active(context_t *context);
//int libusb_wait_for_event(context_t *context, struct timeval *tv);
//int libusb_handle_events_locked(context_t *context, struct timeval *tv);
//int libusb_handle_events_timeout_completed(context_t *context, struct timeval *tv, int *completed);
//int libusb_handle_events_completed(context_t *context, int *completed);
#endif

