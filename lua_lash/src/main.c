/*
 * Copyright (c) 2007,2008 Neil Richardson (nrich@iinet.net.au)
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */

// removed Rijndael suport
// and removed the file based commands as well


#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if !defined(LUA_VERSION_NUM) || (LUA_VERSION_NUM < 501)
#include <compat-5.1.h>
#endif

#ifdef _WIN32
    #define LH_EXPORT __declspec(dllexport)
#else
    #define LH_EXPORT
#endif

//int MD5File(lua_State *L);
int MD5String(lua_State *L);

int CRC32Int(lua_State *L);
int CRC32String(lua_State *L);
//int CRC32FileInt(lua_State *L);
//int CRC32FileString(lua_State *L);

//int SHA1File(lua_State *L);
int SHA1String(lua_State *L);

/*
int RijndaelEncrypt(lua_State *L);
int RijndaelDecrypt(lua_State *L);
*/

static const luaL_Reg lash_md5[] = {
//    {"file2hex", MD5File},
    {"string2hex", MD5String},
    {NULL, NULL}
};

static const luaL_Reg lash_crc32[] = {
    {"string2num", CRC32Int},
    {"string2hex", CRC32String},
//    {"file2num", CRC32FileInt},
//    {"file2hex", CRC32FileString},
    {NULL, NULL}
};

static const luaL_Reg lash_sha1[] = {
//    {"file2hex", SHA1File},
    {"string2hex", SHA1String},
    {NULL, NULL}
};

/*
static const luaL_Reg lash_rijndael[] = {
    {"encrypt", RijndaelEncrypt},
    {"decrypt", RijndaelDecrypt},
    {NULL, NULL}
};
*/

LH_EXPORT int luaopen_lash(lua_State *L) {
    luaL_register(L, "lash.MD5", lash_md5);
    luaL_register(L, "lash.CRC32", lash_crc32);
    luaL_register(L, "lash.SHA1", lash_sha1);
//    luaL_register(L, "lash.Rijndael", lash_rijndael);

    /*
     * push the created table to the top off the stack
     * so "lash = require('lash')" works
     */
    lua_getglobal(L, "lash");

    return 1;
}
