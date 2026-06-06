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
    
int trace_objects = 0;

static int TraceObjects(lua_State *L)
    {
    trace_objects = checkboolean(L, 1);
    return 0;
    }

static int Now(lua_State *L)
    {
    lua_pushnumber(L, now());
    return 1;
    }

static int Since(lua_State *L)
    {
    double t = luaL_checknumber(L, 1);
    lua_pushnumber(L, since(t));
    return 1;
    }

/* ----------------------------------------------------------------------- */
 
static int Setlocale(lua_State *L)
    {
    const char* locale = luaL_checkstring(L, 1);
    int ec = libusb_setlocale(locale);
    CheckError(L, ec);
    return 0;
    }
 
static int has_cap = 0;
static int Has_capability(lua_State *L)
    {
    uint32_t cap = checkcapability(L, 1);
    if(!has_cap)
        return luaL_error(L, "the libusb_has_capability() API is not available");
    lua_pushboolean(L, libusb_has_capability(cap));
    return 1;
    }

static int Version(lua_State *L)
    {
    const struct libusb_version *v = libusb_get_version();
    lua_pushinteger(L, v->major);
    lua_pushinteger(L, v->minor);
    lua_pushinteger(L, v->micro);
    lua_pushinteger(L, v->nano);
    lua_pushfstring(L, "%s", v->rc ? v->rc : "???");
    return 5;
    }

//const char *libusb_error_name(int errcode);
//const char *libusb_strerror(int errcode);
int pusherrcode(lua_State *L, int ec)
    {
#if 0
    const char *s = libusb_strerror(ec);
    if(s)
        lua_pushstring(L, s);
    else
        lua_pushfstring(L, "unknown libusb error code %d", ec);
    return 1;
#endif
    switch(ec)
        {
#define CASE(code, s)   case code: lua_pushstring(L, s); break
        CASE(LIBUSB_SUCCESS, "success");
        CASE(LIBUSB_ERROR_IO, "io error");
        CASE(LIBUSB_ERROR_INVALID_PARAM, "invalid param");
        CASE(LIBUSB_ERROR_ACCESS, "access");
        CASE(LIBUSB_ERROR_NO_DEVICE, "no device");
        CASE(LIBUSB_ERROR_NOT_FOUND, "not found");
        CASE(LIBUSB_ERROR_BUSY, "busy");
        CASE(LIBUSB_ERROR_TIMEOUT, "timeout");
        CASE(LIBUSB_ERROR_OVERFLOW, "overflow");
        CASE(LIBUSB_ERROR_PIPE, "pipe");
        CASE(LIBUSB_ERROR_INTERRUPTED, "interrupted");
        CASE(LIBUSB_ERROR_NO_MEM, "no mem");
        CASE(LIBUSB_ERROR_NOT_SUPPORTED, "not supported");
        CASE(LIBUSB_ERROR_OTHER, "other");
//      positive values
//      CASE(LIBUSB_TRANSFER_COMPLETED, "success");
        CASE(LIBUSB_TRANSFER_ERROR, "error");
        CASE(LIBUSB_TRANSFER_TIMED_OUT, "timeout");
        CASE(LIBUSB_TRANSFER_CANCELLED, "cancelled");
        CASE(LIBUSB_TRANSFER_STALL, "stall");
        CASE(LIBUSB_TRANSFER_NO_DEVICE, "no device");
        CASE(LIBUSB_TRANSFER_OVERFLOW, "overflow");
#undef CASE
        default:
            lua_pushstring(L, "unknown");
        }
    return 1;
    }

static int AddVersions(lua_State *L)
    {
    const struct libusb_version *v;
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, "MoonUSB "MOONUSB_VERSION);
    lua_settable(L, -3);

    v = libusb_get_version();
    lua_pushstring(L, "_LIBUSB_VERSION");
    lua_pushfstring(L, "libusb %d.%d.%d", v->major, v->minor, v->micro);
    lua_settable(L, -3);
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "trace_objects", TraceObjects },
        { "now", Now },
        { "since", Since },
        { "setlocale", Setlocale },
        { "has_capability", Has_capability },
        { "get_version", Version },
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_tracing(lua_State *L)
    {
    AddVersions(L);
    luaL_setfuncs(L, Functions, 0);
    has_cap = libusb_has_capability(LIBUSB_CAP_HAS_CAPABILITY);
    }

