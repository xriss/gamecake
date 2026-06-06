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

#ifndef internalDEFINED
#define internalDEFINED

#ifdef LINUX
#define _ISOC11_SOURCE /* see man aligned_alloc(3) */
#endif
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "moonusb.h"

#define TOSTR_(x) #x
#define TOSTR(x) TOSTR_(x)

#include "tree.h"
#include "objects.h"
#include "enums.h"

/* Note: all the dynamic symbols of this library (should) start with 'moonusb_' .
 * The only exception is the luaopen_moonusb() function, which is searched for
 * with that name by Lua.
 * MoonUSB's string references on the Lua registry also start with 'moonusb_'.
 */

#if 0
/* .c */
#define  moonusb_
#endif

/* flags.c */
#define checkflags(L, arg) luaL_checkinteger((L), (arg))
#define optflags(L, arg, defval) luaL_optinteger((L), (arg), (defval))
#define pushflags(L, val) lua_pushinteger((L), (val))

/* utils.c */
void moonusb_utils_init(lua_State *L);
#define copytable moonusb_copytable
int copytable(lua_State *L);
#define noprintf moonusb_noprintf
int noprintf(const char *fmt, ...); 
#define now moonusb_now
double now(void);
#define sleeep moonusb_sleeep
void sleeep(double seconds);
#define since(t) (now() - (t))
#define notavailable moonusb_notavailable
int notavailable(lua_State *L, ...);
#define Malloc moonusb_Malloc
void *Malloc(lua_State *L, size_t size);
#define MallocNoErr moonusb_MallocNoErr
void *MallocNoErr(lua_State *L, size_t size);
#define Strdup moonusb_Strdup
char *Strdup(lua_State *L, const char *s);
#define Free moonusb_Free
void Free(lua_State *L, void *ptr);
#define checkboolean moonusb_checkboolean
int checkboolean(lua_State *L, int arg);
#define testboolean moonusb_testboolean
int testboolean(lua_State *L, int arg, int *err);
#define optboolean moonusb_optboolean
int optboolean(lua_State *L, int arg, int d);
#define checklightuserdata moonusb_checklightuserdata
void *checklightuserdata(lua_State *L, int arg);
#define checklightuserdataorzero moonusb_checklightuserdataorzero
void *checklightuserdataorzero(lua_State *L, int arg);
#define optlightuserdata moonusb_optlightuserdata
void *optlightuserdata(lua_State *L, int arg);
#define testindex moonusb_testindex
int testindex(lua_State *L, int arg, int *err);
#define checkindex moonusb_checkindex
int checkindex(lua_State *L, int arg);
#define optindex moonusb_optindex
int optindex(lua_State *L, int arg, int optval);
#define pushindex moonusb_pushindex
void pushindex(lua_State *L, int val);

/* Internal error codes */
#define ERR_NOTPRESENT       1
#define ERR_SUCCESS          0
#define ERR_GENERIC         -1
#define ERR_TYPE            -2
#define ERR_ELEMTYPE        -3
#define ERR_VALUE           -4
#define ERR_ELEMVALUE       -5
#define ERR_TABLE           -6
#define ERR_FUNCTION        -7
#define ERR_EMPTY           -8
#define ERR_MEMORY          -9
#define ERR_MALLOC_ZERO     -10
#define ERR_LENGTH          -11
#define ERR_POOL            -12
#define ERR_BOUNDARIES      -13
#define ERR_RANGE           -14
#define ERR_FOPEN           -15
#define ERR_OPERATION       -16
#define ERR_UNKNOWN         -17
#define errstring moonusb_errstring
const char* errstring(int err);

/* device.c */
#define newdevice moonusb_newdevice
int newdevice(lua_State *L, context_t *context, device_t *device);

/* devhandle.c */
#define newdevhandle moonusb_newdevhandle
int newdevhandle(lua_State *L, device_t *device, devhandle_t *devhandle);

/* interface.c */
#define newinterface moonusb_newinterface
int newinterface(lua_State *L, devhandle_t *devhandle, int interface_number);

/* datahandling.c */
#define sizeoftype moonusb_sizeoftype
size_t sizeoftype(int type);
#define toflattable moonusb_toflattable
int toflattable(lua_State *L, int arg);
#define testdata moonusb_testdata
int testdata(lua_State *L, int type, size_t n, void *dst, size_t dstsize);
#define checkdata moonusb_checkdata
int checkdata(lua_State *L, int arg, int type, void *dst, size_t dstsize);
#define pushdata moonusb_pushdata
int pushdata(lua_State *L, int type, void *data, size_t datalen);

/* datastructs.c */
#define checkucharlist moonusb_checkucharlist
unsigned char* checkucharlist(lua_State *L, int arg, int *count, int *err);
#define checkcontrolsetup moonusb_checkcontrolsetup
int checkcontrolsetup(lua_State *L, int arg, struct libusb_control_setup *dst);
#define pushcontrolsetup moonusb_pushcontrolsetup
void pushcontrolsetup(lua_State *L, const struct libusb_control_setup *s);
#define pushdevicedescriptor moonusb_pushdevicedescriptor
int pushdevicedescriptor(lua_State *L, const struct libusb_device_descriptor *s, device_t *device, context_t *context);
#define pushconfigdescriptor moonusb_pushconfigdescriptor
void pushconfigdescriptor(lua_State *L, const struct libusb_config_descriptor *s, context_t *context);
#define pushbosdescriptor moonusb_pushbosdescriptor
void pushbosdescriptor(lua_State *L, struct libusb_bos_descriptor *s, context_t *context);

/* tracing.c */
#define pusherrcode moonusb_pusherrcode
int pusherrcode(lua_State *L, int ec);
#define trace_objects moonusb_trace_objects
extern int trace_objects;

/* main.c */
extern lua_State *moonusb_L;
int luaopen_moonusb(lua_State *L);
void moonusb_open_enums(lua_State *L);
//void moonusb_open_flags(lua_State *L);
void moonusb_open_tracing(lua_State *L);
void moonusb_open_context(lua_State *L);
void moonusb_open_device(lua_State *L);
void moonusb_open_devhandle(lua_State *L);
void moonusb_open_synch(lua_State *L);
void moonusb_open_transfer(lua_State *L);
void moonusb_open_polling(lua_State *L);
void moonusb_open_hotplug(lua_State *L);
void moonusb_open_interface(lua_State *L);
void moonusb_open_datahandling(lua_State *L);
void moonusb_open_hostmem(lua_State *L);

/*------------------------------------------------------------------------------*
 | Debug and other utilities                                                    |
 *------------------------------------------------------------------------------*/

#define CheckError(L, ec) \
    do { if(ec != LIBUSB_SUCCESS) { pusherrcode((L), (ec)); return lua_error(L); } } while(0)

/* If this is printed, it denotes a suspect bug: */
#define UNEXPECTED_ERROR "unexpected error (%s, %d)", __FILE__, __LINE__
#define unexpected(L) luaL_error((L), UNEXPECTED_ERROR)

/* Errors with internal error code (ERR_XXX) */
#define failure(L, errcode) luaL_error((L), errstring((errcode)))
#define argerror(L, arg, errcode) luaL_argerror((L), (arg), errstring((errcode)))
#define errmemory(L) luaL_error((L), errstring((ERR_MEMORY)))

#define notsupported(L) luaL_error((L), "operation not supported")
#define badvalue(L, s)   lua_pushfstring((L), "invalid value '%s'", (s))

/* Reference/unreference variables on the Lua registry */
#define Unreference(L, ref) do {                        \
    if((ref)!= LUA_NOREF)                               \
        {                                               \
        luaL_unref((L), LUA_REGISTRYINDEX, (ref));      \
        (ref) = LUA_NOREF;                              \
        }                                               \
} while(0)

#define Reference(L, arg, ref)  do {                    \
    Unreference((L), (ref));                            \
    lua_pushvalue(L, (arg));                            \
    (ref) = luaL_ref(L, LUA_REGISTRYINDEX);             \
} while(0)

/* DEBUG -------------------------------------------------------- */
#if defined(DEBUG)

#define DBG printf
#define TR() do { printf("trace %s %d\n",__FILE__,__LINE__); } while(0)
#define BK() do { printf("break %s %d\n",__FILE__,__LINE__); getchar(); } while(0)
#define TSTART double ts = now();
#define TSTOP do {                                          \
    ts = since(ts); ts = ts*1e6;                            \
    printf("%s %d %.3f us\n", __FILE__, __LINE__, ts);      \
    ts = now();                                             \
} while(0);

#else 

#define DBG noprintf
#define TR()
#define BK()
#define TSTART do {} while(0) 
#define TSTOP do {} while(0)    

#endif /* DEBUG ------------------------------------------------- */

#endif /* internalDEFINED */
