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

static int freehotplug(lua_State *L, ud_t *ud)
    {
    hotplug_t *hotplug = (hotplug_t*)ud->handle;
    context_t *context = ud->context;
//  freechildren(L, _MT, ud);
    if(!freeuserdata(L, ud, "hotplug")) return 0;
    libusb_hotplug_deregister_callback(context, hotplug->cb_handle);
    Free(L, hotplug);
    return 0;
    }

static int Callback(context_t *context, device_t *device, libusb_hotplug_event event, void *user_data)
    {
#define L moonusb_L
#define hotplug ((hotplug_t*)(user_data))
    int rc;
    ud_t *ud, *device_ud;
    int top = lua_gettop(L);
    ud = userdata(hotplug);
    if(!ud)
        { unexpected(L); return 0; }
    lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref1);
    pushcontext(L, context);
    device_ud = userdata(device);
    if(!device_ud) /* new device, create it */
        newdevice(L, context, device); /* this also pushes the device on the stack */
    else
        pushdevice(L, device);
    pushhotplugevent(L, event);
    rc = lua_pcall(L, 3, 1, 0);
    if(rc!=LUA_OK)
        { lua_error(L); return 0; }
    rc = lua_toboolean(L, -1); /* true -> deregister the callback, i.e. delete this hotplug */
    if(rc) ud->destructor(L, ud);
    lua_settop(L, top);
    return 0; /* returning 1 would cause the callback to be dereferenced */
#undef hotplug
#undef L
    }

static int Hotplug_register(lua_State *L)
    {
    ud_t *ud;
    int ec, ref = LUA_NOREF;
    hotplug_t *hotplug;
    context_t *context = checkcontext(L, 1, NULL);
    int events = checkhotplugevent(L, 2); //libusb_hotplug_event  
    int enumerate = optboolean(L, 4, 0);
    int vendor_id = luaL_optinteger(L, 5, LIBUSB_HOTPLUG_MATCH_ANY);
    int product_id = luaL_optinteger(L, 6, LIBUSB_HOTPLUG_MATCH_ANY);
    int dev_class = luaL_optinteger(L, 7, LIBUSB_HOTPLUG_MATCH_ANY);
    int flags = enumerate ? LIBUSB_HOTPLUG_ENUMERATE : 0;
    if(!lua_isfunction(L, 3)) 
        return argerror(L, 3, ERR_FUNCTION);
    Reference(L, 3, ref);
    hotplug = Malloc(L, sizeof(hotplug_t));
    ud = newuserdata(L, hotplug, HOTPLUG_MT, "hotplug");
    ud->parent_ud = userdata(context);
    ud->destructor = freehotplug;
    ud->context = context;
    ud->ref1 = ref;
    /* Note: if the LIBUSB_HOTPLUG_ENUMERATE flag is set, the callback is executed
     * repeatedly (once per attached device) before the register function returns, so
     * we must create the userdata before registering the callback.
     */
    ec = libusb_hotplug_register_callback(context, events, flags, vendor_id, product_id, 
            dev_class, Callback, hotplug, &(hotplug->cb_handle));
    if(ec)
        { ud->destructor(L, ud); CheckError(L, ec); return 0; }
    return 1;
    }

DESTROY_FUNC(hotplug)

static const struct luaL_Reg ContextMethods[] = 
    {
        { "hotplug_register", Hotplug_register },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Methods[] = 
    {
        { "deregister", Destroy },
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

void moonusb_open_hotplug(lua_State *L)
    {
    udata_define(L, HOTPLUG_MT, Methods, MetaMethods);
    udata_addmethods(L, CONTEXT_MT, ContextMethods);
    luaL_setfuncs(L, Functions, 0);
    }

