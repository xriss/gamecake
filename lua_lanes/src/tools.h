/*
* TOOLS.H
*/
#ifndef TOOLS_H
#define TOOLS_H

#include "lua.h"
#include "threading.h"
    // LOCK_T

#include <assert.h>

// Note: The < -10000 test is to leave registry/global/upvalue indices untouched
//
#define /*int*/ STACK_ABS(L,n) \
	( ((n) >= 0 || (n) <= -10000) ? (n) : lua_gettop(L) +(n) +1 )

#ifdef NDEBUG
  #define _ASSERT_L(lua,c)  /*nothing*/
  #define STACK_CHECK(L)    /*nothing*/
  #define STACK_MID(c)      /*nothing*/
  #define STACK_END(c)      /*nothing*/
  #define STACK_DUMP(L)    /*nothing*/
  #define DEBUG()   /*nothing*/
#else
  #define _ASSERT_L(lua,c)  { if (!(c)) luaL_error( lua, "ASSERT failed: %s:%d '%s'", __FILE__, __LINE__, #c ); }
  //
  #define STACK_CHECK(L)     { lua_State* _L_= (L); int _oldtop_= lua_gettop(_L_);
  #define STACK_MID(change)  { int a= lua_gettop(_L_)-_oldtop_; int b= (change); \
                               if (a != b) luaL_error( _L_, "STACK ASSERT failed (%d not %d): %s:%d", a, b, __FILE__, __LINE__ ); }
  #define STACK_END(change)  STACK_MID(change) }

  #define STACK_DUMP(L)    luaG_dump(L);
  #define DEBUG()   fprintf( stderr, "<<%s %d>>\n", __FILE__, __LINE__ );
#endif
#define ASSERT_L(c) _ASSERT_L(L,c)

#define STACK_GROW(L,n) { if (!lua_checkstack(L,n)) luaL_error( L, "Cannot grow stack!" ); }

#define LUAG_FUNC( func_name ) static int LG_##func_name( lua_State *L )

#define luaG_optunsigned(L,i,d) ((uint_t) luaL_optinteger(L,i,d))
#define luaG_tounsigned(L,i) ((uint_t) lua_tointeger(L,i))

#define luaG_isany(L,i)  (!lua_isnil(L,i))

#define luaG_typename( L, index ) lua_typename( L, lua_type(L,index) )

void luaG_dump( lua_State* L );

const char *luaG_openlibs( lua_State *L, const char *libs );

int luaG_deep_userdata( lua_State *L );
void *luaG_todeep( lua_State *L, lua_CFunction idfunc, int index );

typedef struct {
    volatile int refcount;
    void *deep;
} DEEP_PRELUDE;

void luaG_push_proxy( lua_State *L, lua_CFunction idfunc, DEEP_PRELUDE *deep_userdata );

void luaG_inter_copy( lua_State *L, lua_State *L2, uint_t n );
void luaG_inter_move( lua_State *L, lua_State *L2, uint_t n );

// Lock for deep userdata reference counter inc/dec (to be given, and
// initialized by outside code)
//
extern LOCK_T deep_lock;

#if (defined PLATFORM_WIN32) || (defined PLATFORM_POCKETPC)
# define LUAG_EXPORT __declspec(dllexport)
#else
# define LUAG_EXPORT /*nothing*/
#endif

#endif
    // TOOLS_H
