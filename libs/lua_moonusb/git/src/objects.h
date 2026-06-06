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

#ifndef objectsDEFINED
#define objectsDEFINED

#include "tree.h"
#include "udata.h"

/* libusb renaming (for internal use) */
#define context_t libusb_context
#define device_t libusb_device
#define devhandle_t libusb_device_handle
#define transfer_t struct libusb_transfer
#define hotplug_t moonusb_hotplug_t
#define interface_t moonusb_interface_t
#define hostmem_t moonusb_hostmem_t

typedef struct {
    libusb_hotplug_callback_handle cb_handle; /* this is unique per context */
} moonusb_hotplug_t;

typedef struct {
    int number; /* bInterfaceNumber */
    devhandle_t *devhandle;
} moonusb_interface_t;

/* host accessible memory: */
typedef struct {
    unsigned char *ptr;
    size_t size;
} moonusb_hostmem_t;

/* Objects' metatable names */
#define CONTEXT_MT "moonusb_context"
#define DEVICE_MT "moonusb_device"
#define DEVHANDLE_MT "moonusb_devhandle"
#define TRANSFER_MT "moonusb_transfer"
#define HOTPLUG_MT "moonusb_hotplug"
#define INTERFACE_MT "moonusb_interface"
#define HOSTMEM_MT "moonusb_hostmem"

/* Userdata memory associated with objects */
#define ud_t moonusb_ud_t
typedef struct moonusb_ud_s ud_t;

struct moonusb_ud_s {
    void *handle; /* the object handle bound to this userdata */
    int (*destructor)(lua_State *L, ud_t *ud);  /* self destructor */
    ud_t *parent_ud; /* the ud of the parent object */
    context_t *context;
    uint32_t marks;
    int ref1, ref2, ref3, ref4; /* refs for callbacks, automatically unreferenced at destruction */
    void *info; /* object specific info (ud_info_t, subject to Free() at destruction, if not NULL) */
};
    
/* Marks.  m_ = marks word (uint32_t) , i_ = bit number (0 .. 31)  */
#define MarkGet(m_,i_)  (((m_) & ((uint32_t)1<<(i_))) == ((uint32_t)1<<(i_)))
#define MarkSet(m_,i_)  do { (m_) = ((m_) | ((uint32_t)1<<(i_))); } while(0)
#define MarkReset(m_,i_) do { (m_) = ((m_) & (~((uint32_t)1<<(i_)))); } while(0)

#define IsValid(ud)             MarkGet((ud)->marks, 0)
#define MarkValid(ud)           MarkSet((ud)->marks, 0) 
#define CancelValid(ud)         MarkReset((ud)->marks, 0)

#define IsBorrowed(ud)          MarkGet((ud)->marks, 1)
#define MarkBorrowed(ud)        MarkSet((ud)->marks, 1) 
#define CancelBorrowed(ud)      MarkReset((ud)->marks, 1)

#define IsClaimed(ud)          MarkGet((ud)->marks, 2)
#define MarkClaimed(ud)        MarkSet((ud)->marks, 2) 
#define CancelClaimed(ud)      MarkReset((ud)->marks, 2)

#define IsAllocated(ud)         MarkGet((ud)->marks, 3)
#define MarkAllocated(ud)       MarkSet((ud)->marks, 3) 
#define CancelAllocated(ud)     MarkReset((ud)->marks, 3)

#define IsDma(ud)               MarkGet((ud)->marks, 4)
#define MarkDma(ud)             MarkSet((ud)->marks, 4) 
#define CancelDma(ud)           MarkReset((ud)->marks, 4)

#define IsSubmitted(ud)         MarkGet((ud)->marks, 5)
#define MarkSubmitted(ud)       MarkSet((ud)->marks, 5) 
#define CancelSubmitted(ud)     MarkReset((ud)->marks, 5)

#if 0
/* .c */
#define  moonusb_
#endif

#define setmetatable moonusb_setmetatable
int setmetatable(lua_State *L, const char *mt);

#define newuserdata moonusb_newuserdata
ud_t *newuserdata(lua_State *L, void *handle, const char *mt, const char *tracename);
#define freeuserdata moonusb_freeuserdata
int freeuserdata(lua_State *L, ud_t *ud, const char *tracename);
#define pushuserdata moonusb_pushuserdata 
int pushuserdata(lua_State *L, ud_t *ud);

#define freechildren moonusb_freechildren
int freechildren(lua_State *L,  const char *mt, ud_t *parent_ud);

#define userdata_unref(L, handle) udata_unref((L),(handle))

#define UD(handle) userdata((handle)) /* dispatchable objects only */
#define userdata moonusb_userdata
ud_t *userdata(const void *handle);
#define testxxx moonusb_testxxx
void *testxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define checkxxx moonusb_checkxxx
void *checkxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define optxxx moonusb_optxxx
void *optxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define pushxxx moonusb_pushxxx
int pushxxx(lua_State *L, void *handle);
#define checkxxxlist moonusb_checkxxxlist
void** checkxxxlist(lua_State *L, int arg, int *count, int *err, const char *mt);

/* context.c */
#define checkcontext(L, arg, udp) (context_t*)checkxxx((L), (arg), (udp), CONTEXT_MT)
#define testcontext(L, arg, udp) (context_t*)testxxx((L), (arg), (udp), CONTEXT_MT)
#define optcontext(L, arg, udp) (context_t*)optxxx((L), (arg), (udp), CONTEXT_MT)
#define pushcontext(L, handle) pushxxx((L), (void*)(handle))
#define checkcontextlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), CONTEXT_MT)

/* device.c */
#define checkdevice(L, arg, udp) (device_t*)checkxxx((L), (arg), (udp), DEVICE_MT)
#define testdevice(L, arg, udp) (device_t*)testxxx((L), (arg), (udp), DEVICE_MT)
#define optdevice(L, arg, udp) (device_t*)optxxx((L), (arg), (udp), DEVICE_MT)
#define pushdevice(L, handle) pushxxx((L), (void*)(handle))
#define checkdevicelist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), DEVICE_MT)

/* devhandle.c */
#define checkdevhandle(L, arg, udp) (devhandle_t*)checkxxx((L), (arg), (udp), DEVHANDLE_MT)
#define testdevhandle(L, arg, udp) (devhandle_t*)testxxx((L), (arg), (udp), DEVHANDLE_MT)
#define optdevhandle(L, arg, udp) (devhandle_t*)optxxx((L), (arg), (udp), DEVHANDLE_MT)
#define pushdevhandle(L, handle) pushxxx((L), (void*)(handle))
#define checkdevhandlelist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), DEVHANDLE_MT)

/* transfer.c */
#define checktransfer(L, arg, udp) (transfer_t*)checkxxx((L), (arg), (udp), TRANSFER_MT)
#define testtransfer(L, arg, udp) (transfer_t*)testxxx((L), (arg), (udp), TRANSFER_MT)
#define opttransfer(L, arg, udp) (transfer_t*)optxxx((L), (arg), (udp), TRANSFER_MT)
#define pushtransfer(L, handle) pushxxx((L), (void*)(handle))
#define checktransferlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), TRANSFER_MT)

/* hotplug.c */
#define checkhotplug(L, arg, udp) (hotplug_t*)checkxxx((L), (arg), (udp), HOTPLUG_MT)
#define testhotplug(L, arg, udp) (hotplug_t*)testxxx((L), (arg), (udp), HOTPLUG_MT)
#define opthotplug(L, arg, udp) (hotplug_t*)optxxx((L), (arg), (udp), HOTPLUG_MT)
#define pushhotplug(L, handle) pushxxx((L), (void*)(handle))
#define checkhotpluglist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), HOTPLUG_MT)

/* interface.c */
#define checkinterface(L, arg, udp) (interface_t*)checkxxx((L), (arg), (udp), INTERFACE_MT)
#define testinterface(L, arg, udp) (interface_t*)testxxx((L), (arg), (udp), INTERFACE_MT)
#define optinterface(L, arg, udp) (interface_t*)optxxx((L), (arg), (udp), INTERFACE_MT)
#define pushinterface(L, handle) pushxxx((L), (void*)(handle))
#define checkinterfacelist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), INTERFACE_MT)

/* hostmem.c */
#define checkhostmem(L, arg, udp) (hostmem_t*)checkxxx((L), (arg), (udp), HOSTMEM_MT)
#define testhostmem(L, arg, udp) (hostmem_t*)testxxx((L), (arg), (udp), HOSTMEM_MT)
#define opthostmem(L, arg, udp) (hostmem_t*)optxxx((L), (arg), (udp), HOSTMEM_MT)
#define pushhostmem(L, handle) pushxxx((L), (void*)(handle))
#define checkhostmemlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), HOSTMEM_MT)

#if 0 // 7yy
/* zzz.c */
#define checkzzz(L, arg, udp) (zzz_t*)checkxxx((L), (arg), (udp), ZZZ_MT)
#define testzzz(L, arg, udp) (zzz_t*)testxxx((L), (arg), (udp), ZZZ_MT)
#define optzzz(L, arg, udp) (zzz_t*)optxxx((L), (arg), (udp), ZZZ_MT)
#define pushzzz(L, handle) pushxxx((L), (void*)(handle))
#define checkzzzlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), ZZZ_MT)

#endif

#define RAW_FUNC(xxx)                       \
static int Raw(lua_State *L)                \
    {                                       \
    lua_pushinteger(L, (uintptr_t)check##xxx(L, 1, NULL));  \
    return 1;                               \
    }

#define TYPE_FUNC(xxx) /* */                \
static int Type(lua_State *L)               \
    {                                       \
    (void)check##xxx(L, 1, NULL);           \
    lua_pushstring(L, ""#xxx);              \
    return 1;                               \
    }

#define DESTROY_FUNC(xxx)                   \
static int Destroy(lua_State *L)            \
    {                                       \
    ud_t *ud;                               \
    (void)test##xxx(L, 1, &ud);             \
    if(!ud) return 0; /* already deleted */ \
    return ud->destructor(L, ud);           \
    }

#define PARENT_FUNC(xxx)                    \
static int Parent(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->parent_ud) return 0;            \
    return pushuserdata(L, ud->parent_ud);  \
    }

#endif /* objectsDEFINED */
