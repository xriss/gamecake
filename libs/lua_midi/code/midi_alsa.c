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
part_ptr *m=0;

	m = ((part_ptr *)luaL_wetestudata(l, idx , lua_midi_alsa_ptr_name));

	return m;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

lua_midi_alsa_get_ptr but with raises an error on a null ptr and 
returns the dereference pointer

**+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_midi_alsa_check_ptr (lua_State *l, int idx)
{
part_ptr *m=lua_midi_alsa_get_ptr(l,idx);

	if( (m == 0) || (*m == 0) )
	{
		luaL_error(l, "bad midi_alsa userdata" );
	}

	return *m;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

alloc an item, returns userdata

**+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr * lua_midi_alsa_create_ptr(lua_State *l)
{
part_ptr *m;
	m = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	(*m)=0;
	luaL_getmetatable(l, lua_midi_alsa_ptr_name);
	lua_setmetatable(l, -2);
	return m;
}

int lua_midi_alsa_create(lua_State *l)
{
part_ptr *m;
	m=lua_midi_alsa_create_ptr(l);
	if( snd_seq_open(m,"default",SND_SEQ_OPEN_DUPLEX,0) < 0 )
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
part_ptr *m=lua_midi_alsa_get_ptr(l,idx);
	if(m)
	{
		if(*m)
		{
			snd_seq_close(*m);
		}
		(*m)=0;
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

get all values associated with this created client

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_get (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

const char *s;

snd_seq_client_info_t *info;

	snd_seq_client_info_malloc(&info);
	snd_seq_get_client_info(m,info);


	s=snd_seq_client_info_get_name(info);
	lua_pushstring(l,"name");
	lua_pushstring(l,s);
	lua_rawset(l,2);
	
	lua_pushstring(l,"type");
	lua_pushnumber(l,(double)snd_seq_type(m));
	lua_rawset(l,2);

	lua_pushstring(l,"client");
	lua_pushnumber(l,(double)snd_seq_client_id(m));
	lua_rawset(l,2);

	snd_seq_client_info_free(info);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

set all values associated with this created client

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_set (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

const char *s;

snd_seq_client_info_t *info;

	snd_seq_client_info_malloc(&info);
	snd_seq_get_client_info(m,info);

	lua_getfield(l,2,"name");
	if( lua_isstring(l,-1) )
	{
		s = lua_tostring(l,-1);
		snd_seq_client_info_set_name(info, s);
	}
	lua_pop(l,1);

	snd_seq_set_client_info(m,info);
	snd_seq_client_info_free(info);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

get list of all clients and ports and connections

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_scan (lua_State *l)
{
int cidx=0;
int pidx=0;
int sidx=0;

	int flag=0;
	int port=0;
	int client=0;
	
	const snd_seq_addr_t *addr;

	snd_seq_query_subscribe_t *subs; // we have to malloc this?
	if(0!=snd_seq_query_subscribe_malloc(&subs))
	{
		return 0; // error
	}

	part_ptr m = lua_midi_alsa_check_ptr(l,1);

	

	snd_seq_client_info_t *c;
	snd_seq_client_info_alloca( &c );

	snd_seq_port_info_t *p;
	snd_seq_port_info_alloca( &p );

	if(lua_istable(l,2)) // input table?
	{
		lua_pushvalue(l,2); // input table is output
	}
	else
	{
		lua_newtable(l); // new table is output
	}
	
	lua_pushstring(l,"clients");
	lua_newtable(l);

	snd_seq_client_info_set_client( c , -1 );
	cidx=0;
	while( snd_seq_query_next_client( m , c) >= 0 )
	{
		lua_pushnumber(l,++cidx);
		lua_newtable(l);

		lua_pushstring(l,"name");
		lua_pushstring(l,snd_seq_client_info_get_name(c));
		lua_rawset(l,-3);

		client=snd_seq_client_info_get_client(c);
		lua_pushstring(l,"client");
		lua_pushnumber(l,client);
		lua_rawset(l,-3);

		lua_pushstring(l,"ports");
		lua_newtable(l);
        snd_seq_port_info_set_client(p, client );
        snd_seq_port_info_set_port(p, -1);
		pidx=0;
        while (snd_seq_query_next_port(m, p) == 0)
        {
			lua_pushnumber(l,++pidx);
			lua_newtable(l);

			port=snd_seq_port_info_get_port(p);
			lua_pushstring(l,"port");
			lua_pushnumber(l,(double)port);
			lua_rawset(l,-3);

			lua_pushstring(l,"type");
			lua_pushnumber(l,(double)snd_seq_port_info_get_type(p));
			lua_rawset(l,-3);
			
			lua_pushstring(l,"capability");
			lua_pushnumber(l,(double)snd_seq_port_info_get_capability(p));
			lua_rawset(l,-3);
			
			lua_pushstring(l,"name");
			lua_pushstring(l,snd_seq_port_info_get_name(p));
			lua_rawset(l,-3);
			
			lua_pushstring(l,"client");
			lua_pushnumber(l,client);
			lua_rawset(l,-3);


			lua_pushstring(l,"subscriptions");
			lua_newtable(l);

			sidx=0;
			snd_seq_query_subscribe_set_client(subs, client);
			snd_seq_query_subscribe_set_port(subs, port);
			snd_seq_query_subscribe_set_type(subs, SND_SEQ_QUERY_SUBS_READ );
			snd_seq_query_subscribe_set_index(subs, 0);
			while(snd_seq_query_port_subscribers(m, subs) >= 0)
			{

				lua_pushnumber(l,++sidx);
				lua_newtable(l);
				
				lua_pushstring(l,"source_client");
				lua_pushnumber(l,client);
				lua_rawset(l,-3);
				lua_pushstring(l,"source_port");
				lua_pushnumber(l,port);
				lua_rawset(l,-3);

				addr=snd_seq_query_subscribe_get_addr(subs);
				lua_pushstring(l,"dest_client");
				lua_pushnumber(l,addr->client);
				lua_rawset(l,-3);
				lua_pushstring(l,"dest_port");
				lua_pushnumber(l,addr->port);
				lua_rawset(l,-3);

				lua_pushstring(l,"queue");
				lua_pushnumber(l,snd_seq_query_subscribe_get_queue(subs));
				lua_rawset(l,-3);

				lua_pushstring(l,"exclusive");
				lua_pushboolean(l,snd_seq_query_subscribe_get_exclusive(subs));
				lua_rawset(l,-3);

				lua_pushstring(l,"time_update");
				lua_pushboolean(l,snd_seq_query_subscribe_get_time_update(subs));
				lua_rawset(l,-3);

				lua_pushstring(l,"time_real");
				lua_pushboolean(l,snd_seq_query_subscribe_get_time_real(subs));
				lua_rawset(l,-3);

				lua_rawset(l,-3);

				snd_seq_query_subscribe_set_index(subs, sidx);
			}

			lua_rawset(l,-3);
			

			lua_rawset(l,-3);
		}
		lua_rawset(l,-3);


		lua_rawset(l,-3);
	}


	lua_rawset(l,-3);

	snd_seq_query_subscribe_free(subs);

	return 1;
}



static void fill_subs( lua_State *l , snd_seq_port_subscribe_t *sub , int idx )
{
	int i;
	snd_seq_addr_t addr[1];

	lua_getfield(l,idx,"source_client");
	if( lua_isnumber(l,-1) )
	{
		addr->client = lua_tonumber(l,-1);
		lua_getfield(l,idx,"source_port");
		if( lua_isnumber(l,-1) )
		{
			addr->port = lua_tonumber(l,-1);
			snd_seq_port_subscribe_set_sender(sub, addr);
		}
		lua_pop(l,1);
	}
	lua_pop(l,1);

	lua_getfield(l,idx,"dest_client");
	if( lua_isnumber(l,-1) )
	{
		addr->client = lua_tonumber(l,-1);
		lua_getfield(l,idx,"dest_port");
		if( lua_isnumber(l,-1) )
		{
			addr->port = lua_tonumber(l,-1);
			snd_seq_port_subscribe_set_dest(sub, addr);
		}
		lua_pop(l,1);
	}
	lua_pop(l,1);

	lua_getfield(l,idx,"time_update");
	if( lua_isnumber(l,-1) )
	{
		i = lua_tonumber(l,-1);
		snd_seq_port_subscribe_set_time_update(sub, i);
	}
	lua_pop(l,1);

	lua_getfield(l,idx,"time_real");
	if( lua_isnumber(l,-1) )
	{
		i = lua_tonumber(l,-1);
		snd_seq_port_subscribe_set_time_real(sub, i);
	}
	lua_pop(l,1);

	lua_getfield(l,idx,"queue");
	if( lua_isnumber(l,-1) )
	{
		i = lua_tonumber(l,-1);
		snd_seq_port_subscribe_set_queue(sub, i);
	}
	lua_pop(l,1);

	lua_getfield(l,idx,"exclusive");
	if( lua_isnumber(l,-1) )
	{
		i = lua_tonumber(l,-1);
		snd_seq_port_subscribe_set_exclusive(sub, i);
	}
	lua_pop(l,1);

}

/*+-----------------------------------------------------------------------------------------------------------------+**

subscribe ports

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_subscribe (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	snd_seq_port_subscribe_t* sub;
	int err;

	snd_seq_port_subscribe_alloca(&sub);
	
	fill_subs(l,sub,2);

	if((err=snd_seq_subscribe_port(m, sub)))
	{
		return 0;
	}

	lua_pushboolean(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

subscribe ports

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_unsubscribe (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	snd_seq_port_subscribe_t* sub;
	int err;

	snd_seq_port_subscribe_alloca(&sub);
	
	fill_subs(l,sub,2);

	if((err=snd_seq_unsubscribe_port(m, sub)))
	{
		return 0;
	}

	lua_pushboolean(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

port create

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_port_create (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	int port = snd_seq_create_simple_port(m,
		luaL_checkstring(l,2),  // name
		luaL_checknumber(l,3),  // caps
		luaL_checknumber(l,4)); // type

	if( port<0 ) { return 0; }

	lua_pushnumber(l,port);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+**

port destroy

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_port_destroy (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	int port = luaL_checknumber(l,2);

	if( snd_seq_delete_simple_port(m,port) < 0 )
	{
		return 0;
	}

	lua_pushboolean(l,1);
	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+**

port set

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_port_set (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);
const char *s;

	lua_getfield(l,2,"name");
	if( lua_isstring(l,-1) )
	{
		s = lua_tostring(l,-1);
//		snd_seq_client_info_set_name(sub, s);
	}
	lua_pop(l,1);


	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+**

port get

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_port_get (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+**

pull an event ( or an error )

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_pull (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	snd_seq_event_t *e;

	snd_seq_event_input(m, &e); // get event
	if(!e) { return 0; }

	lua_newtable(l);

	int idx=1;
	int b=0;
	
	lua_pushstring(l,"type");
	lua_pushnumber(l,(double)e->type);
	lua_rawset(l,-3);

	lua_pushstring(l,"flags");
	lua_pushnumber(l,(double)e->flags);
	lua_rawset(l,-3);

	lua_pushstring(l,"tag");
	lua_pushnumber(l,(double)e->tag);
	lua_rawset(l,-3);

	lua_pushstring(l,"queue");
	lua_pushnumber(l,(double)e->queue);
	lua_rawset(l,-3);

	lua_pushstring(l,"time_tick");
	lua_pushnumber(l,(double)e->time.tick);
	lua_rawset(l,-3);

	lua_pushstring(l,"source_client");
	lua_pushnumber(l,(double)(e->source.client));
	lua_rawset(l,-3);
	lua_pushstring(l,"source_port");
	lua_pushnumber(l,(double)(e->source.port));
	lua_rawset(l,-3);

	lua_pushstring(l,"dest_client");
	lua_pushnumber(l,(double)(e->dest.client));
	lua_rawset(l,-3);
	lua_pushstring(l,"dest_port");
	lua_pushnumber(l,(double)(e->dest.port));
	lua_rawset(l,-3);

	for(b=0;b<12;b++)
	{
		lua_pushnumber(l,b+1);
		lua_pushnumber(l,(double)e->data.raw8.d[b]);
		lua_rawset(l,-3);
	}

	for(b=0;b<3;b++)
	{
		lua_pushnumber(l,b+13);
		lua_pushnumber(l,(double)e->data.raw32.d[b]);
		lua_rawset(l,-3);
	}

	snd_seq_free_event(e); // free event
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

peek an event, non blocking and may return nil

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_peek (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);

	if( snd_seq_event_input_pending(m, 0) > 0)
	{
		return lua_midi_alsa_pull(l);
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+**

push an event

**+-----------------------------------------------------------------------------------------------------------------+*/
int lua_midi_alsa_push (lua_State *l)
{
part_ptr m = lua_midi_alsa_check_ptr(l,1);
int b=0;

	snd_seq_event_t e[1]={0};
	

	lua_getfield(l,2,"type");
	e->type=(snd_seq_event_type_t)luaL_checknumber(l,-1);
	lua_pop(l,1);

	lua_getfield(l,2,"source_client");
	e->source.client=(snd_seq_event_type_t)luaL_checknumber(l,-1);
	lua_pop(l,1);

	lua_getfield(l,2,"source_port");
	e->source.port=(snd_seq_event_type_t)luaL_checknumber(l,-1);
	lua_pop(l,1);

	lua_getfield(l,2,"dest_client");
	e->dest.client=(snd_seq_event_type_t)luaL_checknumber(l,-1);
	lua_pop(l,1);

	lua_getfield(l,2,"dest_port");
	e->dest.port=(snd_seq_event_type_t)luaL_checknumber(l,-1);
	lua_pop(l,1);

	lua_rawgeti(l,2,13);
	if( lua_isnumber(l,-1) ) // test for 32bit data
	{
		if(lua_isnumber(l,-1))
		{
			e->data.raw32.d[0]=lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_rawgeti(l,2,14);
		if(lua_isnumber(l,-1))
		{
			e->data.raw32.d[1]=lua_tonumber(l,-1);
		}
		lua_pop(l,1);

		lua_rawgeti(l,2,15);
		if(lua_isnumber(l,-1))
		{
			e->data.raw32.d[2]=lua_tonumber(l,-1);
		}
		lua_pop(l,1);
	}
	else // if no 32bit data then use 8bit data
	{
		lua_pop(l,1); // remove the 32bit test nil
		for(b=0;b<12;b++)
		{
			lua_rawgeti(l,2,b+1);
			if(lua_isnumber(l,-1))
			{
				e->data.raw8.d[b]=lua_tonumber(l,-1);
			}
			lua_pop(l,1);
		}
	}


	snd_seq_ev_set_direct(e);

	snd_seq_event_output_direct(m,e);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+**

open library.

**+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_midi_alsa_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"create",			lua_midi_alsa_create		},
		{"destroy",			lua_midi_alsa_destroy		},
		{"get",				lua_midi_alsa_get			},
		{"set",				lua_midi_alsa_set			},
		
		{"port_create",		lua_midi_alsa_port_create	},
		{"port_destroy",	lua_midi_alsa_port_destroy	},
		{"port_get",		lua_midi_alsa_port_get		},
		{"port_set",		lua_midi_alsa_port_set		},

		{"scan",			lua_midi_alsa_scan			},

		{"subscribe",		lua_midi_alsa_subscribe		},
		{"unsubscribe",		lua_midi_alsa_unsubscribe	},

		{"peek",			lua_midi_alsa_peek			},
		{"pull",			lua_midi_alsa_pull			},
		{"push",			lua_midi_alsa_push			},

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
