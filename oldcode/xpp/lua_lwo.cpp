/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lualwo_load (lua_State *L)
{
lwObject *item;
unsigned int fail_ID;
int fail_pos;

char *filename;

	filename=(char*)luaL_checkstring(L, 1);

	item=lwGetObject(filename,&fail_ID,&fail_pos);

	if(item)
	{
        lua_pushlightuserdata(L, (void*)item);
		return 1;
	}
	else
	{
		return luaL_error(L, "Failed to load lwo \"%s\" error=%d pos=%d.",filename,fail_ID,fail_pos);
	}
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lualwo_free (lua_State *L)
{
lwObject *item;

	if(!lua_islightuserdata(L,1))
	{
		return luaL_error(L, "not a pointer.");
	}
	item=(lwObject *)lua_touserdata(L,1);

	lwFreeObject(item);

	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lualwo_dump (lua_State *L)
{
lwObject *item;
lwLayer *layer;

s32 i;

	if(!lua_islightuserdata(L,1))
	{
		return luaL_error(L, "not a pointer.");
	}
	item=(lwObject *)lua_touserdata(L,1);

	DBG_Info("surfs=%d\n",item->nsurfs);


      DBG_Info(
         "Layers:  %d\n"
         "Surfaces:  %d\n"
         "Envelopes:  %d\n"
         "Clips:  %d\n\n",
         item->nlayers, item->nsurfs, item->nenvs, item->nclips);


	

	  for(i=0 , layer=item->layer ; i<item->nlayers ; i++ , layer=layer->next)
	  {
      DBG_Info(
         "Points (layer %d):  %d\n"
         "Polygons (layer %d):  %d\n\n",
         i+1,item->layer->point.count, i+1,item->layer->polygon.count );
	  }


	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// library defs
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static const luaL_reg lualwolib[] = {
  {"load",   lualwo_load},
  {"free",   lualwo_free},
  {"dump",   lualwo_dump},
  {NULL, NULL}
};

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// open library.
//
/*----------------------------------------------------------------------------------------------------------------------------*/

int luaopen_lwo (lua_State *L) {
  luaL_openlib(L, LUA_LWOLIBNAME, lualwolib, 0);
  return 1;
}

