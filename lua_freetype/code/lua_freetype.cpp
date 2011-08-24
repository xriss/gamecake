/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

// although we pass in an "object" as the first value, we actually pull information from these upvalues
// this is good if you only have a small number of object and many functions
// bad if you have a small number of functions and many objects
// so we can choose to dump the upvalues later if this turns out to be a problem and instead use the actual objects

#define UPVALUE_LIB 1
#define UPVALUE_PTR 2
#define UPVALUE_TAB 3

void lua_freetype_tab_openlib (lua_State *l, int upvalues);



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_freetype_ptr_name="freetype*ptr";


// the main data struct/pointer we are using

typedef struct lua_freetype_font part_struct ;
typedef part_struct * part_ptr ;


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a table at the given index contains a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_freetype_check (lua_State *l, int idx)
{
part_ptr p;

	lua_pushstring(l, lua_freetype_ptr_name );
	lua_gettable(l,idx);

	p=(part_ptr )(*(void **)luaL_checkudata(l,lua_gettop(l),lua_freetype_ptr_name));

	lua_pop(l,1);

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null so this never ever fails...
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_freetype_get_ptr (lua_State *l)
{
part_ptr p;

	p=(part_ptr )(*(void **)lua_touserdata(l,lua_upvalueindex(UPVALUE_PTR)));


	if (p == 0)
	{
		luaL_error(l, "null pointer in freetype usedata" );
	}

	return p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_getinfo (lua_State *l, part_ptr p, int tab)
{
	if(p)
	{
		if(p->face)
		{
			
lua_pushliteral(l,"num_glyphs"); if(p->face) { lua_pushnumber(l,p->face->num_glyphs);  } else { lua_pushnil(l); } lua_rawset(l,tab);
lua_pushliteral(l,"family_name");if(p->face) { lua_pushstring(l,p->face->family_name); } else { lua_pushnil(l); } lua_rawset(l,tab);
lua_pushliteral(l,"style_name"); if(p->face) { lua_pushstring(l,p->face->style_name);  } else { lua_pushnil(l); } lua_rawset(l,tab);

			lua_pushliteral(l,"font_box");
			if(p->face)
			{
				lua_newtable(l);
				lua_pushliteral(l,"xmin"); lua_pushnumber(l,p->face->bbox.xMin/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"xmax"); lua_pushnumber(l,p->face->bbox.xMax/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"ymin"); lua_pushnumber(l,p->face->bbox.yMin/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"ymax"); lua_pushnumber(l,p->face->bbox.yMax/64.0); lua_rawset(l,-3);
			}
			else
			{
				lua_pushnil(l);
			}
			lua_rawset(l,tab);


			if(p->face->glyph)
			{
				lua_pushliteral(l,"bitmap_left"); lua_pushnumber(l,p->face->glyph->bitmap_left); lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_top"); lua_pushnumber(l,p->face->glyph->bitmap_top); lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_width"); lua_pushnumber(l,p->face->glyph->bitmap.width); lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_height"); lua_pushnumber(l,p->face->glyph->bitmap.rows); lua_rawset(l,tab);
				
				lua_pushliteral(l,"advance"); lua_pushnumber(l,p->face->glyph->linearHoriAdvance/65536.0); lua_rawset(l,tab);
				lua_pushliteral(l,"advance_vert"); lua_pushnumber(l,p->face->glyph->linearVertAdvance/65536.0); lua_rawset(l,tab);
			}

		}
		else
		{
			p->error=-1;
		}
		
		lua_pushliteral(l,"error");
		if(p->error) 	{ lua_pushnumber(l,p->error); }
		else		{ lua_pushnil(l); }
		lua_rawset(l,tab);
		
	}
	else
	{
		lua_pushliteral(l,"err"); lua_pushstring(l,"unbound freetype"); lua_rawset(l,tab);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_create (lua_State *l)
{
part_ptr *pp;
const char *s;

int idx_ptr;
int idx_tab;

	pp = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ptr=lua_gettop(l);

	(*pp)=0;

	luaL_getmetatable(l, lua_freetype_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

	idx_tab=lua_gettop(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // put our base table
	lua_pushvalue(l, idx_ptr ); // put our userdata,
	lua_pushvalue(l, idx_tab ); // put our userdata,

	lua_freetype_tab_openlib(l,3);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_freetype_ptr_name );
	lua_pushvalue(l, idx_ptr ); // get our userdata,
	lua_rawset(l,-3);


	(*pp)=0;


	(*pp)=(part_ptr)calloc(sizeof(part_struct),1);
	
	(*pp)->error = FT_Init_FreeType( &(*pp)->library );
	if( !(*pp)->error )
	{
		s=lua_tostring(l,1); // the file name of the font to open
		(*pp)->error = FT_New_Face( (*pp)->library,
			s,
			0,
			&(*pp)->face );			
	}

	
	lua_freetype_getinfo(l,*pp,lua_gettop(l));

	lua_remove(l, idx_ptr );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy_idx (lua_State *l, int idx)
{
part_ptr *pp;
	
	pp = (part_ptr *)luaL_checkudata(l, idx, lua_freetype_ptr_name);

	if(*pp)
	{
		if((*pp)->library) // if set then we allocated things
		{
			FT_Done_FreeType((*pp)->library); // free everything
			(*pp)->library=0;
		}
		free(*pp);
	}
	(*pp)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy_ptr (lua_State *l)
{
	return lua_freetype_destroy_idx(l,1);
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy (lua_State *l)
{
	return lua_freetype_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_size (lua_State *l)
{
part_ptr p=lua_freetype_get_ptr(l);

int width=lua_tonumber(l,2);
int height=lua_tonumber(l,3);

	p->error=FT_Set_Pixel_Sizes( p->face, width, height );

	lua_freetype_getinfo(l,p,1);
	
	return 0;
}

			
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// select a glyph ( do not render )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_glyph (lua_State *l)
{
part_ptr p=lua_freetype_get_ptr(l);

int ucode=lua_tonumber(l,2);

int glyph_index=FT_Get_Char_Index( p->face, ucode);

	p->error=FT_Load_Glyph( p->face, glyph_index , 0 );

	lua_freetype_getinfo(l,p,1);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Select a glyph and render its bitmap
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_render (lua_State *l)
{
part_ptr p=lua_freetype_get_ptr(l);

int ucode=lua_tonumber(l,2);

int glyph_index=FT_Get_Char_Index( p->face, ucode);

	p->error=FT_Load_Glyph( p->face, glyph_index , 0 );

	p->error = FT_Render_Glyph( p->face->glyph , FT_RENDER_MODE_NORMAL ); 
      
    lua_freetype_getinfo(l,p,1);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get the bitmapdata of the current glyph as a table.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_bitmap (lua_State *l)
{
part_ptr p=lua_freetype_get_ptr(l);

int x,y,i;
unsigned char* b;

	if(p->face->glyph)
	{
		b=p->face->glyph->bitmap.buffer;
		
		lua_newtable(l);
		i=1;
		for( y=0 ; y<p->face->glyph->bitmap.rows ; y++ )
		{
			for( x=0 ; x<p->face->glyph->bitmap.width ; x++ )
			{
				lua_pushnumber(l,b[x]);
				lua_rawseti(l,-2,i++);
			}
			b+=p->face->glyph->bitmap.pitch;
		}
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}
                
            
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_freetype_destroy_ptr},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our tab functions
//
// all functions expect the self table to be passed in as arg1 and as an upvalue
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"destroy",			lua_freetype_destroy},
		{"size",			lua_freetype_size},
		{"glyph",			lua_freetype_glyph},
		{"render",			lua_freetype_render},
		{"bitmap",			lua_freetype_bitmap},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"create"	,	lua_freetype_create	},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_freetype (lua_State *l)
{


	luaL_newmetatable(l, lua_freetype_ptr_name);
	lua_freetype_ptr_openlib(l,0);
	lua_pop(l,1);

	lua_newtable(l);
	lua_pushstring(l, LUA_freetype_LIB_NAME );
	lua_pushvalue(l, -2); // have this table as the first up value?
	lua_pushvalue(l, -1); // and we need to save one to return
	lua_freetype_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int luaclose_freetype (lua_State *l)
{
	lua_pushstring(l, LUA_freetype_LIB_NAME);
	lua_pushnil(l);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 0;
}
