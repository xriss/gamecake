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

static int core_setup(lua_State *l)
{
	struct fenestra *fenestra = (struct fenestra *)lua_touserdata(l, 1 );

	fenestra->ogl->setup(fenestra);

	lua_pushlightuserdata(l,fenestra->ogl);
	
	lua_pushstring(l,(const char*)glGetString(GL_VERSION));

	return 2;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->clean();

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_getset(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

const char *s=lua_tostring(l,2);
f32 f;
bool b;
	if( strcmp(s,"width")==0 )
	{
		lua_pushnumber(l,core->width);
		return 1;
	}
	else
	if( strcmp(s,"height")==0 )
	{
		lua_pushnumber(l,core->height);
		return 1;
	}
	else
	if( strcmp(s,"force_diffuse")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_diffuse=(u32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_diffuse);
		return 1;
	}
	else
	if( strcmp(s,"force_spec")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_spec=(u32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_spec);
		return 1;
	}
	else
	if( strcmp(s,"force_gloss")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_gloss=(f32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_gloss);
		return 1;
	}
/*
	else
	if( strcmp(s,"blend_sub")==0 )
	{
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		
glBlendFunc(GL_SRC_ALPHA,GL_ONE);
glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

	}
	else
	if( strcmp(s,"blend_add")==0 )
	{
		glBlendEquation(GL_FUNC_ADD);
	}
*/
	

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
/*
static int core_test(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );


	struct XOX0_header *head = (struct XOX0_header *)lua_touserdata(l, 2 );

	core->test(head->data());

	return 0;
}
*/
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_draw_cube(lua_State *l)
{
float f;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	f=(float)lua_tonumber(l,2);
	
	core->draw_cube(f);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
s32 w=0;
s32 h=0;

	if( lua_isnumber(l,2) )
	{
		w=lua_tonumber(l,2);
	}
	
	if( lua_isnumber(l,3) )
	{
		h=lua_tonumber(l,3);
	}
	
	core->begin(w,h);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clip2d(lua_State *l)
{
float xp;
float yp;
float xh;
float yh;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	xp=(float)lua_tonumber(l,2);
	yp=(float)lua_tonumber(l,3);
	xh=(float)lua_tonumber(l,4);
	yh=(float)lua_tonumber(l,5);

	core->clip2d(xp,yp,xh,yh);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_project23d(lua_State *l)
{
float aspect;
float fov;
float depth;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	aspect=(float)lua_tonumber(l,2);
	fov=(float)lua_tonumber(l,3);
	depth=(float)lua_tonumber(l,4);

	core->project23d(aspect,fov,depth);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_swap(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->swap();
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_target(lua_State *l)
{
int w,h,r;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	w=(int)lua_tonumber(l,2);
	h=(int)lua_tonumber(l,3);

	r=core->set_target(w,h);
	
	lua_pushnumber(l,r);
	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->debug_font_start();

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_end(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->debug_font_end();
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_print(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"x");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_x=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"y");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_y=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"color");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_color=(u32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"s");
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
		}
		lua_pop(l,1);
	}
	else
	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}
	
	if(s)
	{
		core->debug_font_draw_string(s);
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_print_alt(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"x");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_x=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"y");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_y=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"color");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_color=(u32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"s");
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
		}
		lua_pop(l,1);
	}
	else
	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}
	
	if(s)
	{
		core->debug_font_draw_string_alt(s);
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_rect(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	f32 x1=(f32)lua_tonumber(l,2);
	f32 y1=(f32)lua_tonumber(l,3);
	
	f32 x2=(f32)lua_tonumber(l,4);
	f32 y2=(f32)lua_tonumber(l,5);

	u32 color=(u32)lua_tonumber(l,6);
	
	core->debug_rect(x1,y1,x2,y2,color);
			
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_polygon_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	core->debug_polygon_begin();
			
	return 0;
}
static int core_polygon_vertex(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	f32 x=(f32)lua_tonumber(l,2);
	f32 y=(f32)lua_tonumber(l,3);
	u32 color=(u32)lua_tonumber(l,4);
	
	core->debug_polygon_vertex(x,y,color);
			
	return 0;
}
static int core_polygon_end(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	core->debug_polygon_end();
			
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_xox_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX0_header *xox_head = (struct XOX0_header *)lua_touserdata(l, 2 );
	struct XOX0_info   *xox_info = xox_head->data();
	
	struct XOX *xox = (struct XOX *)lua_newuserdata(l, core->xox_sizeof(xox_info) );
	
	if(xox)
	{
		core->xox_setup(xox,xox_info);
	}
			
	return 1;
}
static int core_xox_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		core->xox_clean(xox);
	}
			
	return 0;
}
static int core_xox_draw(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		core->xox_update(xox);
		core->xox_draw(xox);
	}
			
	return 0;
}
static int core_xox_get(lua_State *l)
{
XOX_surface *surf;
int i;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	lua_newtable(l);

	if(xox)
	{
		lua_newtable(l);
		
		for( surf=xox->surfaces , i=1 ; surf < xox->surfaces + xox->numof_surfaces ; surf++ , i++ )
		{
			lua_newtable(l);
			
			lua_pushstring(l,surf->name);
			lua_setfield(l,-2,"name");
			
			lua_pushnumber(l,surf->argb);
			lua_setfield(l,-2,"argb");
			
			lua_pushnumber(l,surf->spec);
			lua_setfield(l,-2,"spec");
			
			lua_pushnumber(l,surf->gloss);
			lua_setfield(l,-2,"gloss");
			
			lua_rawseti(l,-2,i);
		}
		
		lua_setfield(l,-2,"surfaces");
	}
			
	return 1;
}
static int core_xox_set(lua_State *l)
{
XOX_surface *surf;
int i;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		lua_getfield(l,3,"surfaces");
		
		for( surf=xox->surfaces , i=1 ; surf < xox->surfaces + xox->numof_surfaces ; surf++ , i++ )
		{
			lua_rawgeti(l,-1,i);
			
			lua_getfield(l,-1,"argb");
			surf->argb=(u32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_getfield(l,-1,"spec");
			surf->spec=(u32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_getfield(l,-1,"gloss");
			surf->gloss=(f32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
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
// read pixels baby
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_readpixels(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	int width=(int)(core->width);
	int height=(int)(core->height);
	int size=width*height*4;

	void *dat=malloc(size);
	if(dat)
	{
		glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, dat);
		
		lua_pushnumber(l,width);
		lua_pushnumber(l,height);
		lua_pushlstring(l,(const char *)dat,size);
		
		free(dat);
		
		return 3;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_fbo_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
		
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_newuserdata(l, sizeof(fogl_fbo) );
	
	fbo->width=(s32)lua_tonumber(l,2);
	fbo->height=(s32)lua_tonumber(l,3);
	
	if(fbo)
	{
		core->fbo_setup(fbo);
	}
			
	return 1;
}
static int core_fbo_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	if(fbo)
	{
		core->fbo_clean(fbo);
	}
			
	return 0;
}
static int core_fbo_bind(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	core->fbo_bind(fbo); // call even if fbo is nil
	
	return 0;
}
static int core_fbo_texture(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	if(fbo)
	{
		core->fbo_texture(fbo);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	{"setup",					core_setup},
	{"clean",					core_clean},
	{"begin",					core_begin},
	{"clip2d",					core_clip2d},
	{"project23d",				core_project23d},
	{"swap",					core_swap},
	{"target",					core_target},
	{"get",						core_getset},
	{"set",						core_getset},
	{"readpixels",				core_readpixels},
	
	{"fbo_setup",				core_fbo_setup},
	{"fbo_clean",				core_fbo_clean},
	{"fbo_bind",				core_fbo_bind},
	{"fbo_texture",				core_fbo_texture},

	{"draw_cube",				core_draw_cube},
	
	{"debug_begin",				core_debug_begin},
	{"debug_print",				core_debug_print},
	{"debug_rect",				core_debug_rect},
	{"debug_polygon_begin",		core_polygon_begin},
	{"debug_polygon_vertex",	core_polygon_vertex},
	{"debug_polygon_end",		core_polygon_end},
	{"debug_end",				core_debug_end},
	
	{"debug_print_alt",			core_debug_print_alt},
	
	{"xox_setup",				core_xox_setup},
	{"xox_clean",				core_xox_clean},
	{"xox_draw",				core_xox_draw},
	{"xox_get",					core_xox_get},
	{"xox_set",					core_xox_set},
	
	{"xsx_setup",				core_xsx_setup},
	{"xsx_clean",				core_xsx_clean},
	{"xsx_draw",				core_xsx_draw},
	{"xsx_get",					core_xsx_get},
	{"xsx_set",					core_xsx_set},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_fenestra_core_ogl (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);

	return 1;
}

