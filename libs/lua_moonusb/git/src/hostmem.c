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

#if defined(LINUX)
#define AlignedAlloc aligned_alloc
#define AlignedFree  free
#elif defined(MINGW)
#define AlignedAlloc _aligned_malloc
#define AlignedFree  _aligned_free
#else
#error "Cannot determine platform"
#endif

static unsigned char *AllocMem(lua_State *L, devhandle_t *devhandle, size_t alignment, size_t size, int *dma)
    {
    unsigned char *ptr = NULL;
    *dma = 0;
    if(devhandle)
        { ptr = libusb_dev_mem_alloc(devhandle, size); *dma = 1; }
    if(!ptr) ptr = (unsigned char*)AlignedAlloc(alignment, size);
    if(!ptr) luaL_error(L, "failed to allocate memory");
    return ptr;
    }

static void FreeMem(devhandle_t *devhandle, unsigned char *ptr, size_t size)
    {
    if(devhandle) libusb_dev_mem_free(devhandle, ptr, size);
    else AlignedFree(ptr);
    }

static int freehostmem(lua_State *L, ud_t *ud)
    {
    devhandle_t *devhandle;
    hostmem_t* hostmem = (hostmem_t*)ud->handle;
    ud_t *parent_ud = ud->parent_ud;
    int dma = IsDma(ud);
    int allocated = IsAllocated(ud);
    if(!freeuserdata(L, ud, "hostmem")) return 0;
    if(allocated)
        {
        devhandle = parent_ud ? (devhandle_t*)parent_ud->handle : NULL;
        FreeMem(dma ? devhandle : NULL, hostmem->ptr, hostmem->size);
        }
    Free(L, hostmem);
    return 0;
    }

static ud_t *newhostmem(lua_State *L, hostmem_t* hostmem, devhandle_t *devhandle)
    {
    ud_t *ud;
    ud = newuserdata(L, hostmem, HOSTMEM_MT, "hostmem");
    ud->destructor = freehostmem;
    if(devhandle)
        {
        ud->parent_ud = userdata(devhandle);
        ud->context = userdata(devhandle)->context;
        }
    return ud;
    }

static int CreateAllocated(lua_State *L, unsigned char *ptr, size_t size, devhandle_t *devhandle, int dma)
    {
    ud_t *ud;
    hostmem_t* hostmem;
    hostmem = (hostmem_t*)MallocNoErr(L, sizeof(hostmem_t));
    if(!hostmem)
        {
        free(ptr);
        return luaL_error(L, errstring(ERR_MEMORY));
        }
    hostmem->ptr = ptr;
    hostmem->size = size;
    ud = newhostmem(L, hostmem, devhandle);
    MarkAllocated(ud);
    if(dma) MarkDma(ud);
    return 1;
    }

static int CreatePack(lua_State *L, int arg, size_t alignment, devhandle_t *devhandle)
    {
    int err, dma;
    unsigned char *ptr;
    int type = checktype(L, arg);
    size_t n = toflattable(L, arg+1);
    size_t size = n * sizeoftype(type);
    if(size == 0) 
        return luaL_argerror(L, arg+1, errstring(ERR_LENGTH));
    ptr = AllocMem(L, devhandle, alignment, size, &dma);
    err = testdata(L, type, n, ptr, size);
    if(err)
        {
        FreeMem(dma ? devhandle : NULL, ptr, size);
        return luaL_argerror(L, arg+1, errstring(err));
        }
    CreateAllocated(L, ptr, size, devhandle, dma);
    return 1;
    }

static int Create(lua_State *L, int arg, size_t alignment, devhandle_t *devhandle)
    {
    int dma;
    const char *data = NULL;
    unsigned char *ptr;
    size_t size;
    if(lua_type(L, arg) == LUA_TSTRING)
        {
        if(!lua_isnoneornil(L, arg+1))
            return CreatePack(L, arg, alignment, devhandle);
        data = luaL_checklstring(L, arg, &size);
        if(size == 0) 
            return luaL_argerror(L, arg, errstring(ERR_LENGTH));
        }
    else
        {
        size = luaL_checkinteger(L, arg);
        if(size == 0) 
            return luaL_argerror(L, arg, errstring(ERR_VALUE));
        }
    ptr = AllocMem(L, devhandle, alignment, size, &dma);
    if(data)
        memcpy(ptr, data, size);
    else
        memset(ptr, 0, size);
    CreateAllocated(L, ptr, size, devhandle, dma);
    return 1;
    }

static int CreateAlignedAlloc(lua_State *L)
    {
    size_t alignment = luaL_checkinteger(L, 1);
    return Create(L, 2, alignment, NULL);
    }

static int CreateMalloc(lua_State *L)
    {
    devhandle_t *devhandle = optdevhandle(L, 1, NULL);
    return Create(L, 2, 8, devhandle);
    }

static int CreateHostmem(lua_State *L)
    {
    size_t size;
    const char *ptr;
    hostmem_t* hostmem;
    if(lua_type(L, 1) == LUA_TSTRING)
        ptr = lua_tolstring(L, 1, &size);
    else
        {
        size = luaL_checkinteger(L, 1);
        ptr = (char*)checklightuserdata(L, 2);
        }
    if(size == 0)
        return luaL_argerror(L, 1, errstring(ERR_LENGTH));
    hostmem = (hostmem_t*)MallocNoErr(L, sizeof(hostmem_t));
    if(!hostmem)
        {
        Free(L, hostmem);
        return luaL_error(L, errstring(ERR_MEMORY));
        }
    hostmem->ptr = (unsigned char*)ptr;
    hostmem->size = size;
    newhostmem(L, hostmem, NULL);
    return 1;
    }

static int WriteData(lua_State *L)
    {
    size_t size;
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_checkinteger(L, 2);
    /* arg 3 should be nil */
    const char *data = luaL_checklstring(L, 4, &size);
    if(size == 0)
        return 0;
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    memcpy(hostmem->ptr + offset, data, size);
    return 0;
    }

static int CopyPtr(lua_State *L)
    {
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);
    void *ptr = checklightuserdata(L, 4);
    if(size == 0)
        return 0;
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    memcpy(hostmem->ptr + offset, ptr, size);
    return 0;
    }

static int CopyHostmem(lua_State *L)
    {
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);
    hostmem_t* srchostmem = checkhostmem(L, 4, NULL);
    size_t srcoffset = luaL_checkinteger(L, 5);
    if(hostmem == srchostmem)
        return luaL_argerror(L, 4, "source and destination hostmem are the same");
    if(size == 0)
        return 0;
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    if((srcoffset >= srchostmem->size) || (size > srchostmem->size - srcoffset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    memcpy(hostmem->ptr + offset, srchostmem->ptr + srcoffset, size);
    return 0;
    }

static int WritePack(lua_State *L) 
    {
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_checkinteger(L, 2);
    int type = checktype(L, 3);
    size_t size = hostmem->size - offset;
    if(offset >= hostmem->size)
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    checkdata(L, 4, type, hostmem->ptr + offset, size);
    return 0;
    }

static int Write(lua_State *L)
    {
    int t = lua_type(L, 3);
    if(t == LUA_TSTRING) return WritePack(L);
    if(t == LUA_TNIL) return WriteData(L);
    return luaL_argerror(L, 3, errstring(ERR_TYPE));    
    }

static int Copy(lua_State *L)
    {
    int t = lua_type(L, 4);
    if(t == LUA_TLIGHTUSERDATA) return CopyPtr(L);
    if(t == LUA_TUSERDATA) return CopyHostmem(L);
    return luaL_argerror(L, 4, errstring(ERR_TYPE));    
    }
        
static int Clear(lua_State *L)
/* clear(offset, size, c) 
 */
    {
    size_t len;
    const char *s;
    char c;
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);
    if(lua_type(L, 4) == LUA_TSTRING)
        {
        s = luaL_checklstring(L, 4, &len);
        if(len != 1)
            return luaL_argerror(L, 4, "invalid length"); /* must be 1 */
        c = s[0];
        }
    else
        c = luaL_optinteger(L, 4, 0);
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    if(size == 0)
        return 0;
    memset(hostmem->ptr + offset, c, size);
    return 0;
    }

static int Read(lua_State *L)
    {
    int type;
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_optinteger(L, 2, 0);
    size_t size = luaL_optinteger(L, 3, hostmem->size - offset);
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    if(lua_type(L, 4) == LUA_TSTRING) /* unpack according to 'type' */
        {
        type = checktype(L, 4);
        if(size == 0)
            { lua_newtable(L); return 1; }
        return pushdata(L, type, hostmem->ptr + offset, size);
        }
    if(size == 0)
        lua_pushstring(L, ""); 
    else
        lua_pushlstring(L, (char*)(hostmem->ptr + offset), size);
    return 1;
    }

static int Ptr(lua_State *L)
    {
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    size_t offset = luaL_optinteger(L, 2, 0);
    size_t size = luaL_optinteger(L, 3, 0);
    if((offset >= hostmem->size) || (size > hostmem->size - offset))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    lua_pushlightuserdata(L, hostmem->ptr + offset);
    return 1;
    }

static int Size(lua_State *L)
    {
    size_t offset;
    unsigned char *ptr;
    hostmem_t* hostmem = checkhostmem(L, 1, NULL);
    if(lua_isnoneornil(L, 2))
        lua_pushinteger(L, hostmem->size);
    else if(lua_isinteger(L, 2))
        {
        offset = luaL_checkinteger(L, 2);
        if(offset >= hostmem->size)
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, hostmem->size - offset);
        }
    else
        {
        ptr = (unsigned char*)checklightuserdata(L, 2);
        if((ptr < hostmem->ptr) || (ptr > (hostmem->ptr + hostmem->size)))
            lua_pushinteger(L, 0);
        else
            lua_pushinteger(L, hostmem->ptr + hostmem->size - ptr);
        }
    return 1;
    }

DESTROY_FUNC(hostmem)

static const struct luaL_Reg Methods[] = 
    {
        { "free", Destroy },
        { "write", Write },
        { "copy", Copy },
        { "clear", Clear },
        { "read", Read },
        { "ptr", Ptr },
        { "size", Size },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "malloc", CreateMalloc },
        { "aligned_alloc", CreateAlignedAlloc },
        { "hostmem", CreateHostmem },
        { "free",  Destroy },
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_hostmem(lua_State *L)
    {
    udata_define(L, HOSTMEM_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

