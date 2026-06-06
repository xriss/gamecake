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

static int Control_transfer(lua_State *L)
// actual_length = f(devhandle, ptr, timeout)
// expects the setup packet in the first 8 bytes
    {
    int ec;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 2);
    int length = luaL_checkinteger(L, 3);
    unsigned int timeout = luaL_checkinteger(L, 4);
    struct libusb_control_setup *s = (struct libusb_control_setup*)ptr;
    unsigned char *data = ptr + sizeof(struct libusb_control_setup);
    if(length < (s->wLength + 8))
        return argerror(L, 3, ERR_VALUE);
    ec = libusb_control_transfer(devhandle, s->bmRequestType, s->bRequest, s->wValue, s->wIndex,
            data, s->wLength, timeout);
    if(ec>=0)
        {
        lua_pushinteger(L, ec); // actual length of data starting from ptr+8
        return 1;
        }
    CheckError(L, ec);
    return 0;
    }

// actual_length = f(devhandle, endpoint, ptr, length, timeout, datastring)
#define F(Func, func)                                                   \
static int Func(lua_State *L)                                           \
    {                                                                   \
    int ec, transferred;                                                \
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);                \
    unsigned char endpoint = luaL_checknumber(L, 2);                    \
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 3);      \
    int length = luaL_checkinteger(L, 4);                               \
    unsigned int timeout = luaL_checkinteger(L, 5);                     \
    ec = func(devhandle, endpoint, ptr, length, &transferred, timeout); \
    CheckError(L, ec);                                                  \
    lua_pushinteger(L, transferred);                                    \
    return 1;                                                           \
    }
F(Bulk_transfer, libusb_bulk_transfer)
F(Interrupt_transfer, libusb_interrupt_transfer)
#undef F

static const struct luaL_Reg Methods[] = 
    {
        { "control_transfer", Control_transfer },
        { "bulk_transfer", Bulk_transfer },
        { "interrupt_transfer", Interrupt_transfer },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_synch(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    udata_addmethods(L, DEVHANDLE_MT, Methods);
    }

