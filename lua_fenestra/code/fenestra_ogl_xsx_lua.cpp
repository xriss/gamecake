/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_xsx_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XSX0_header *xsx_head = (struct XSX0_header *)lua_touserdata(l, 2 );
	struct XSX0_info   *xsx_info = xsx_head->data();
	
	struct XSX *xsx = (struct XSX *)lua_newuserdata(l, core->xsx_sizeof(xsx_info) );
	
	if(xsx)
	{
		core->xsx_setup(xsx,xsx_info);
	}
			
	return 1;
}
static int core_xsx_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XSX *xsx = (struct XSX *)lua_touserdata(l, 2 );
	
	if(xsx)
	{
		core->xsx_clean(xsx);
	}
			
	return 0;
}
static int core_xsx_draw(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XSX *xsx = (struct XSX *)lua_touserdata(l, 2 );
	
	float f = (float)lua_tonumber(l, 3 );
	
	if(xsx)
	{
		core->xsx_update(xsx,f);
		core->xsx_draw(xsx);
	}
			
	return 0;
}
static int core_xsx_get(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XSX *xsx = (struct XSX *)lua_touserdata(l, 2 );
	
	if(xsx)
	{
		lua_newtable(l);
		
//some interesting numbers	
		lua_pushnumber(l,xsx->info->start);
		lua_setfield(l,-2,"start");
		lua_pushnumber(l,xsx->info->length);
		lua_setfield(l,-2,"length");

		lua_newtable(l);
		
XSX_item *item;
int i;
		for( item=xsx->items , i=1 ; item<xsx->items+xsx->numof_items ; item++ , i++)
		{

			lua_newtable(l);
			
			lua_pushstring(l,item->item->name);
			lua_setfield(l,-2,"name");
			
			lua_pushnumber(l,item->item->type);
			lua_setfield(l,-2,"type");
			
			lua_pushnumber(l,item->item->flags);
			lua_setfield(l,-2,"flags");
			
			lua_newtable(l);
			lua_pushnumber(l,item->pos->x);
			lua_rawseti(l,-2,1);
			lua_pushnumber(l,item->pos->y);
			lua_rawseti(l,-2,2);
			lua_pushnumber(l,item->pos->z);
			lua_rawseti(l,-2,3);
			lua_setfield(l,-2,"pos");
			
			lua_newtable(l);
			lua_pushnumber(l,item->siz->x);
			lua_rawseti(l,-2,1);
			lua_pushnumber(l,item->siz->y);
			lua_rawseti(l,-2,2);
			lua_pushnumber(l,item->siz->z);
			lua_rawseti(l,-2,3);
			lua_setfield(l,-2,"size");
			
			lua_newtable(l);
			lua_pushnumber(l,item->morphs[0]);
			lua_rawseti(l,-2,1);
			lua_pushnumber(l,item->morphs[1]);
			lua_rawseti(l,-2,2);
			lua_pushnumber(l,item->morphs[2]);
			lua_rawseti(l,-2,3);
			lua_pushnumber(l,item->morphs[3]);
			lua_rawseti(l,-2,4);
			lua_setfield(l,-2,"morphs");
			
			lua_rawseti(l,-2,i);
				
		}
		
		lua_setfield(l,-2,"items");
	}
	
	return 1;
}
static int core_xsx_set(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XSX *xsx = (struct XSX *)lua_touserdata(l, 2 );
	struct XOX *xox;
	
	
	if(xsx)
	{
	
		lua_getfield(l,3,"items");
		
	// 3 is the main table we should be updating data from
		
	XOX0_morph *m;
	XOX_morph *mi;
	XSX_item *item;
	XSX_stream *s;
	int i;
	int j;
	int k;

		for( item=xsx->items , i=1 ; item<xsx->items+xsx->numof_items ; item++ , i++)
		{
			lua_rawgeti(l,-1,i);
			
			lua_getfield(l,-1,"pos");
			lua_rawgeti(l,-1,1);
			item->pos->x=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			item->pos->y=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,3);
			item->pos->z=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_pop(l,1);
 					
			lua_getfield(l,-1,"size");
			lua_rawgeti(l,-1,1);
			item->siz->x=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			item->siz->y=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,3);
			item->siz->z=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_pop(l,1);
 					
			lua_getfield(l,-1,"morphs");
			lua_rawgeti(l,-1,1);
			item->morphs[0]=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			item->morphs[1]=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,3);
			item->morphs[2]=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,4);
			item->morphs[3]=(f32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_pop(l,1);
			
			for(j=1;j<=4;j++)
			{
				xox=0;
				
				lua_rawgeti(l,-1,j);
				if( lua_istable(l,-1) )
				{
					lua_getfield(l,-1,"core");
					xox = (struct XOX *)lua_touserdata(l, -1 );
					lua_pop(l,1);
				}
				lua_pop(l,1);

				
				item->xoxs[j-1]=xox; // draw this
				if(xox)
				{
					if(item->item->flags&XSX0_ITEM_FLAG_FLIPX)
					{
						xox->flipX=true;
					}
					else
					{
						xox->flipX=false;
					}
					xox->need_rebuild=true;
					
					for( m=xox->info->morphs , mi=xox->morphs ; m<xox->info->morphs+xox->info->numof_morphs ; m++ , mi++ )
					{
						mi->link=0;
						for( s=item->streams , k=0 ; s<item->streams+item->numof_streams ; s++ , k++ )
						{
							if(k>=9)
							{
								if	(
										(m->name[0]) &&
										(s->stream->name[0]) &&
										(stricmp(m->name,s->stream->name)==0)
									)
								{
									mi->link=k;
									break;
								}
							}
						}
					}
				}
			}
			
			lua_pop(l,1);
		}
		lua_pop(l,1);
	
	}
	
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {

	{"xsx_setup",				core_xsx_setup},
	{"xsx_clean",				core_xsx_clean},
	{"xsx_draw",				core_xsx_draw},
	{"xsx_get",					core_xsx_get},
	{"xsx_set",					core_xsx_set},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

LUALIB_API int luaopen_fenestra_core_ogl_xsx (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);

	return 1;
}

