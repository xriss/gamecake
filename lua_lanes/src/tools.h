/*
* TOOLS.H
*/
#ifndef TOOLS_H
#define TOOLS_H

#include "lua.h"
#include "threading.h"
    // MUTEX_T

#include <assert.h>

// Note: The < LUA_REGISTRYINDEX test is to leave registry/global/upvalue indices untouched
//
#define /*int*/ STACK_ABS(L,n) \
	( ((n) >= 0 || (n) <= LUA_REGISTRYINDEX) ? (n) : lua_gettop(L) +(n) +1 )

#ifdef NDEBUG
  #define _ASSERT_L(lua,c)  /*nothing*/
  #define STACK_CHECK(L)    /*nothing*/
  #define STACK_MID(L,c)    /*nothing*/
  #define STACK_END(L,c)    /*nothing*/
  #define STACK_DUMP(L)    /*nothing*/
  #define DEBUG()   /*nothing*/
  #define DEBUGEXEC(_code) {}  /*nothing*/
#else
  #define _ASSERT_L(lua,c)  do { if (!(c)) luaL_error( lua, "ASSERT failed: %s:%d '%s'", __FILE__, __LINE__, #c ); } while( 0)
  //
  #define STACK_CHECK(L)     { int _oldtop_##L = lua_gettop(L);
  #define STACK_MID(L,change)  { int a= lua_gettop(L)-_oldtop_##L; int b= (change); \
                               if (a != b) luaL_error( L, "STACK ASSERT failed (%d not %d): %s:%d", a, b, __FILE__, __LINE__ ); }
  #define STACK_END(L,change)  STACK_MID(L,change) }

  #define STACK_DUMP(L)    luaG_dump(L);
  #define DEBUG()   fprintf( stderr, "<<%s %d>>\n", __FILE__, __LINE__ );
  #define DEBUGEXEC(_code) {_code;}  /*nothing*/
#endif
#define ASSERT_L(c) _ASSERT_L(L,c)

#define STACK_GROW(L,n) do { if (!lua_checkstack(L,n)) luaL_error( L, "Cannot grow stack!" ); } while( 0)

#define LUAG_FUNC( func_name ) static int LG_##func_name( lua_State *L )

#define luaG_optunsigned(L,i,d) ((uint_t) luaL_optinteger(L,i,d))
#define luaG_tounsigned(L,i) ((uint_t) lua_tointeger(L,i))

#define luaG_isany(L,i)  (!lua_isnil(L,i))

#define luaG_typename( L, index ) lua_typename( L, lua_type(L,index) )

typedef void (*luaG_IdFunction)( lua_State *L, char const * const which);

void luaG_dump( lua_State* L );

const char *luaG_openlibs( lua_State *L, const char *libs );

int luaG_deep_userdata( lua_State *L, luaG_IdFunction idfunc);
void *luaG_todeep( lua_State *L, luaG_IdFunction idfunc, int index );

typedef struct {
    volatile int refcount;
    void *deep;
} DEEP_PRELUDE;

void luaG_push_proxy( lua_State *L, luaG_IdFunction idfunc, DEEP_PRELUDE *deep_userdata );

int luaG_inter_copy( lua_State *L, lua_State *L2, uint_t n);
int luaG_inter_move( lua_State *L, lua_State *L2, uint_t n);

// Lock for reference counter inc/dec locks (to be initialized by outside code)
//
extern MUTEX_T deep_lock;
extern MUTEX_T mtid_lock;

void populate_func_lookup_table( lua_State *L, int _i, char const *_name);
void serialize_require( lua_State *L);
extern MUTEX_T require_cs;

#endif
    // TOOLS_H
