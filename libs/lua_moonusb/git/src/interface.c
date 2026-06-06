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

static int freeinterface(lua_State *L, ud_t *ud)
    {
    interface_t *interface = (interface_t*)ud->handle;
    int isclaimed = IsClaimed(ud);
//  freechildren(L, _MT, ud);
    if(!freeuserdata(L, ud, "interface")) return 0;
    if(isclaimed)
        libusb_release_interface(interface->devhandle, interface->number);
    Free(L, interface);
    return 0;
    }

int newinterface(lua_State *L, devhandle_t *devhandle, int interface_number)
    {
    int ec;
    ud_t *ud;
    interface_t *interface;
    interface = Malloc(L, sizeof(interface_t));
    interface->devhandle = devhandle;
    interface->number = interface_number;
    ud = newuserdata(L, interface, INTERFACE_MT, "interface");
    ud->parent_ud = userdata(devhandle);
    ud->context = userdata(devhandle)->context;
    ud->destructor = freeinterface;
    CancelClaimed(ud);
    ec = libusb_claim_interface(interface->devhandle, interface->number);
    if(ec)
        {
        ud->destructor(L, ud);
        lua_pop(L, 1);
        CheckError(L, ec);
        }
    MarkClaimed(ud);
    return 1;
    }

static int Set_alt_setting(lua_State *L)
    {
    interface_t *interface = checkinterface(L, 1, NULL);
    int alt_setting = luaL_checkinteger(L, 2);
    int ec = libusb_set_interface_alt_setting(interface->devhandle, interface->number, alt_setting);
    CheckError(L, ec);
    return 0;
    }

#if 0
static int Get_number(lua_State *L)
    {
    interface_t *interface = checkinterface(L, 1, NULL);
    lua_pushinteger(L, interface->number);
    return 1;
    }
#endif

DESTROY_FUNC(interface)
//PARENT_FUNC(interface)

static const struct luaL_Reg Methods[] = 
    {
        { "release", Destroy },
        { "set_alt_setting", Set_alt_setting },
//      { "get_number", Get_number },
//      { "get_devhandle", Parent },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_interface(lua_State *L)
    {
    udata_define(L, INTERFACE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

#if 0
// Not exposed (auto_detach is set when opening the device, see newdevhandle):
int libusb_set_auto_detach_kernel_driver(devhandle_t *devhandle, int enable);
int libusb_kernel_driver_active(devhandle_t *devhandle, int interface_number);
int libusb_detach_kernel_driver(devhandle_t *devhandle, int interface_number);
int libusb_attach_kernel_driver(devhandle_t *devhandle, int interface_number);
#endif
