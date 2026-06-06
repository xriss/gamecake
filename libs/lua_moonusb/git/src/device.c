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

static int freedevice(lua_State *L, ud_t *ud)
    {
    device_t *device = (device_t*)ud->handle;
    freechildren(L, DEVHANDLE_MT, ud);
    if(!freeuserdata(L, ud, "device")) return 0;
    /* After having deleted all devhandles, now the ref count should be 1
     * and by decrementing it to 0 libusb should delete it */
    libusb_unref_device(device);
    return 0;
    }

int newdevice(lua_State *L, context_t *context, device_t *device)
    {
    ud_t *ud;
    libusb_ref_device(device);
    ud = newuserdata(L, device, DEVICE_MT, "device");
    ud->parent_ud = userdata(context);
    ud->context = context;
    ud->destructor = freedevice;
    return 1;
    }

static int Open(lua_State *L)
    {
    int ec;
    ud_t *ud;
    devhandle_t *devhandle;
    device_t *device = checkdevice(L, 1, &ud);
    ec = libusb_open(device, &devhandle); // this increases device's ref count
    CheckError(L, ec);
    newdevhandle(L, device, devhandle);
    return 1;
    }

static int Get_bus_number(lua_State *L)
    {
    device_t *device = checkdevice(L, 1, NULL);
    uint8_t n = libusb_get_bus_number(device);
    lua_pushinteger(L, n);
    return 1;
    }

static int Get_port_number(lua_State *L)
    {
    device_t *device = checkdevice(L, 1, NULL);
    uint8_t n = libusb_get_port_number(device);
    lua_pushinteger(L, n);
    return 1;
    }

static int Get_port_numbers(lua_State *L)
    {
    int i, rc;
    uint8_t *n;
    device_t *device = checkdevice(L, 1, NULL);
    int len = luaL_optinteger(L, 2, 8);
    if(len <= 0) return argerror(L, 2, ERR_LENGTH);
    n = Malloc(L, len*sizeof(uint8_t));
    rc = libusb_get_port_numbers(device, n, len);
    if(rc<0)
        { Free(L, n); CheckError(L, rc); }
    lua_newtable(L);
    for(i=0; i<rc; i++)
        {
        lua_pushinteger(L, n[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, n);
    return 1;
    }

static int Get_parent(lua_State *L)
    {
    ud_t *ud;
    device_t *device = checkdevice(L, 1, &ud);
    device_t *parent = libusb_get_parent(device);
    if(parent)
        {
        if(!userdata(parent)) /* unknown device: create it */
            newdevice(L, ud->context, parent);
        else
            pushdevice(L, parent);
        }
    else
        lua_pushnil(L);
    return 1;
    }

static int Get_device_address(lua_State *L)
    {
    device_t *device = checkdevice(L, 1, NULL);
    uint8_t address = libusb_get_device_address(device);
    lua_pushinteger(L, address);
    return 1;
    }

static int Get_device_speed(lua_State *L)
    {
    device_t *device = checkdevice(L, 1, NULL);
    int speed = libusb_get_device_speed(device);
    pushspeed(L, speed);
    return 1;
    }

#define F(Func, func)                                   \
static int Func(lua_State *L)                           \
    {                                                   \
    device_t *device = checkdevice(L, 1, NULL);         \
    unsigned char endpoint = luaL_checkinteger(L, 2);   \
    int rc = func(device, endpoint);                    \
    CheckError(L, rc);                                  \
    lua_pushinteger(L, rc);                             \
    return 1;                                           \
    }
F(Get_max_packet_size, libusb_get_max_packet_size)
F(Get_max_iso_packet_size, libusb_get_max_iso_packet_size)
#undef F

static int Get_device_descriptor(lua_State *L)
    {
    ud_t *ud;
    struct libusb_device_descriptor desc;
    device_t *device = checkdevice(L, 1, &ud);
    int ec = libusb_get_device_descriptor(device, &desc);
    CheckError(L, ec);
    pushdevicedescriptor(L, &desc, device, ud->context);
    return 1;
    }

static int Get_active_config_descriptor(lua_State *L)
    {
    ud_t *ud;
    struct libusb_config_descriptor *desc;
    device_t *device = checkdevice(L, 1, &ud);
    int ec = libusb_get_active_config_descriptor(device, &desc);
    CheckError(L, ec);
    pushconfigdescriptor(L, desc, ud->context);
    libusb_free_config_descriptor(desc);
    return 1;
    }

#define F(Func, func)                                       \
static int Func(lua_State *L)                               \
    {                                                       \
    ud_t *ud;                                               \
    struct libusb_config_descriptor *desc;                  \
    device_t *device = checkdevice(L, 1, &ud);              \
    uint8_t val = luaL_checkinteger(L, 2);                  \
    int ec = func(device, val, &desc);                      \
    CheckError(L, ec);                                      \
    pushconfigdescriptor(L, desc, ud->context);             \
    libusb_free_config_descriptor(desc);                    \
    return 1;                                               \
    }
F(Get_config_descriptor, libusb_get_config_descriptor)
F(Get_config_descriptor_by_value, libusb_get_config_descriptor_by_value)
#undef F

RAW_FUNC(device)
DESTROY_FUNC(device)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "open", Open },
        { "free", Destroy },
        { "get_bus_number", Get_bus_number },
        { "get_port_number", Get_port_number },
        { "get_port_numbers", Get_port_numbers },
        { "get_parent", Get_parent },
        { "get_address", Get_device_address },
        { "get_speed", Get_device_speed },
        { "get_max_packet_size", Get_max_packet_size },
        { "get_max_iso_packet_size", Get_max_iso_packet_size },
        { "get_device_descriptor", Get_device_descriptor },
        { "get_active_config_descriptor", Get_active_config_descriptor },
        { "get_config_descriptor", Get_config_descriptor },
        { "get_config_descriptor_by_value", Get_config_descriptor_by_value },
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

void moonusb_open_device(lua_State *L)
    {
    udata_define(L, DEVICE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }
 
#if 0
//device_t * libusb_ref_device(device_t *dev);
//void libusb_unref_device(device_t *dev);
#endif
