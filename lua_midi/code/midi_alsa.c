/*+-----------------------------------------------------------------------------------------------------------------+**

(C) Kriss@XIXs.com 2019

**+-----------------------------------------------------------------------------------------------------------------+*/

#include <lua.h>
#include <lauxlib.h>
#include <alsa/asoundlib.h>

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_midi_alsa_ptr_name="midi_alsa*ptr";


// the data pointer we are using for this library
typedef snd_seq_t * part_ptr ;


// pull in a hack
//extern "C" 
extern void * luaL_wetestudata(lua_State *L, int index, const char *tname);


/*+-----------------------------------------------------------------------------------------------------------------+**

check that a userdata at the given index is a midi_alsa object
return the part_ptr if it does, otherwise return 0

**+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_midi_alsa_get_ptr (lua_State *l, int idx)
{
part_ptr *p=0;

	p = ((part_ptr *)luaL_wetestudata(l, idx , lua_midi_alsa_ptr_name));

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

lua_midi_alsa_get_ptr but with raises an error on a null ptr and 
returns the dereference pointer

**+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_midi_alsa_check_ptr (lua_State *l, int idx)
{
part_ptr *p=lua_midi_alsa_get_ptr(l,idx);

	if( (p == 0) || (*p == 0) )
	{
		luaL_error(l, "bad midi_alsa userdata" );
	}

	return *p;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

alloc an item, returns userdata

**+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr * lua_midi_alsa_create_ptr(lua_State *l)
{
part_ptr *p;
	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	(*p)=0;
	luaL_getmetatable(l, lua_midi_alsa_ptr_name);
	lua_setmetatable(l, -2);
	return p;
}

int lua_midi_alsa_create(lua_State *l)
{
part_ptr *p;
	p=lua_midi_alsa_create_ptr(l);
	if( snd_seq_open(p,"default",SND_SEQ_OPEN_DUPLEX,0) < 0 )
	{
		return 0;
	}
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

destroy pointer in table at given index

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_destroy_idx (lua_State *l, int idx)
{
part_ptr *p=lua_midi_alsa_get_ptr(l,idx);
	if(p)
	{
		if(*p)
		{
			snd_seq_close(*p);
		}
		(*p)=0;
	}
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

destroy pointer in table

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_destroy (lua_State *l)
{
	lua_midi_alsa_destroy_idx(l, 1);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

get list of clients

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_clients (lua_State *l)
{
int idx=0;

	part_ptr p = lua_midi_alsa_check_ptr(l,1);

	snd_seq_client_info_t *c;
	if( snd_seq_client_info_malloc( &c ) < 0 ) { return 0; }

	lua_newtable(l);

	snd_seq_client_info_set_client( c , -1 );
	while( snd_seq_query_next_client( p , c) >= 0 )
	{
		lua_pushnumber(l,++idx);
		lua_newtable(l);

		lua_pushstring(l,"name");
		lua_pushstring(l,snd_seq_client_info_get_name(c));
		lua_rawset(l,-3);

		lua_pushstring(l,"client");
		lua_pushnumber(l,snd_seq_client_info_get_client(c));
		lua_rawset(l,-3);

		lua_pushstring(l,"ports");
		lua_pushnumber(l,snd_seq_client_info_get_num_ports(c));
		lua_rawset(l,-3);

		lua_rawset(l,-3);
	}

	snd_seq_client_info_free( c );

	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+**

open library.

**+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_midi_alsa_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"create",			lua_midi_alsa_create	},
		{"destroy",			lua_midi_alsa_destroy	},
		{"clients",			lua_midi_alsa_clients	},
		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{"__gc",			lua_midi_alsa_destroy	},
		{0,0}
	};

	luaL_newmetatable(l, lua_midi_alsa_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}
