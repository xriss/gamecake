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

static int freecontext(lua_State *L, ud_t *ud)
    {
    context_t *context = (context_t*)ud->handle;
    freechildren(L, HOTPLUG_MT, ud);
    freechildren(L, DEVICE_MT, ud);
    if(!freeuserdata(L, ud, "context")) return 0;
    libusb_exit(context);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud;
    context_t *context;
    int ec = libusb_init(&context);
    CheckError(L, ec);
    ud = newuserdata(L, context, CONTEXT_MT, "context");
    ud->parent_ud = NULL;
    ud->destructor = freecontext;
    return 1;
    }

static int Set_option(lua_State *L)
    {
    int level, ec=0;
    context_t *context;
    int option = checkoption(L, 2);
    switch(option)
        {
        case LIBUSB_OPTION_LOG_LEVEL:
            context = checkcontext(L, 1, NULL);
            level = checkloglevel(L, 3);
            ec = libusb_set_option(context, option, level);
            break;
        case LIBUSB_OPTION_USE_USBDK:
            context = checkcontext(L, 1, NULL);
            ec = libusb_set_option(context, option);
            break;
        case LIBUSB_OPTION_WEAK_AUTHORITY:
            ec = libusb_set_option(NULL, option);
            break;
        default:
            return unexpected(L);
        }
    CheckError(L, ec);
    return 0;
    }

static int log_cb_ref = LUA_NOREF; /* reference for global log cb */
static void LogCallback(context_t *context, enum libusb_log_level level, const char *str)
    {
#define L moonusb_L
    ud_t *ud;
    int top = lua_gettop(L);
    if(context) /* context callback */
        {
        ud = userdata(context);
        if(!ud) { unexpected(L); return; } 
        lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref1);
        pushcontext(L, context);
        }
    else /* global callback */
        {
        lua_rawgeti(L, LUA_REGISTRYINDEX, log_cb_ref);
        lua_pushnil(L);
        }
    pushloglevel(L, level);
    lua_pushstring(L, str);
    if(lua_pcall(L, 3, 0, 0) != LUA_OK)
        { lua_error(L); return; }
    lua_settop(L, top);
    return;
#undef L
    }

static int Set_log_cb(lua_State *L)
    {
    ud_t *ud;
    int mode;
    context_t *context = optcontext(L, 1, &ud);
    if(!lua_isfunction(L, 2))
        { return argerror(L, 2, ERR_FUNCTION); }
    if(context)
        {
        mode = LIBUSB_LOG_CB_CONTEXT;
        Reference(L, 2, ud->ref1);
        }
    else
        {
        mode = LIBUSB_LOG_CB_GLOBAL;
        Reference(L, 2, log_cb_ref);
        }
    libusb_set_log_cb(context, LogCallback, mode);
    return 0;
    }

static int Get_device_list(lua_State *L)
    {
    device_t **list;
    context_t *context = checkcontext(L, 1, NULL);
    ssize_t i, n = libusb_get_device_list(context, &list);
    if(n < 0) CheckError(L, n);
    lua_newtable(L);
    for(i=0; i<n; i++)
        {
        newdevice(L, context, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    libusb_free_device_list(list, 0); /* don't decrement ref count (see freedevice in device.c) */
    return 1;
    }

static int Open_device(lua_State *L)
    {
    device_t *device;
    context_t *context = checkcontext(L, 1, NULL);
    uint16_t vendor_id = luaL_checkinteger(L, 2);
    uint16_t product_id = luaL_checkinteger(L, 3);
    devhandle_t *devhandle = libusb_open_device_with_vid_pid(context, vendor_id, product_id);
    if(!devhandle) return luaL_error(L, "cannot open device");
    device = libusb_get_device(devhandle);
    if(!userdata(device))
        newdevice(L, context, device);
    else
        pushdevice(L, device);
    newdevhandle(L, device, devhandle);
    return 2; // device, devhandle
    }

#if 0
//@@int libusb_wrap_sys_device(context_t *context, intptr_t sys_dev, devhandle_t **devhandle);
static int Wrap_sys_device(lua_State *L)
    {
    context_t *context = checkcontext(L, 1, NULL);
    (void)context;
    return 0;
    }
#endif

RAW_FUNC(context)
DESTROY_FUNC(context)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "exit", Destroy },
        { "set_option", Set_option },
        { "set_log_cb", Set_log_cb },
        { "get_device_list", Get_device_list },
        { "open_device", Open_device },
//      { "wrap_sys_device", Wrap_sys_device },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "init", Create },
        { "set_option", Set_option },
        { "set_log_cb", Set_log_cb },
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_context(lua_State *L)
    {
    udata_define(L, CONTEXT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }



