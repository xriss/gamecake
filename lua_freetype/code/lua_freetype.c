/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


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
// check that a userdata at the given index is a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_freetype_get_ptr (lua_State *l, int idx)
{
part_ptr *p=0;

	p = ((part_ptr *)luaL_checkudata(l, idx , lua_freetype_ptr_name));

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua_grd_check but with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_freetype_check_ptr (lua_State *l, int idx)
{
part_ptr *p=lua_freetype_get_ptr(l,idx);

	if (*p == 0)
	{
		luaL_error(l, "bad freetype userdata" );
	}

	return *p;
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
			
lua_pushliteral(l,"num_glyphs"); if(p->face             ) { lua_pushnumber(l,p->face->num_glyphs);  } else { lua_pushnil(l); } lua_rawset(l,tab);
lua_pushliteral(l,"family_name");if(p->face->family_name) { lua_pushstring(l,p->face->family_name); } else { lua_pushnil(l); } lua_rawset(l,tab);
lua_pushliteral(l,"style_name"); if(p->face->style_name ) { lua_pushstring(l,p->face->style_name);  } else { lua_pushnil(l); } lua_rawset(l,tab);

			lua_pushliteral(l,"font_box");
//			if(p->face->bbox)
//			{
				lua_newtable(l);
				lua_pushliteral(l,"xmin"); lua_pushnumber(l,p->face->bbox.xMin/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"xmax"); lua_pushnumber(l,p->face->bbox.xMax/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"ymin"); lua_pushnumber(l,p->face->bbox.yMin/64.0); lua_rawset(l,-3);
				lua_pushliteral(l,"ymax"); lua_pushnumber(l,p->face->bbox.yMax/64.0); lua_rawset(l,-3);
//			}
//			else
//			{
//				lua_pushnil(l);
//			}
			lua_rawset(l,tab);


			if(p->face->glyph)
			{
				lua_pushliteral(l,"bitmap_left");   lua_pushnumber(l,p->face->glyph->bitmap_left);  lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_top");    lua_pushnumber(l,p->face->glyph->bitmap_top);   lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_width");  lua_pushnumber(l,p->face->glyph->bitmap.width); lua_rawset(l,tab);
				lua_pushliteral(l,"bitmap_height"); lua_pushnumber(l,p->face->glyph->bitmap.rows);  lua_rawset(l,tab);
				
				lua_pushliteral(l,"advance");       lua_pushnumber(l,p->face->glyph->linearHoriAdvance/65536.0); lua_rawset(l,tab);
				lua_pushliteral(l,"advance_vert");  lua_pushnumber(l,p->face->glyph->linearVertAdvance/65536.0); lua_rawset(l,tab);
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
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_info (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);
	lua_freetype_getinfo(l,p,2);
	lua_pushvalue(l,2);
	return 1;
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
int slen;

	pp = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	(*pp)=0;
	luaL_getmetatable(l, lua_freetype_ptr_name);
	lua_setmetatable(l, -2);

	(*pp)=(part_ptr)calloc(sizeof(part_struct),1);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load font from file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_load_file (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);
const char *s;
size_t slen;
	
	p->error = FT_Init_FreeType( &p->library );
	if( !p->error )
	{
		s=lua_tolstring(l,2,&slen); // the file name of the font to open
		p->error = FT_New_Face( p->library,
			s,
			0,
			&p->face );			
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load font from memory
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_load_data (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);
const char *s;
size_t slen;
	
	p->error = FT_Init_FreeType( &p->library );
	if( !p->error )
	{
		s=lua_tolstring(l,2,&slen); // the file name of the font to open
		p->error = FT_New_Memory_Face( p->library,
						  (const FT_Byte*)s,
						  slen,
						  0,
						  &p->face );
	}

	return 0;
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
int lua_freetype_destroy (lua_State *l)
{
	return lua_freetype_destroy_idx(l,1);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set font size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_size (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);

int width=lua_tonumber(l,2);
int height=lua_tonumber(l,3);

	p->error=FT_Set_Pixel_Sizes( p->face, width, height );

	return 0;
}

			
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// select a glyph ( do not render )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_glyph (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);

int ucode=lua_tonumber(l,2);

int glyph_index=FT_Get_Char_Index( p->face, ucode);

/*
	if(glyph_index==0) // replace missing glyphs with spaces?
	{
		glyph_index=FT_Get_Char_Index( p->face, 32);
		if(glyph_index!=0)
		{
			p->error=FT_Load_Glyph( p->face, glyph_index , 0 );
		}
	}
	else
	{
		p->error=FT_Load_Glyph( p->face, glyph_index , 0 );
	}
*/

	p->error=FT_Load_Glyph( p->face, glyph_index , 0 );	

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Select a glyph and render its bitmap
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_render (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);

int ucode=(int)lua_tonumber(l,2);

int glyph_index=FT_Get_Char_Index( p->face, ucode);

	if(glyph_index==0) // bad code
	{
		lua_pushboolean(l,0);
		return 1;
	}

	p->error=FT_Load_Glyph( p->face, glyph_index , FT_LOAD_RENDER );

	if(p->error)
	{
//printf("ERROR %d\n",p->error);
		lua_pushboolean(l,0);
		return 1;
	}
	lua_pushboolean(l,1);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get the bitmapdata of the current glyph as a numeric table.
// good for debugering
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_tab (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);

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
// get the bitmapdata of the current glyph as a grd.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_grd (lua_State *l)
{
part_ptr p=lua_freetype_check_ptr(l,1);

int x,y,i;
unsigned char* b;
unsigned char* c;

struct grd *g;

	if(p->face && p->face->glyph)
	{
		b=p->face->glyph->bitmap.buffer;

		g=lua_grd_check_ptr(l,2); // need a grd to write too
		
		g=grd_realloc(g,GRD_FMT_U8_ALPHA,p->face->glyph->bitmap.width,p->face->glyph->bitmap.rows,1); // new size
		
		if(!g)
		{
			lua_pushnil(l);
			lua_pushstring(l,"failed to realloc grd");
			return 2;
		}

		for( y=0 ; y<p->face->glyph->bitmap.rows ; y++ )
		{
			c=grdinfo_get_data(g->bmap,0,y,0);
			for( x=0 ; x<p->face->glyph->bitmap.width ; x++ )
			{
				c[x]=b[x]; // save into grd
			}
			b+=p->face->glyph->bitmap.pitch;
		}
		
		lua_pushboolean(l,1);
		
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}

/*
FT_Vector  delta;


      FT_Get_Kerning( face, previous, glyph_index,
                      FT_KERNING_DEFAULT, &delta );

      pen_x += delta.x >> 6;
*/
      

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_wetgenes_freetype_core (lua_State *l)
{

	const luaL_Reg lib[] =
	{
		{"create",			lua_freetype_create	},

		{"load_file",		lua_freetype_load_file	},
		{"load_data",		lua_freetype_load_data	},

		{"destroy",			lua_freetype_destroy},
		
		{"size",			lua_freetype_size},
		{"glyph",			lua_freetype_glyph},
		{"render",			lua_freetype_render},
		{"tab",				lua_freetype_tab},
		{"grd",				lua_freetype_grd},
		{"info",			lua_freetype_info},

		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{"__gc",			lua_freetype_destroy},

		{0,0}
	};

	luaL_newmetatable(l, lua_freetype_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

