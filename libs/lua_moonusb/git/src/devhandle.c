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

static int lock_on_close = 0;

static int freedevhandle(lua_State *L, ud_t *ud)
    {
    devhandle_t *devhandle = (devhandle_t*)ud->handle;
    context_t *context = ud->context;
    freechildren(L, HOSTMEM_MT, ud);
    freechildren(L, TRANSFER_MT, ud);
    freechildren(L, INTERFACE_MT, ud);
    if(!freeuserdata(L, ud, "devhandle")) return 0;
    if(lock_on_close)
        {
        libusb_lock_events(context);
        libusb_close(devhandle);
        libusb_unlock_events(context);
        }
    else
        libusb_close(devhandle);
    return 0;
    }

int newdevhandle(lua_State *L, device_t *device, devhandle_t *devhandle)
    {
    ud_t *ud;
    ud = newuserdata(L, devhandle, DEVHANDLE_MT, "devhandle");
    ud->parent_ud = userdata(device);
    ud->context = userdata(device)->context;
    ud->destructor = freedevhandle;
    // Automatically detach the kernel driver when an interface is claimed,
    // and re-attach it when the interface is released (only relevant on linux):
    (void)libusb_set_auto_detach_kernel_driver(devhandle, 1);
    return 1;
    }

static int ClaimInterface(lua_State *L)
    {
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    int interface_number = luaL_checkinteger(L, 2);
    newinterface(L, devhandle, interface_number);
    return 1;
    }

static int Get_descriptor(lua_State *L)
    {
    int ec;
    char *ptr=NULL;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    uint8_t desc_type = luaL_checkinteger(L, 2); /* libusb_descriptor_type */
    uint8_t desc_index = luaL_checkinteger(L, 3);
    char *buf = (char*)optlightuserdata(L, 4);
    size_t len = luaL_checkinteger(L, 5);
    if(len == 0)
        return argerror(L, 5, ERR_VALUE);
    if(!buf)
        {
        ptr = Malloc(L, len*sizeof(char));
        buf = ptr;
        }
    ec = libusb_get_descriptor(devhandle, desc_type, desc_index, (unsigned char*)ptr, (int)len);
    if(ec<0)
        { Free(L, ptr); CheckError(L, ec); }
    lua_pushlstring(L, buf, ec); // ec = no. of bytes received
    Free(L, ptr);
    return 1;
    }

static int Get_string_descriptor(lua_State *L)
    {
#define maxlen 256 /* the length field of the descriptor is 8 bits long */
    int ec;
    uint16_t langid;
    unsigned char data[maxlen];
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    uint8_t index = luaL_checkinteger(L, 2);
    if(index==0)
        { lua_pushnil(L); return 1; }
    if(lua_isnoneornil(L, 3))
        ec = libusb_get_string_descriptor_ascii(devhandle, index, data, maxlen);
    else
        {
        langid = luaL_checkinteger(L, 3);
        ec = libusb_get_string_descriptor(devhandle, index, langid, data, maxlen);
        }
    if(ec<0) CheckError(L, ec);
    lua_pushlstring(L, (char*)data, ec);
    return 1;
#undef maxlen
    }

static int Get_configuration(lua_State *L)
    {
    int value;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    int ec = libusb_get_configuration(devhandle, &value);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int Set_configuration(lua_State *L)
    {
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    int value = luaL_checkinteger(L, 2);
    int ec = libusb_set_configuration(devhandle, value);
    CheckError(L, ec);
    return 0;
    }

static int Clear_halt(lua_State *L)
    {
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char endpoint = luaL_checkinteger(L, 2);
    int ec = libusb_clear_halt(devhandle, endpoint);
    CheckError(L, ec);
    return 0;
    }

static int Reset_device(lua_State *L)
    {
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    int ec = libusb_reset_device(devhandle);
    CheckError(L, ec);
    return 0;
    }

static int Get_bos_descriptor(lua_State *L)
    {
    ud_t *ud;
    devhandle_t *devhandle = checkdevhandle(L, 1, &ud);
    struct libusb_bos_descriptor *bos;
    int ec = libusb_get_bos_descriptor(devhandle, &bos);
    CheckError(L, ec);
    pushbosdescriptor(L, bos, ud->context);
    libusb_free_bos_descriptor(bos);
    return 1;
    }

static int Alloc_streams(lua_State *L)
    {
    int num_endpoints, err, ec;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    uint32_t num_streams = luaL_checkinteger(L, 2);
    unsigned char *endpoints = checkucharlist(L, 3, &num_endpoints, &err);
    /* All endpoints must belong  to the same interface, otherwise the function fails */
    if(err) return argerror(L, 3, err);
    ec = libusb_alloc_streams(devhandle, num_streams, endpoints, num_endpoints);
    Free(L, endpoints);
    if(ec<0) CheckError(L, ec);
    lua_pushinteger(L, ec); // number of streams,  valid stream ids are 1, 2,..., ec.
    return 1;
    }

static int Free_streams(lua_State *L)
    {
    int num_endpoints, err, ec;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char *endpoints = checkucharlist(L, 2, &num_endpoints, &err);
    if(err) return argerror(L, 2, err);
    /* Note that streams are automatically free'd when the interface is released, so there
     * is no need for automatic cleanup here */
    ec = libusb_free_streams(devhandle, endpoints, num_endpoints);
    Free(L, endpoints);
    CheckError(L, ec);
    return 0;
    }

static int LockOnClose(lua_State *L)
    {
    lock_on_close = checkboolean(L, 1);
    return 0;
    }


RAW_FUNC(devhandle)
DESTROY_FUNC(devhandle)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "close", Destroy },
        { "claim_interface", ClaimInterface },
        { "get_descriptor", Get_descriptor },
        { "get_string_descriptor", Get_string_descriptor },
        { "get_configuration", Get_configuration },
        { "set_configuration", Set_configuration },
        { "clear_halt", Clear_halt },
        { "reset_device", Reset_device },
        { "get_bos_descriptor", Get_bos_descriptor },
        { "alloc_streams", Alloc_streams },
        { "free_streams", Free_streams },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "lock_on_close", LockOnClose },
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_devhandle(lua_State *L)
    {
    udata_define(L, DEVHANDLE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

#if 0
//device_t * libusb_get_device(devhandle_t *devhandle);
#endif
