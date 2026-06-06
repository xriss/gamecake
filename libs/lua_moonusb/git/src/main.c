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

lua_State *moonusb_L;

static void AtExit(void)
    {
    if(moonusb_L)
        {
        enums_free_all(moonusb_L);
        moonusb_L = NULL;
        }
    }

 
int luaopen_moonusb(lua_State *L)
/* Lua calls this function to load the module */
    {
    moonusb_L = L;

    moonusb_utils_init(L);
    atexit(AtExit);

    lua_newtable(L); /* the module table */
    moonusb_open_enums(L);
//  moonusb_open_flags(L);
    moonusb_open_tracing(L);
    moonusb_open_context(L);
    moonusb_open_device(L);
    moonusb_open_devhandle(L);
    moonusb_open_synch(L);
    moonusb_open_transfer(L);
    moonusb_open_polling(L);
    moonusb_open_hotplug(L);
    moonusb_open_interface(L);
    moonusb_open_datahandling(L);
    moonusb_open_hostmem(L);

    /* Add functions implemented in Lua */
    lua_pushvalue(L, -1); lua_setglobal(L, "moonusb");
    if(luaL_dostring(L, "require('moonusb.bosdescriptors')") != 0) lua_error(L);
    if(luaL_dostring(L, "require('moonusb.utils')") != 0) lua_error(L);
    lua_pushnil(L);  lua_setglobal(L, "moonusb");

    return 1;
    }

