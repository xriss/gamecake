/*
 * TOOLS.C   	                    Copyright (c) 2002-10, Asko Kauppi
 *
 * Lua tools to support Lanes.
*/

/*
===============================================================================

Copyright (C) 2002-10 Asko Kauppi <akauppi@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
*/

#include "tools.h"
#include "keeper.h"

#include "lualib.h"
#include "lauxlib.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

MUTEX_T deep_lock;
MUTEX_T mtid_lock;

/*---=== luaG_dump ===---*/

void luaG_dump( lua_State* L ) {

    int top= lua_gettop(L);
    int i;

	fprintf( stderr, "\n\tDEBUG STACK:\n" );

	if (top==0)
		fprintf( stderr, "\t(none)\n" );

	for( i=1; i<=top; i++ ) {
		int type= lua_type( L, i );

		fprintf( stderr, "\t[%d]= (%s) ", i, lua_typename(L,type) );

		// Print item contents here...
		//
		// Note: this requires 'tostring()' to be defined. If it is NOT,
		//       enable it for more debugging.
		//
    STACK_CHECK(L)
        STACK_GROW( L, 2);

        lua_getglobal( L, "tostring" );
            //
            // [-1]: tostring function, or nil
        
        if (!lua_isfunction(L,-1)) {
             fprintf( stderr, "('tostring' not available)" );
         } else {
             lua_pushvalue( L, i );
             lua_call( L, 1 /*args*/, 1 /*retvals*/ );

             // Don't trust the string contents
             //                
             fprintf( stderr, "%s", lua_tostring(L,-1) );
         }
         lua_pop(L,1);
    STACK_END(L,0)
		fprintf( stderr, "\n" );
		}
	fprintf( stderr, "\n" );
}


/*---=== luaG_openlibs ===---*/

static const luaL_Reg libs[] = {
  { LUA_LOADLIBNAME, luaopen_package },
  { LUA_TABLIBNAME, luaopen_table },
  { LUA_IOLIBNAME, luaopen_io },
  { LUA_OSLIBNAME, luaopen_os },
  { LUA_STRLIBNAME, luaopen_string },
  { LUA_MATHLIBNAME, luaopen_math },
  { LUA_DBLIBNAME, luaopen_debug },
  //
  { "base", NULL },         // ignore "base" (already acquired it)
  { "coroutine", NULL },    // part of Lua 5.1 base package
  { NULL, NULL }
};

static bool_t openlib( lua_State *L, const char *name, size_t len ) {

	unsigned i;
	bool_t all= strncmp( name, "*", len ) == 0;

	for( i=0; libs[i].name; i++ )
	{
		if (all || (strncmp(name, libs[i].name, len) ==0))
		{
			if (libs[i].func)
			{
				STACK_GROW(L,1);
				STACK_CHECK(L)
				lua_pushcfunction( L, libs[i].func);
				// pushes the module table on the stack
				lua_call( L, 0, 1);
				populate_func_lookup_table( L, -1, libs[i].name);
				// remove the module when we are done
				lua_pop( L, 1);
				STACK_END(L, 0)
			}
			if (!all) return TRUE;
		}
	}
	return all;
}

static int dummy_writer(lua_State *L, const void* p, size_t sz, void* ud)
{
	(void)L; (void)p; (void)sz; (void) ud; // unused
	return 666;
}


/*
 * differentiation between C, bytecode and JIT-fast functions
 *
 *
 *                   +----------+------------+----------+
 *                   | bytecode | C function | JIT-fast |
 * +-----------------+----------+------------+----------+
 * | lua_topointer   |          |            |          |
 * +-----------------+----------+------------+----------+
 * | lua_tocfunction |  NULL    |            |  NULL    |
 * +-----------------+----------+------------+----------+
 * | lua_dump        |  666     |  1         |  1       |
 * +-----------------+----------+------------+----------+
 */

typedef enum
{
	FST_Bytecode,
	FST_Native,
	FST_FastJIT
} FuncSubType;

FuncSubType luaG_getfuncsubtype( lua_State *L, int _i)
{
	if( lua_tocfunction( L, _i))
	{
		return FST_Native;
	}
	{
		int mustpush = 0, dumpres;
		if( STACK_ABS( L, _i) != lua_gettop( L))
		{
			lua_pushvalue( L, _i);
			mustpush = 1;
		}
		// the provided writer fails with code 666
		// therefore, anytime we get 666, this means that lua_dump() attempted a dump
		// all other cases mean this is either a C or LuaJIT-fast function
		dumpres = lua_dump( L, dummy_writer, NULL);
		lua_pop( L, mustpush);
		if( dumpres == 666)
		{
			return FST_Bytecode;
		}
	}
	return FST_FastJIT;
}

static lua_CFunction luaG_tocfunction( lua_State *L, int _i, FuncSubType *_out)
{
	lua_CFunction p = lua_tocfunction( L, _i);
	*_out = luaG_getfuncsubtype( L, _i);
	return p;
}


#define LOOKUP_KEY "ddea37aa-50c7-4d3f-8e0b-fb7a9d62bac5"
#define LOOKUP_KEY_CACHE "d1059270-4976-4193-a55b-c952db5ab7cd"


// inspired from tconcat() in ltablib.c
static char const * luaG_pushFQN(lua_State *L, int t, int last)
{
	int i = 1;
	luaL_Buffer b;
	STACK_CHECK( L)
	luaL_buffinit(L, &b);
	for( ; i < last; i++)
	{
		lua_rawgeti( L, t, i);
		luaL_addvalue( &b);
		luaL_addlstring(&b, ".", 1);
	}
	if (i == last)  /* add last value (if interval was not empty) */
	{
		lua_rawgeti( L, t, i);
		luaL_addvalue( &b);
	}
	luaL_pushresult( &b);
	STACK_END( L, 1)
	return lua_tostring( L, -1);
}


static void populate_func_lookup_table_recur( lua_State *L, int _ctx_base, int _i, int _depth)
{
	lua_Integer visit_count;
	// slot 1 in the stack contains the table that receives everything we found
	int const dest = _ctx_base;
	// slot 2 contains a table that, when concatenated, produces the fully qualified name of scanned elements in the table provided at slot _i
	int const fqn = _ctx_base + 1;
	// slot 3 contains a cache that stores all already visited tables to avoid infinite recursion loops
	int const cache = _ctx_base + 2;
	// we need to remember subtables to process them after functions encountered at the current depth (breadth-first search)
	int const breadth_first_cache = lua_gettop( L) + 1;

	STACK_GROW( L, 6);
	// slot _i contains a table where we search for functions
	STACK_CHECK( L)                                                                           // ... {_i}

	// if table is already visited, we are done
	lua_pushvalue( L, _i);                                                                    // ... {_i} {}
	lua_rawget( L, cache);                                                                    // ... {_i} nil|n
	visit_count = lua_tointeger( L, -1); // 0 if nil, else n
	lua_pop( L, 1);                                                                           // ... {_i}
	STACK_MID( L, 0)
	if( visit_count > 0)
	{
		return;
	}

	// remember we visited this table (1-visit count)
	lua_pushvalue( L, _i);                                                                    // ... {_i} {}
	lua_pushinteger( L, visit_count + 1);                                                     // ... {_i} {} 1
	lua_rawset( L, cache);                                                                    // ... {_i}
	STACK_MID( L, 0)

	// this table is at breadth_first_cache index
	lua_newtable( L);                                                                         // ... {_i} {bfc}
	ASSERT_L( lua_gettop( L) == breadth_first_cache);
	// iterate over all entries in the processed table
	lua_pushnil( L);                                                                          // ... {_i} {bfc} nil
	while( lua_next( L, _i) != 0)                                                             // ... {_i} {bfc} k v
	{
		// just for debug, not actually needed
		//char const * key = (lua_type( L, -2) == LUA_TSTRING) ? lua_tostring( L, -2) : "not a string";
		// subtable: process it recursively
		if( lua_istable( L, -1))                                                                // ... {_i} {bfc} k {}
		{
			// increment visit count to make sure we will actually scan it at this recursive level
			lua_pushvalue( L, -1);                                                                // ... {_i} {bfc} k {} {}
			lua_pushvalue( L, -1);                                                                // ... {_i} {bfc} k {} {} {}
			lua_rawget( L, cache);                                                                // ... {_i} {bfc} k {} {} n?
			visit_count = lua_tointeger( L, -1) + 1; // 1 if we got nil, else n+1
			lua_pop( L, 1);                                                                       // ... {_i} {bfc} k {} {}
			lua_pushinteger( L, visit_count);                                                     // ... {_i} {bfc} k {} {} n
			lua_rawset( L, cache);                                                                // ... {_i} {bfc} k {}
			// store the table in the breadth-first cache
			lua_pushvalue( L, -2);                                                                // ... {_i} {bfc} k {} k
			lua_insert( L, -2);                                                                   // ... {_i} {bfc} k k {}
			lua_rawset( L, breadth_first_cache);                                                  // ... {_i} {bfc} k
			STACK_MID( L, 2)
		}
		else if( lua_isfunction( L, -1))                                                        // ... {_i} {bfc} k func
		{
			if( luaG_getfuncsubtype( L, -1) != FST_Bytecode)
			{
				//char const *fqnString; for debugging
				bool_t not_registered;
				// first, skip everything if the function is already known
				lua_pushvalue( L, -1);                                                              // ... {_i} {bfc} k func func
				lua_rawget( L, dest);                                                               // ... {_i} {bfc} k func name?
				not_registered = lua_isnil( L, -1);
				lua_pop( L, 1);                                                                     // ... {_i} {bfc} k func
				if( not_registered)
				{
					++ _depth;
					// push function name in fqn stack (note that concatenation will crash if name is a not string!)
					lua_pushvalue( L, -2);                                                            // ... {_i} {bfc} k func k
					lua_rawseti( L, fqn, _depth);                                                     // ... {_i} {bfc} k func
					// generate name
					/*fqnString =*/ (void) luaG_pushFQN( L, fqn, _depth);                             // ... {_i} {bfc} k func "f.q.n"
					//puts( fqnString);
					// prepare the stack for database feed
					lua_pushvalue( L, -1);                                                            // ... {_i} {bfc} k func "f.q.n" "f.q.n"
					lua_pushvalue( L, -3);                                                            // ... {_i} {bfc} k func "f.q.n" "f.q.n" func
					// t["f.q.n"] = func
					lua_rawset( L, dest);                                                             // ... {_i} {bfc} k func "f.q.n"
					// t[func] = "f.q.n"
					lua_rawset( L, dest);                                                             // ... {_i} {bfc} k
					// remove table name from fqn stack
					lua_pushnil( L);                                                                  // ... {_i} {bfc} k nil
					lua_rawseti( L, fqn, _depth);                                                     // ... {_i} {bfc} k
					-- _depth;
				}
				else
				{
					lua_pop( L, 1);                                                                   // ... {_i} {bfc} k
				}
			}
			else
			{
				lua_pop( L, 1);                                                                     // ... {_i} {bfc} k
			}
		}
		else
		{
			lua_pop( L, 1);                                                                       // ... {_i} {bfc} k
		}
		STACK_MID( L, 2)
	}
	// now process the tables we encountered at that depth
	++ _depth;
	lua_pushnil( L);                                                                          // ... {_i} {bfc} nil
	while( lua_next( L, breadth_first_cache) != 0)                                            // ... {_i} {bfc} k {}
	{
		// un-visit this table in case we do need to process it
		lua_pushvalue( L, -1);                                                                  // ... {_i} {bfc} k {} {}
		lua_rawget( L, cache);                                                                  // ... {_i} {bfc} k {} n
		ASSERT_L( lua_type( L, -1) == LUA_TNUMBER);
		visit_count = lua_tointeger( L, -1) - 1;
		lua_pop( L, 1);                                                                         // ... {_i} {bfc} k {}
		lua_pushvalue( L, -1);                                                                  // ... {_i} {bfc} k {} {}
		if( visit_count > 0)
		{
			lua_pushinteger( L, visit_count);                                                     // ... {_i} {bfc} k {} {} n
		}
		else
		{
			lua_pushnil( L);                                                                      // ... {_i} {bfc} k {} {} nil
		}
		lua_rawset( L, cache);                                                                  // ... {_i} {bfc} k {}
		// push table name in fqn stack (note that concatenation will crash if name is a not string!)
		lua_pushvalue( L, -2);                                                                  // ... {_i} {bfc} k {} k
		lua_rawseti( L, fqn, _depth);                                                           // ... {_i} {bfc} k {}
		populate_func_lookup_table_recur( L, _ctx_base, lua_gettop( L), _depth);                // ... {_i} {bfc} k {}
		lua_pop( L, 1);                                                                         // ... {_i} {bfc} k
		STACK_MID( L, 2)
	}
	// remove table name from fqn stack
	lua_pushnil( L);                                                                          // ... {_i} {bfc} nil
	lua_rawseti( L, fqn, _depth);                                                             // ... {_i} {bfc}
	-- _depth;
	// we are done with our cache
	lua_pop( L, 1);                                                                           // ... {_i}
	STACK_END( L, 0)
	// we are done                                                                            // ... {_i} {bfc}
}

/*
 * create a "fully.qualified.name" <-> function equivalence dabase
 */
void populate_func_lookup_table( lua_State *L, int _i, char const *_name)
{
	int const ctx_base = lua_gettop( L) + 1;
	int const in_base = STACK_ABS( L, _i);
	int const start_depth = _name ? 1 : 0;
	//printf( "%p: populate_func_lookup_table('%s')\n", L, _name ? _name : "NULL");
	STACK_GROW( L, 3);
	STACK_CHECK( L)
	lua_getfield( L, LUA_REGISTRYINDEX, LOOKUP_KEY);                          // {}?
	if( lua_isnil( L, -1))                                                    // nil
	{
		lua_pop( L, 1);                                                         //
		lua_newtable( L);                                                       // {}
		lua_pushvalue( L, -1);                                                  // {} {}
		lua_setfield( L, LUA_REGISTRYINDEX, LOOKUP_KEY);                        // {}
	}
	lua_newtable( L);                                                         // {} {fqn}
	if( _name)
	{
		lua_pushstring( L, _name);                                              // {} {fqn} "name"
		lua_rawseti( L, -2, start_depth);                                       // {} {fqn}
	}
	lua_getfield( L, LUA_REGISTRYINDEX, LOOKUP_KEY_CACHE);                    // {} {fqn} {cache}?
	if( lua_isnil( L, -1))
	{
		lua_pop( L, 1);                                                         // {} {fqn}
		lua_newtable( L);                                                       // {} {fqn} {cache}
		lua_pushvalue( L, -1);                                                  // {} {fqn} {cache} {cache}
		lua_setfield( L, LUA_REGISTRYINDEX, LOOKUP_KEY_CACHE);                  // {} {fqn} {cache}
	}
	populate_func_lookup_table_recur( L, ctx_base, in_base, start_depth);     // {...} {fqn} {cache}
	lua_pop( L, 3);
	STACK_END( L, 0)
}

/* 
* Like 'luaL_openlibs()' but allows the set of libraries be selected
*
*   NULL    no libraries, not even base
*   ""      base library only
*   "io,string"     named libraries
*   "*"     all libraries
*
* Base ("unpack", "print" etc.) is always added, unless 'libs' is NULL.
*
* Returns NULL for ok, position of error within 'libs' on failure.
*/
#define is_name_char(c) (isalpha(c) || (c)=='*')

const char *luaG_openlibs( lua_State *L, const char *libs)
{
	const char *p;
	unsigned len;

	if (!libs) return NULL;     // no libs, not even 'base'

	// 'lua.c' stops GC during initialization so perhaps its a good idea. :)
	//
	lua_gc( L, LUA_GCSTOP, 0);

	// Anything causes 'base' to be taken in
	//
	STACK_GROW(L,2);
	STACK_CHECK(L)
	lua_pushcfunction( L, luaopen_base);
	lua_call( L, 0, 1);
	// after opening base, register the functions they exported in our name<->function database
	populate_func_lookup_table( L, LUA_GLOBALSINDEX, NULL);
	lua_pop( L, 1);
	STACK_MID( L, 0);
	for( p= libs; *p; p+=len )
	{
		len=0;
		while (*p && !is_name_char(*p)) p++;    // bypass delimiters
		while (is_name_char(p[len])) len++;     // bypass name
		if (len && (!openlib( L, p, len )))
			break;
	}
	STACK_END(L,0)
	lua_gc(L, LUA_GCRESTART, 0);

	return *p ? p : NULL;
}



/*---=== Deep userdata ===---*/

/* The deep portion must be allocated separately of any Lua state's; it's
* lifespan may be longer than that of the creating state.
*/
#define DEEP_MALLOC malloc
#define DEEP_FREE   free

/* 
* 'registry[REGKEY]' is a two-way lookup table for 'idfunc's and those type's
* metatables:
*
*   metatable   ->  idfunc
*   idfunc      ->  metatable
*/
#define DEEP_LOOKUP_KEY ((void*)set_deep_lookup)
    // any unique light userdata


/*
* The deep proxy cache is a weak valued table listing all deep UD proxies indexed by the deep UD that they are proxying
*/
#define DEEP_PROXY_CACHE_KEY ((void*)luaG_push_proxy)

static void push_registry_subtable_mode( lua_State *L, void *token, const char* mode );
static void push_registry_subtable( lua_State *L, void *token );

/*
* Sets up [-1]<->[-2] two-way lookups, and ensures the lookup table exists.
* Pops the both values off the stack.
*/
void set_deep_lookup( lua_State *L ) {

    STACK_GROW(L,3);

  STACK_CHECK(L)
#if 1
    push_registry_subtable( L, DEEP_LOOKUP_KEY );
#else
    /* ..to be removed.. */
    lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
    lua_rawget( L, LUA_REGISTRYINDEX );

    if (lua_isnil(L,-1)) {
        // First time here; let's make the lookup
        //
        lua_pop(L,1);

        lua_newtable(L);
        lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
        lua_pushvalue(L,-2);
            //
            // [-3]: {} (2nd ref)
            // [-2]: DEEP_LOOKUP_KEY
            // [-1]: {}

        lua_rawset( L, LUA_REGISTRYINDEX );
            //
            // [-1]: lookup table (empty)
    }
#endif
  STACK_MID(L,1)

    lua_insert(L,-3);

    // [-3]: lookup table
    // [-2]: A
    // [-1]: B
    
    lua_pushvalue( L,-1 );  // B
    lua_pushvalue( L,-3 );  // A
    lua_rawset( L, -5 );    // B->A
    lua_rawset( L, -3 );    // A->B
    lua_pop( L,1 );

  STACK_END(L,-2)
}

/*
* Pops the key (metatable or idfunc) off the stack, and replaces with the
* deep lookup value (idfunc/metatable/nil).
*/
void get_deep_lookup( lua_State *L ) {
    
    STACK_GROW(L,1);

  STACK_CHECK(L)    
    lua_pushlightuserdata( L, DEEP_LOOKUP_KEY );
    lua_rawget( L, LUA_REGISTRYINDEX );
    
    if (!lua_isnil(L,-1)) {
        // [-2]: key (metatable or idfunc)
        // [-1]: lookup table
    
        lua_insert( L, -2 );
        lua_rawget( L, -2 );
    
        // [-2]: lookup table
        // [-1]: value (metatable / idfunc / nil)
    }    
    lua_remove(L,-2);
        // remove lookup, or unused key
  STACK_END(L,0)
}

/*
* Return the registered ID function for 'index' (deep userdata proxy),
* or NULL if 'index' is not a deep userdata proxy.
*/
static
luaG_IdFunction get_idfunc( lua_State *L, int index )
{
    luaG_IdFunction ret;

    index = STACK_ABS( L, index);

    STACK_GROW(L,1);

    STACK_CHECK(L)
    if (!lua_getmetatable( L, index ))
        return NULL;    // no metatable
    
    // [-1]: metatable of [index]

    get_deep_lookup(L);
    //    
    // [-1]: idfunc/nil

    ret= (luaG_IdFunction)lua_touserdata(L,-1);
    lua_pop(L,1);
    STACK_END(L,0)
    return ret;
}


/*
* void= mt.__gc( proxy_ud )
*
* End of life for a proxy object; reduce the deep reference count and clean
* it up if reaches 0.
*/
static
int deep_userdata_gc( lua_State *L )
{
    DEEP_PRELUDE **proxy= (DEEP_PRELUDE**)lua_touserdata( L, 1 );
    DEEP_PRELUDE *p= *proxy;
    int v;

    *proxy= 0;  // make sure we don't use it any more

    MUTEX_LOCK( &deep_lock );
    v= --(p->refcount);
    MUTEX_UNLOCK( &deep_lock );

    if (v==0)
    {
        // Call 'idfunc( "delete", deep_ptr )' to make deep cleanup
        //
        luaG_IdFunction idfunc = get_idfunc(L,1);
        ASSERT_L(idfunc);
        
        lua_settop( L, 0);    // clean stack so we can call 'idfunc' directly

        // void= idfunc( "delete", lightuserdata )
        //
        lua_pushlightuserdata( L, p->deep );
        idfunc( L, "delete");

        // top was set to 0, then userdata was pushed. "delete" might want to pop the userdata (we don't care), but should not push anything!
        if ( lua_gettop( L) > 1)
            luaL_error( L, "Bad idfunc on \"delete\": returned something");

        DEEP_FREE( (void*)p );
    }
    return 0;
}


/*
* Push a proxy userdata on the stack.
*
* Initializes necessary structures if it's the first time 'idfunc' is being
* used in this Lua state (metatable, registring it). Otherwise, increments the
* reference count.
*/
void luaG_push_proxy( lua_State *L, luaG_IdFunction idfunc, DEEP_PRELUDE *prelude )
{
    DEEP_PRELUDE **proxy;

    // Check if a proxy already exists
    push_registry_subtable_mode(L, DEEP_PROXY_CACHE_KEY, "v");
    lua_pushlightuserdata(L, prelude->deep);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1))
    {
        lua_remove(L, -2); // deep proxy cache table
        return;
    }
    else
    {
        lua_pop(L, 2); // Pop the nil and proxy cache table
    }

    MUTEX_LOCK( &deep_lock );
    ++(prelude->refcount);  // one more proxy pointing to this deep data
    MUTEX_UNLOCK( &deep_lock );

    STACK_GROW(L,4);

    STACK_CHECK(L)

    proxy= lua_newuserdata( L, sizeof( DEEP_PRELUDE* ) );
    ASSERT_L(proxy);
    *proxy= prelude;

    // Get/create metatable for 'idfunc' (in this state)
    //
    lua_pushlightuserdata( L, idfunc );    // key
    get_deep_lookup(L);
        //
        // [-2]: proxy
        // [-1]: metatable / nil
    
    if (lua_isnil(L,-1))
    {
        // No metatable yet. We have two things to do:
        // 1 - make one and register it
        {
            int oldtop;

            lua_pop( L, 1);

            // tbl= idfunc( "metatable" )
            //
            oldtop = lua_gettop( L);
            idfunc( L, "metatable");
            //
            // [-2]: proxy
            // [-1]: metatable (returned by 'idfunc')

            if (lua_gettop( L) - oldtop != 1 || !lua_istable(L, -1))
            {
                luaL_error( L, "Bad idfunc on \"metatable\": did not return one" );
            }

            // Add '__gc' method
            //
            lua_pushcfunction( L, deep_userdata_gc );
            lua_setfield( L, -2, "__gc" );

            // Memorize for later rounds
            //
            lua_pushvalue( L,-1 );
            lua_pushlightuserdata( L, idfunc );
            //
            // [-4]: proxy
            // [-3]: metatable (2nd ref)
            // [-2]: metatable
            // [-1]: idfunc

            set_deep_lookup(L);
        }

        // 2 - cause the target state to require the module that exported the idfunc
        // this is needed because we must make sure the shared library is still loaded as long as we hold a pointer on the idfunc
        STACK_CHECK(L)
        {
            char const * modname;
            // make sure the function pushed a single value on the stack!
            {
                int oldtop = lua_gettop( L);
                idfunc( L, "module");                                                                        // ... "module"/nil
                if( lua_gettop( L) - oldtop != 1)
                {
                    luaL_error( L, "Bad idfunc on \"module\": should return a single value");
                }
            }
            modname = luaL_optstring( L, -1, NULL); // raises an error if not a string or nil
            if( modname) // we actually got a module name
            {
                // somehow, L.registry._LOADED can exist without having registered the 'package' library.
                lua_getglobal( L, "require");                                                                // ... "module" require()
                // check that the module is already loaded (or being loaded, we are happy either way)
                if( lua_isfunction( L, -1))
                {
                    lua_insert( L, -2);                                                                      // ... require() "module"
                    lua_getfield( L, LUA_REGISTRYINDEX, "_LOADED");                                          // ... require() "module" L.registry._LOADED
                    if( lua_istable( L, -1))
                    {
                        bool_t alreadyloaded;
                        lua_pushvalue( L, -2);                                                               // ... require() "module" L.registry._LOADED "module"
                        lua_rawget( L, -2);                                                                  // ... require() "module" L.registry._LOADED module
                        alreadyloaded = lua_toboolean( L, -1);
                        if( !alreadyloaded) // not loaded
                        {
                            lua_pop( L, 2);                                                                  // ... require() "module"
                            lua_call( L, 1, 0); // call require "modname"                                    // ...
                        }
                        else // already loaded, we are happy
                        {
                            lua_pop( L, 4);                                                                  // ...
                        }
                    }
                    else // no L.registry._LOADED; can this ever happen?
                    {
                        luaL_error( L, "unexpected error while requiring a module");
                        lua_pop( L, 3);                                                                      // ...
                    }
                }
                else // a module name, but no require() function :-(
                {
                    luaL_error( L, "lanes receiving deep userdata should register the 'package' library");
                    lua_pop( L, 2);                                                                          // ...
                }
            }
            else // no module name
            {
                lua_pop( L, 1);                                                                              // ...
            }
        }
        STACK_END(L,0)
    }
    STACK_MID(L,2)
    ASSERT_L( lua_isuserdata(L,-2) );
    ASSERT_L( lua_istable(L,-1) );

    // [-2]: proxy userdata
    // [-1]: metatable to use

    lua_setmetatable( L, -2 );
    
    // If we're here, we obviously had to create a new proxy, so cache it.
    push_registry_subtable_mode(L, DEEP_PROXY_CACHE_KEY, "v");
    lua_pushlightuserdata(L, (*proxy)->deep);
    lua_pushvalue(L, -3); // Copy of the proxy
    lua_rawset(L, -3);
    lua_pop(L, 1); // Remove the cache proxy table

    STACK_END(L,1)
    // [-1]: proxy userdata
}


/*
* Create a deep userdata
*
*   proxy_ud= deep_userdata( idfunc [, ...] )
*
* Creates a deep userdata entry of the type defined by 'idfunc'.
* Other parameters are passed on to the 'idfunc' "new" invocation.
*
* 'idfunc' must fulfill the following features:
*
*   lightuserdata= idfunc( "new" [, ...] )      -- creates a new deep data instance
*   void= idfunc( "delete", lightuserdata )     -- releases a deep data instance
*   tbl= idfunc( "metatable" )          -- gives metatable for userdata proxies
*
* Reference counting and true userdata proxying are taken care of for the
* actual data type.
*
* Types using the deep userdata system (and only those!) can be passed between
* separate Lua states via 'luaG_inter_move()'.
*
* Returns:  'proxy' userdata for accessing the deep data via 'luaG_todeep()'
*/
int luaG_deep_userdata( lua_State *L, luaG_IdFunction idfunc)
{
    int oldtop;

    DEEP_PRELUDE *prelude= DEEP_MALLOC( sizeof(DEEP_PRELUDE) );
    ASSERT_L(prelude);

    prelude->refcount= 0;   // 'luaG_push_proxy' will lift it to 1

    STACK_GROW(L,1);
    STACK_CHECK(L)

    // lightuserdata= idfunc( "new" [, ...] )
    //
		oldtop = lua_gettop( L);
    idfunc(L, "new");

    if( lua_gettop( L) - oldtop != 1 || lua_type( L, -1) != LUA_TLIGHTUSERDATA)
    {
        luaL_error( L, "Bad idfunc on \"new\": did not return light userdata");
    }

    prelude->deep= lua_touserdata(L,-1);
    ASSERT_L(prelude->deep);

    lua_pop(L,1);   // pop deep data

    luaG_push_proxy( L, idfunc, prelude );
    //
    // [-1]: proxy userdata

    STACK_END(L,1)
    return 1;
}


/*
* Access deep userdata through a proxy.
*
* Reference count is not changed, and access to the deep userdata is not
* serialized. It is the module's responsibility to prevent conflicting usage.
*/
void *luaG_todeep( lua_State *L, luaG_IdFunction idfunc, int index )
{
    DEEP_PRELUDE **proxy;

    STACK_CHECK(L)
    if( get_idfunc(L,index) != idfunc)
        return NULL;    // no metatable, or wrong kind

    proxy= (DEEP_PRELUDE**)lua_touserdata( L, index );
    STACK_END(L,0)

    return (*proxy)->deep;
}


/*
* Copy deep userdata between two separate Lua states.
*
* Returns:
*   the id function of the copied value, or NULL for non-deep userdata
*   (not copied)
*/
static
luaG_IdFunction luaG_copydeep( lua_State *L, lua_State *L2, int index )
{
    DEEP_PRELUDE **proxy;
    DEEP_PRELUDE *p;

    luaG_IdFunction idfunc = get_idfunc( L, index);
    if (!idfunc)
        return NULL;   // not a deep userdata

    // Increment reference count
    //
    proxy= (DEEP_PRELUDE**)lua_touserdata( L, index );
    p= *proxy;

    luaG_push_proxy( L2, idfunc, p );
        //
        // L2 [-1]: proxy userdata

    return idfunc;
}



/*---=== Inter-state copying ===---*/

/*-- Metatable copying --*/

/*
 * 'reg[ REG_MT_KNOWN ]'= {
 *      [ table ]= id_uint,
 *          ...
 *      [ id_uint ]= table,
 *          ...
 * }
 */

/*
* Does what the original 'push_registry_subtable' function did, but adds an optional mode argument to it
*/
static
void push_registry_subtable_mode( lua_State *L, void *token, const char* mode ) {

    STACK_GROW(L,3);

  STACK_CHECK(L)
    
    lua_pushlightuserdata( L, token );
    lua_rawget( L, LUA_REGISTRYINDEX );
        //
        // [-1]: nil/subtable
    
    if (lua_isnil(L,-1)) {
        lua_pop(L,1);
        lua_newtable(L);                    // value
        lua_pushlightuserdata( L, token );  // key
        lua_pushvalue(L,-2);
            //
            // [-3]: value (2nd ref)
            // [-2]: key
            // [-1]: value

        lua_rawset( L, LUA_REGISTRYINDEX );

        // Set it's metatable if requested
        if (mode) {
            lua_newtable(L);
            lua_pushliteral(L, "__mode");
            lua_pushstring(L, mode);
            lua_rawset(L, -3);
            lua_setmetatable(L, -2);
        }
    }
  STACK_END(L,1)

    ASSERT_L( lua_istable(L,-1) );
}

/*
* Push a registry subtable (keyed by unique 'token') onto the stack.
* If the subtable does not exist, it is created and chained.
*/
static
void push_registry_subtable( lua_State *L, void *token ) {
    push_registry_subtable_mode(L, token, NULL);
}

#define REG_MTID ( (void*) get_mt_id )

/*
* Get a unique ID for metatable at [i].
*/
static
uint_t get_mt_id( lua_State *L, int i ) {
    static uint_t last_id= 0;
    uint_t id;

    i = STACK_ABS( L, i);

    STACK_GROW(L,3);

  STACK_CHECK(L)
    push_registry_subtable( L, REG_MTID );
    lua_pushvalue(L, i);
    lua_rawget( L, -2 );
        //
        // [-2]: reg[REG_MTID]
        // [-1]: nil/uint
    
    id= (uint_t)lua_tointeger(L,-1);    // 0 for nil
    lua_pop(L,1);
  STACK_MID(L,1)
    
    if (id==0) {
        MUTEX_LOCK( &mtid_lock );
            id= ++last_id;
        MUTEX_UNLOCK( &mtid_lock );

        /* Create two-way references: id_uint <-> table
        */
        lua_pushvalue(L,i);
        lua_pushinteger(L,id);
        lua_rawset( L, -3 );
        
        lua_pushinteger(L,id);
        lua_pushvalue(L,i);
        lua_rawset( L, -3 );
    }
    lua_pop(L,1);     // remove 'reg[REG_MTID]' reference

  STACK_END(L,0)
  
    return id;
}


static int buf_writer( lua_State *L, const void* b, size_t n, void* B ) {
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char *)b, n);
  return 0;
}


/* 
 * Check if we've already copied the same table from 'L', and
 * reuse the old copy. This allows table upvalues shared by multiple
 * local functions to point to the same table, also in the target.
 *
 * Always pushes a table to 'L2'.
 *
 * Returns TRUE if the table was cached (no need to fill it!); FALSE if
 * it's a virgin.
 */
static bool_t push_cached_table( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i )
{
	bool_t ret;

	ASSERT_L( L2_cache_i != 0 );

	STACK_GROW(L2,3);

	// L2_cache[id_str]= [{...}]
	//
	STACK_CHECK(L2)

	// We don't need to use the from state ('L') in ID since the life span
	// is only for the duration of a copy (both states are locked).
	//
	lua_pushlightuserdata( L2, (void*)lua_topointer( L, i )); // push a light userdata uniquely representing the table

	//fprintf( stderr, "<< ID: %s >>\n", lua_tostring(L2,-1) );

	lua_pushvalue( L2, -1 );
	lua_rawget( L2, L2_cache_i );
	//
	// [-2]: identity table pointer lightuserdata
	// [-1]: table|nil

	if (lua_isnil(L2,-1))
	{
		lua_pop(L2,1);
		lua_newtable(L2);
		lua_pushvalue(L2,-1);
		lua_insert(L2,-3);
		//
		// [-3]: new table (2nd ref)
		// [-2]: identity table pointer lightuserdata
		// [-1]: new table

		lua_rawset(L2, L2_cache_i);
		//
		// [-1]: new table (tied to 'L2_cache' table')

		ret= FALSE;     // brand new

	}
	else
	{
		lua_remove(L2,-2);
		ret= TRUE;      // from cache
	}
	STACK_END(L2,1)
	//
	// L2 [-1]: table to use as destination

	ASSERT_L( lua_istable(L2,-1) );
	return ret;
}


/* 
 * Check if we've already copied the same function from 'L', and reuse the old
 * copy.
 *
 * Always pushes a function to 'L2'.
 */
static void inter_copy_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i );

static void push_cached_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i )
{
	void * const aspointer = (void*)lua_topointer( L, i );
	// TBD: Merge this and same code for tables
	ASSERT_L( L2_cache_i != 0 );

	STACK_GROW(L2,3);

	// L2_cache[id_str]= function
	//
	STACK_CHECK(L2)

	// We don't need to use the from state ('L') in ID since the life span
	// is only for the duration of a copy (both states are locked).
	//
	lua_pushlightuserdata( L2, aspointer); // push a light userdata uniquely representing the function

	//fprintf( stderr, "<< ID: %s >>\n", lua_tostring(L2,-1) );

	lua_pushvalue( L2, -1 );
	lua_rawget( L2, L2_cache_i );
	//
	// [-2]: identity lightuserdata function pointer
	// [-1]: function|nil|true  (true means: we're working on it; recursive)

	if (lua_isnil(L2,-1))
	{
		lua_pop(L2,1);

		// Set to 'true' for the duration of creation; need to find self-references
		// via upvalues
		//
		lua_pushvalue( L2, -1);
		lua_pushboolean(L2,TRUE);
		lua_rawset( L2, L2_cache_i);

		inter_copy_func( L2, L2_cache_i, L, i );    // pushes a copy of the func

		lua_pushvalue(L2,-1);
		lua_insert(L2,-3);
		//
		// [-3]: function (2nd ref)
		// [-2]: identity lightuserdata function pointer
		// [-1]: function

		lua_rawset(L2,L2_cache_i);
		//
		// [-1]: function (tied to 'L2_cache' table')

	}
	else if (lua_isboolean(L2,-1))
	{
		// Loop in preparing upvalues; either direct or via a table
		// 
		// Note: This excludes the case where a function directly addresses
		//       itself as an upvalue (recursive lane creation).
		//
		STACK_GROW(L,1);
		luaL_error( L, "Recursive use of upvalues; cannot copy the function" );

	}
	else
	{
		lua_remove(L2,-2);
	}
	STACK_END(L2,1)
	//
	// L2 [-1]: function

	ASSERT_L( lua_isfunction(L2,-1));
}

/*
* Push a looked-up native/LuaJIT function.
*/
static void lookup_native_func( lua_State *L2, lua_State *L, uint_t i)
{
	char const *fqn;
	size_t len;
	_ASSERT_L( L, lua_isfunction( L, i));
	STACK_CHECK( L)
	STACK_CHECK( L2)
	// fetch the name from the source state's lookup table
	lua_getfield( L, LUA_REGISTRYINDEX, LOOKUP_KEY);         // {}
	_ASSERT_L( L, lua_istable( L, -1));
	lua_pushvalue( L, i);                                    // {} f
	lua_rawget( L, -2);                                      // {} "f.q.n"
	fqn = lua_tolstring( L, -1, &len);
	if( !fqn)
	{
		luaL_error( L, "function not found in origin transfer database.");
	}
	// push the equivalent function in the destination's stack, retrieved from the lookup table
	lua_getfield( L2, LUA_REGISTRYINDEX, LOOKUP_KEY);                               // {}
	_ASSERT_L( L2, lua_istable( L2, -1));
	lua_pushlstring( L2, fqn, len);                                                 // {} "f.q.n"
	lua_pop( L, 2);                                          //
	lua_rawget( L2, -2);                                                            // {} f
	if( !lua_isfunction( L2, -1))
	{
		// yarglah: luaL_error formatting doesn't support string width modifier!
		char message[256];
		sprintf( message, "function %*s not found in destination transfer database.", len, fqn);
		luaL_error( L, message);
	}
	lua_remove( L2, -2);                                                            // f
	STACK_END( L2, 1)
	STACK_END( L, 0)
}

#define LOG_FUNC_INFO 0

/*
* Copy a function over, which has not been found in the cache.
*/
enum e_vt {
    VT_NORMAL, VT_KEY, VT_METATABLE
};
static bool_t inter_copy_one_( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i, enum e_vt value_type );

static void inter_copy_func( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i )
{
	FuncSubType funcSubType;
	lua_CFunction cfunc = luaG_tocfunction( L, i, &funcSubType); // NULL for LuaJIT-fast && bytecode functions

	ASSERT_L( L2_cache_i != 0 );
	STACK_GROW(L,2);
	STACK_CHECK(L)

	if( funcSubType == FST_Bytecode)
	{
		unsigned n;
		luaL_Buffer b;
		// 'lua_dump()' needs the function at top of stack
		// if already on top of the stack, no need to push again
		int needToPush = (i != (uint_t)lua_gettop( L));
		if( needToPush)
			lua_pushvalue( L, i);

		luaL_buffinit( L, &b);
		//
		// "value returned is the error code returned by the last call 
		// to the writer" (and we only return 0)
		// not sure this could ever fail but for memory shortage reasons
		if( lua_dump( L, buf_writer, &b) != 0)
		{
			luaL_error( L, "internal error: function dump failed.");
		}

		luaL_pushresult( &b);    // pushes dumped string on 'L'

		// if not pushed, no need to pop
		if( needToPush)
		{
			lua_remove( L, -2);
		}

		// transfer the bytecode, then the upvalues, to create a similar closure
		{
			const char *name= NULL;

	#if LOG_FUNC_INFO
			// "To get information about a function you push it onto the 
			// stack and start the what string with the character '>'."
			//
			{
				lua_Debug ar;
				lua_pushvalue( L, i );
				lua_getinfo(L, ">nS", &ar);      // fills 'name' 'namewhat' and 'linedefined', pops function
				name= ar.namewhat;
				fprintf( stderr, "NAME: %s @ %d\n", ar.short_src, ar.linedefined);  // just gives NULL
			}
	#endif // LOG_FUNC_INFO
			{
				const char *s;
				size_t sz;
				s = lua_tolstring( L, -1, &sz);
				ASSERT_L( s && sz);

				// Note: Line numbers seem to be taken precisely from the 
				//       original function. 'name' is not used since the chunk
				//       is precompiled (it seems...). 
				//
				// TBD: Can we get the function's original name through, as well?
				//
				if (luaL_loadbuffer(L2, s, sz, name) != 0)
				{
					// chunk is precompiled so only LUA_ERRMEM can happen
					// "Otherwise, it pushes an error message"
					//
					STACK_GROW( L,1);
					luaL_error( L, "%s", lua_tostring(L2,-1));
				}
				lua_pop( L, 1);   // remove the dumped string
			}
			STACK_MID( L, 0)

			/* push over any upvalues; references to this function will come from
			* cache so we don't end up in eternal loop.
			*/
			for( n=0; lua_getupvalue( L, i, 1+n ) != NULL; n++ )
			{
				if ((!cfunc) && lua_equal(L,i,-1))
				{
					/* Lua closure that has a (recursive) upvalue to itself
					*/
					lua_pushvalue( L2, -((int)n)-1 );
				}
				else
				{
					if( !inter_copy_one_( L2, L2_cache_i, L, lua_gettop(L), VT_NORMAL))
						luaL_error( L, "Cannot copy upvalue type '%s'", luaG_typename( L, -1));
				}
				lua_pop( L, 1);
			}
			// L2: function + 'n' upvalues (>=0)

			STACK_MID(L,0)

			// Set upvalues (originally set to 'nil' by 'lua_load')
			{
				int func_index = lua_gettop( L2) - n;
				for( ; n > 0; -- n)
				{
					char const *rc = lua_setupvalue( L2, func_index, n);
					//
					// "assigns the value at the top of the stack to the upvalue and returns its name.
					// It also pops the value from the stack."

					ASSERT_L(rc);      // not having enough slots?
				}
			}
		}
	}
	else // C function OR LuaJIT fast function!!!
	{
#if LOG_FUNC_INFO
		fprintf( stderr, "NAME: [C] function %p \n", cfunc);
#endif // LOG_FUNC_INFO
		// No need to transfer upvalues for C/JIT functions since they weren't actually copied, only looked up
		lookup_native_func( L2, L, i);
	}
	STACK_END(L,0)
}

/*
* Copies a value from 'L' state (at index 'i') to 'L2' state. Does not remove
* the original value.
*
* NOTE: Both the states must be solely in the current OS thread's posession.
*
* 'i' is an absolute index (no -1, ...)
*
* Returns TRUE if value was pushed, FALSE if its type is non-supported.
*/
static bool_t inter_copy_one_( lua_State *L2, uint_t L2_cache_i, lua_State *L, uint_t i, enum e_vt vt )
{
    bool_t ret= TRUE;

    STACK_GROW( L2, 1 );

  STACK_CHECK(L2)

    switch ( lua_type(L,i) ) {
        /* Basic types allowed both as values, and as table keys */

        case LUA_TBOOLEAN:
            lua_pushboolean( L2, lua_toboolean(L, i) );
            break;

        case LUA_TNUMBER:
            /* LNUM patch support (keeping integer accuracy) */
#ifdef LUA_LNUM
            if (lua_isinteger(L,i)) {
                lua_pushinteger( L2, lua_tointeger(L, i) );
                break;
            }
#endif
            lua_pushnumber( L2, lua_tonumber(L, i) ); 
            break;

        case LUA_TSTRING: {
            size_t len; const char *s = lua_tolstring( L, i, &len );
            lua_pushlstring( L2, s, len );
            } break;

        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata( L2, lua_touserdata(L, i) );
            break;

        /* The following types are not allowed as table keys */

        case LUA_TUSERDATA: if (vt==VT_KEY) { ret=FALSE; break; }
            /* Allow only deep userdata entities to be copied across
             */
            if (!luaG_copydeep( L, L2, i )) {
                // Cannot copy it full; copy as light userdata
                //
                lua_pushlightuserdata( L2, lua_touserdata(L, i) );
            } break;

        case LUA_TNIL: if (vt==VT_KEY) { ret=FALSE; break; }
            lua_pushnil(L2);
            break;

        case LUA_TFUNCTION: if (vt==VT_KEY) { ret=FALSE; break; } {
            /* 
            * Passing C functions is risky; if they refer to LUA_ENVIRONINDEX
            * and/or LUA_REGISTRYINDEX they might work unintended (not work)
            * at the target.
            *
            * On the other hand, NOT copying them causes many self tests not
            * to work (timer, hangtest, ...)
            *
            * The trouble is, we cannot KNOW if the function at hand is safe
            * or not. We cannot study it's behaviour. We could trust the user,
            * but they might not even know they're sending lua_CFunction over
            * (as upvalues etc.).
            */
#if 0
            if (lua_iscfunction(L,i))
                luaL_error( L, "Copying lua_CFunction between Lua states is risky, and currently disabled." ); 
#endif
          STACK_CHECK(L2)
            push_cached_func( L2, L2_cache_i, L, i );
            ASSERT_L( lua_isfunction(L2,-1) );
          STACK_END(L2,1)
            } break;

        case LUA_TTABLE: if (vt==VT_KEY) { ret=FALSE; break; } {
        
          STACK_CHECK(L)
          STACK_CHECK(L2)

            /* Check if we've already copied the same table from 'L' (during this transmission), and
             * reuse the old copy. This allows table upvalues shared by multiple
             * local functions to point to the same table, also in the target.
             * Also, this takes care of cyclic tables and multiple references
             * to the same subtable.
             *
             * Note: Even metatables need to go through this test; to detect
             *      loops s.a. those in required module tables (getmetatable(lanes).lanes == lanes)
             */
            if (push_cached_table( L2, L2_cache_i, L, i )) {
                ASSERT_L( lua_istable(L2, -1) );    // from cache
                break;
            }
            ASSERT_L( lua_istable(L2,-1) );

            STACK_GROW( L, 2 );
            STACK_GROW( L2, 2 );

            lua_pushnil(L);    // start iteration
            while( lua_next( L, i ) ) {
                uint_t val_i= lua_gettop(L);
                uint_t key_i= val_i-1;

                /* Only basic key types are copied over; others ignored
                 */
                if (inter_copy_one_( L2, 0 /*key*/, L, key_i, VT_KEY )) {
                    /*
                    * Contents of metatables are copied with cache checking;
                    * important to detect loops.
                    */
                    if (inter_copy_one_( L2, L2_cache_i, L, val_i, VT_NORMAL )) {
                        ASSERT_L( lua_istable(L2,-3) );
                        lua_rawset( L2, -3 );    // add to table (pops key & val)
                    } else {
                        luaL_error( L, "Unable to copy over type '%s' (in %s)", 
                                        luaG_typename(L,val_i), 
                                        vt==VT_NORMAL ? "table":"metatable" );
                    }
                }
                lua_pop( L, 1 );    // pop value (next round)
            }
          STACK_MID(L,0)
          STACK_MID(L2,1)
          
            /* Metatables are expected to be immutable, and copied only once.
            */
            if (lua_getmetatable( L, i )) {
                //
                // L [-1]: metatable

                uint_t mt_id= get_mt_id( L, -1 );    // Unique id for the metatable

                STACK_GROW(L2,4);

                push_registry_subtable( L2, REG_MTID );
              STACK_MID(L2,2);
                lua_pushinteger( L2, mt_id );
                lua_rawget( L2, -2 );
                    //
                    // L2 ([-3]: copied table)
                    //    [-2]: reg[REG_MTID]
                    //    [-1]: nil/metatable pre-known in L2

              STACK_MID(L2,3);

                if (lua_isnil(L2,-1)) {   /* L2 did not know the metatable */
                    lua_pop(L2,1);
              STACK_MID(L2,2);
ASSERT_L( lua_istable(L,-1) );
                    if (inter_copy_one_( L2, L2_cache_i /*for function cacheing*/, L, lua_gettop(L) /*[-1]*/, VT_METATABLE )) {
                        //
                        // L2 ([-3]: copied table)
                        //    [-2]: reg[REG_MTID]
                        //    [-1]: metatable (copied from L)

              STACK_MID(L2,3);
                        // mt_id -> metatable
                        //
                        lua_pushinteger(L2,mt_id);
                        lua_pushvalue(L2,-2);
                        lua_rawset(L2,-4);

                        // metatable -> mt_id
                        //
                        lua_pushvalue(L2,-1);
                        lua_pushinteger(L2,mt_id);
                        lua_rawset(L2,-4);
                        
              STACK_MID(L2,3);
                    } else {
                        luaL_error( L, "Error copying a metatable" );
                    }
              STACK_MID(L2,3);
                }
                    // L2 ([-3]: copied table)
                    //    [-2]: reg[REG_MTID]
                    //    [-1]: metatable (pre-known or copied from L)

                lua_remove(L2,-2);   // take away 'reg[REG_MTID]'
                    //
                    // L2: ([-2]: copied table)
                    //     [-1]: metatable for that table

                lua_setmetatable( L2, -2 );
                
                // L2: [-1]: copied table (with metatable set if source had it)

                lua_pop(L,1);   // remove source metatable (L, not L2!)
            }
          STACK_END(L2,1)
          STACK_END(L,0)
            } break;

        /* The following types cannot be copied */

        case LUA_TTHREAD: 
            ret=FALSE; break;
    }

  STACK_END(L2, ret? 1:0)

    return ret;
}


/*
* Akin to 'lua_xmove' but copies values between _any_ Lua states.
*
* NOTE: Both the states must be solely in the current OS thread's posession.
*
* Note: Parameters are in this order ('L' = from first) to be same as 'lua_xmove'.
*/
int luaG_inter_copy( lua_State* L, lua_State *L2, uint_t n)
{
	uint_t top_L = lua_gettop( L);
	uint_t top_L2 = lua_gettop( L2);
	uint_t i;
	bool_t copyok = TRUE;

	if( n > top_L)
	{
		// requesting to copy more than is available?
		return -1;
	}

	STACK_GROW( L2, n + 1);

	/*
	* Make a cache table for the duration of this copy. Collects tables and
	* function entries, avoiding the same entries to be passed on as multiple
	* copies. ESSENTIAL i.e. for handling upvalue tables in the right manner!
	*/
	lua_newtable( L2);

	for( i = top_L - n + 1; i <= top_L; ++ i)
	{
		copyok = inter_copy_one_( L2, top_L2 + 1, L, i, VT_NORMAL);
		if( !copyok)
		{
			break;
		}
	}

	/*
	* Remove the cache table. Persistant caching would cause i.e. multiple 
	* messages passed in the same table to use the same table also in receiving
	* end.
	*/
	ASSERT_L( (uint_t) lua_gettop( L) == top_L);
	if( copyok)
	{
		lua_remove( L2, top_L2 + 1);
		ASSERT_L( (uint_t) lua_gettop( L2) == top_L2 + n);
		return 0;
	}
	else
	{
		// error -> pop everything from the target state stack
		lua_settop( L2, top_L2);
		return -2;
	}
}


int luaG_inter_move( lua_State* L, lua_State *L2, uint_t n )
{
	int ret = luaG_inter_copy( L, L2, n);
	lua_pop( L, (int) n);
	return ret;
}

/*---=== Serialize require ===---
*/

MUTEX_T require_cs;

//---
// [val]= new_require( ... )
//
// Call 'old_require' but only one lane at a time.
//
// Upvalues: [1]: original 'require' function
//
static int new_require( lua_State *L)
{
	int rc, i;
	int args = lua_gettop( L);
	//char const *modname = luaL_checkstring( L, 1);

	STACK_GROW( L, args + 1);
	STACK_CHECK( L)

	lua_pushvalue( L, lua_upvalueindex(1));
	for( i = 1; i <= args; ++ i)
		lua_pushvalue( L, i);

	// Using 'lua_pcall()' to catch errors; otherwise a failing 'require' would
	// leave us locked, blocking any future 'require' calls from other lanes.
	//
	MUTEX_LOCK( &require_cs);
	{
		rc = lua_pcall( L, args, 1 /*retvals*/, 0 /*errfunc*/ );
		//
		// LUA_ERRRUN / LUA_ERRMEM
	}
	MUTEX_UNLOCK( &require_cs);

	// the required module (or an error message) is left on the stack as returned value by original require function
	STACK_END( L, 1)

	if (rc)
		lua_error(L);   // error message already at [-1]

	return 1;
}

/*
* Serialize calls to 'require', if it exists
*/
void serialize_require( lua_State *L )
{
	STACK_GROW(L,1);
	STACK_CHECK(L)

	// Check 'require' is there; if not, do nothing
	//
	lua_getglobal( L, "require" );
	if (lua_isfunction( L, -1 ))
	{
		// [-1]: original 'require' function

		lua_pushcclosure( L, new_require, 1 /*upvalues*/ );
		lua_setglobal( L, "require" );

	}
	else
	{
		// [-1]: nil
		lua_pop(L,1);
	}

	STACK_END(L,0)
}
