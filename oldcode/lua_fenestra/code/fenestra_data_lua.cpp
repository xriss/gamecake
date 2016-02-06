/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int core_setup(lua_State *l)
{
	struct fenestra *fenestra = (struct fenestra *)lua_touserdata(l, 1 );

	fenestra->data->setup(fenestra);

	lua_pushlightuserdata(l,fenestra->data);

	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clean(lua_State *l)
{
	struct fenestra_data *core = (struct fenestra_data *)lua_touserdata(l, 1 );

	core->clean();
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load and relocate a data file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_load(lua_State *l)
{
	struct fenestra_data *core = (struct fenestra_data *)lua_touserdata(l, 1 );
	
	const char *s=lua_tostring(l,2);
	int length=lua_objlen(l,2);
	data_header *head=0;
	void *data;
	
	const char *ret="";

	if(length>0)
	{
		head=(data_header *)lua_newuserdata(l, length);
		
		if(head)
		{
			memcpy((void *)head,s,length); // copy from lua string into userdata
			
			head->twiddle();
			
			data=head->data(); // get start of real data
			
			switch(head->id)
			{
				case U32_ID4_XOX0:
				{
					if(head->version==XOX0_VERSION)
					{
						((XOX0_info*)(data))->relocate();
						ret="XOX";
					}
				}
				break;
				
				case U32_ID4_XSX0:
				{
					if(head->version==XSX0_VERSION)
					{
						((XSX0_info*)(data))->relocate();
						ret="XSX";
					}
				}
				break;
				
				case U32_ID4_XTX0:
				{
					if(head->version==XTX0_VERSION)
					{
						((XTX0_info*)(data))->relocate();
						ret="XTX";
					}
				}
				break;
			}
		}
		
	}
	
	if(!head) // failed to allocate?
	{
		lua_pushnil(l); // no data to return
	}
	
	lua_pushstring(l,ret); // type of return
	return 2;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	{"setup",					core_setup},
	{"clean",					core_clean},
	{"load",					core_load},

	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_fenestra_core_data (lua_State *l) {

	luaL_openlib (l, "fenestra.core.data", core_lib, 0);

	return 1;
}

