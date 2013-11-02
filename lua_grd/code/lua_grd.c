/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_grd_ptr_name="grd*ptr";


// the data pointer we are using

typedef struct grd * part_ptr ;

// pull in a hack
//extern "C" 
u8 * lua_toluserdata (lua_State *L, int idx, size_t *len);


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_grd_get_ptr (lua_State *l, int idx)
{
part_ptr *p=0;

	p = ((part_ptr *)luaL_checkudata(l, idx , lua_grd_ptr_name));

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua_grd_check but with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_grd_check_ptr (lua_State *l, int idx)
{
part_ptr *p=lua_grd_get_ptr(l,idx);

	if (*p == 0)
	{
		luaL_error(l, "bad grd userdata" );
	}

	return *p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_getinfo (lua_State *l, part_ptr p, int tab)
{
	if(p)
	{
		if(p->cmap->data)
		{
			lua_pushliteral(l,"cmap");		lua_pushlightuserdata(l,p->cmap->data);		lua_rawset(l,tab);
		}
		lua_pushliteral(l,"data");		lua_pushlightuserdata(l,p->bmap->data);		lua_rawset(l,tab);

		lua_pushliteral(l,"format");	lua_pushnumber(l,p->bmap->fmt);		lua_rawset(l,tab);

		lua_pushliteral(l,"width");		lua_pushnumber(l,p->bmap->w);		lua_rawset(l,tab);
		lua_pushliteral(l,"height");	lua_pushnumber(l,p->bmap->h);		lua_rawset(l,tab);
		lua_pushliteral(l,"depth");		lua_pushnumber(l,p->bmap->d);		lua_rawset(l,tab);

		lua_pushliteral(l,"err");
		if(p->err) 	{ lua_pushstring(l,p->err); }
		else		{ lua_pushnil(l); }
		lua_rawset(l,tab);
	}
	else
	{
		lua_pushliteral(l,"err"); lua_pushstring(l,"unbound grd"); lua_rawset(l,tab);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns userdata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr * lua_grd_create_ptr(lua_State *l)
{
part_ptr *p;
	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	(*p)=0;
	luaL_getmetatable(l, lua_grd_ptr_name);
	lua_setmetatable(l, -2);
	return p;
}

int lua_grd_create(lua_State *l)
{
part_ptr *p;
const char *s;
const char *opts=0;

	p=lua_grd_create_ptr(l);

	if(lua_isnumber(l,2))	// create an image of a given format and size
	{
	s32 fmt,w,h,d;

		fmt=(s32)lua_tonumber(l,1);
		w=(s32)lua_tonumber(l,2);
		h=(s32)lua_tonumber(l,3);
		d=(s32)lua_tonumber(l,4);

		(*p)=grd_create(fmt,w,h,d);
	}
	else // just make a default one
	{
		(*p)=grd_create(GRD_FMT_U8_RGBA,0,0,0);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer in table at given index
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_destroy_idx (lua_State *l, int idx)
{
part_ptr *p=lua_grd_get_ptr(l,idx);

	if(*p)
	{
		grd_free(*p);
	}
	(*p)=0;

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer in table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_destroy (lua_State *l)
{
	lua_grd_destroy_idx(l, 1);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reset the image so all data is lost, resizing it if a new format and size is given w,h,d
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_reset (lua_State *l)
{
part_ptr *p;
part_ptr new_p;
	
	new_p=0;
	
	p=lua_grd_get_ptr(l,1);

	if(lua_isnoneornil(l,1)) // just clear what we have
	{
		new_p=grd_create(GRD_FMT_U8_RGBA,0,0,0);
	}
	else	// change dimensions
	{
	s32 fmt,w,h,d;

		fmt=(s32)lua_tonumber(l,2);
		w=(s32)lua_tonumber(l,3);
		h=(s32)lua_tonumber(l,4);
		d=(s32)lua_tonumber(l,5);

		new_p=grd_create(fmt,w,h,d);
	}

	if(new_p!=0)
	{
		if(*p!=0) // free old
		{
			lua_grd_destroy_idx(l,1);
		}

		(*p)=new_p;
	}
//	lua_grd_getinfo(l,*p,1);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_load (lua_State *l)
{
const char *opts=0;

part_ptr *p;
part_ptr new_p;
const char *filename=0;
const u8 *data=0;
size_t data_len=0;
s32 fmt=0;

	new_p=0;

	p=lua_grd_get_ptr(l,1);



	if(! lua_istable(l,2) )
	{
		lua_pushstring(l,"grd load needs options table");
		lua_error(l);
	}
	
	lua_pushstring(l,"fmt");
	lua_gettable(l,2);
	if(lua_isnumber(l,-1))
	{
		fmt=lua_tonumber(l,-1);
	}
	lua_pop(l,1);
	
	lua_pushstring(l,"filename");
	lua_gettable(l,2);
	if(lua_isstring(l,-1))
	{
		filename=lua_tostring(l,-1);
	}
	lua_pop(l,1);

	lua_pushstring(l,"data");
	lua_gettable(l,2);
	if(lua_isstring(l,-1))
	{
		data=(const u8*)lua_tolstring(l,-1,&data_len);
	}
	if(lua_isuserdata(l,-1))
	{
		data=lua_toluserdata(l,-1,&data_len);
	}
	lua_pop(l,1);

	if(filename)
	{
		new_p=grd_load_file(filename,fmt);
	}
	else
	if(data)
	{
		new_p=grd_load_data(data,data_len,fmt);
	}
		
	if(new_p!=0) // loaded something
	{
		if(new_p->err)
		{
			lua_pushnil(l);
			lua_pushstring(l,new_p->err);
			grd_free(new_p);
			return 2;
		}

		if(*p!=0) // free old
		{
			lua_grd_destroy_idx(l,1);
		}

		(*p)=new_p;
	}

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_save (lua_State *l)
{
part_ptr p;
const char *s;
s32 n=0;

	p=lua_grd_check_ptr(l,1);

	s=lua_tostring(l,2);

	if(lua_isnumber(l,3))
	{
		n=lua_tonumber(l,3);
	}


	if(!s)
	{
		luaL_error(l, "no file name specified" );
	}
	
	if( p->bmap->w<1 )
	{
		luaL_error(l, "image has 0 width" );
	}
	if( p->bmap->h<1 )
	{
		luaL_error(l, "image has 0 height" );
	}
	if( p->bmap->d<1 )
	{
		luaL_error(l, "image has 0 depth" );
	}
	
	if(! grd_save_file(p,s,n) )
	{
		lua_pushnil(l);
		lua_pushstring(l,"failed to save");
		return 2;
	}


	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy bmap (and cmap) from one grd into another (other grid must be the same size/format)
// arg2 is copied into arg1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_copy_data (lua_State *l)
{
part_ptr pa;
part_ptr pb;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	
	if(pa->bmap->fmt!=pb->bmap->fmt)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on fmt");
		return 2;
	}

	if(pa->bmap->w!=pb->bmap->w)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on width");
		return 2;
	}

	if(pa->bmap->h!=pb->bmap->h)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on height");
		return 2;
	}

	if(pa->bmap->d!=pb->bmap->d)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on depth");
		return 2;
	}
	
	grd_copy_data(pa,pb);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy bmap (and cmap) from one grd into another (other grid must be the same size/format)
// arg2 is copied into arg1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_copy_data_layer (lua_State *l)
{
int za,zb;
part_ptr pa;
part_ptr pb;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	
	if(pa->bmap->fmt!=pb->bmap->fmt)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on fmt");
		return 2;
	}

	if(pa->bmap->w!=pb->bmap->w)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on width");
		return 2;
	}

	if(pa->bmap->h!=pb->bmap->h)
	{
		lua_pushnil(l);
		lua_pushstring(l,"copy failed on height");
		return 2;
	}
	za=(int)lua_tonumber(l,3);
	zb=(int)lua_tonumber(l,4);
	
	if( (za<0) || (za>=pa->bmap->d) )
	{
		lua_pushnil(l);
		lua_pushfstring(l,"copy failed invalid destination frame %d",za);
		return 2;
	}
	if( (zb<0) || (zb>=pb->bmap->d) )
	{
		lua_pushnil(l);
		lua_pushfstring(l,"copy failed invalid source frame %d",zb);
		return 2;
	}
	
	grd_copy_data_layer(pa,pb,za,zb);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_quant (lua_State *l)
{
part_ptr p;
s32 num;

	p=lua_grd_check_ptr(l,1);

	num=(s32)lua_tonumber(l,2);
	
	if(num<2) { num=2; }
	if(num>256) { num=256; }
	
	grd_quant(p,num);

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_convert (lua_State *l)
{
part_ptr p;
s32 fmt;

	p=lua_grd_check_ptr(l,1);

	fmt=(s32)lua_tonumber(l,2);

	if(! grd_convert(p,fmt) )
	{
		lua_pushnil(l);
		lua_pushstring(l,"failed to convert");
		return 2;
	}

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}

int lua_grd_duplicate_convert (lua_State *l)
{
part_ptr p;
part_ptr *pp;
part_ptr p2;
s32 fmt;

	p=lua_grd_check_ptr(l,1);

	fmt=(s32)lua_tonumber(l,2);

	p2=grd_duplicate_convert(p,fmt);

	if(! p2 )
	{
		lua_pushnil(l);
		lua_pushstring(l,"failed to convert");
		return 2;
	}

	pp=lua_grd_create_ptr(l);
	*pp=p2;
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_blit (lua_State *l)
{
part_ptr pa;
part_ptr pb;
struct grd g[1];

s32 x;
s32 y;

s32 cx;
s32 cy;
s32 cw;
s32 ch;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	x=(s32)lua_tonumber(l,3);
	y=(s32)lua_tonumber(l,4);

	if( lua_isnumber(l,5) ) // clip the from grd
	{
		cx=(s32)lua_tonumber(l,5);
		cy=(s32)lua_tonumber(l,6);
		cw=(s32)lua_tonumber(l,7);
		ch=(s32)lua_tonumber(l,8);
		if(!grd_clip(g,pb,cx,cy,cw,ch))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
		
		if(!grd_blit(pa,g,x,y))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
	}
	else
	{
		if(!grd_blit(pa,pb,x,y))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
	}

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_paint (lua_State *l)
{
part_ptr pa;
part_ptr pb;
struct grd g[1];

s32 x;
s32 y;

s32 cx;
s32 cy;
s32 cw;
s32 ch;

s32 mode;
u32 trans;
u32 color;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	x=(s32)lua_tonumber(l,3);
	y=(s32)lua_tonumber(l,4);

	if( lua_isnumber(l,5) ) // clip the from grd, pass a false if no need to clip from
	{
		cx=(s32)lua_tonumber(l,5);
		cy=(s32)lua_tonumber(l,6);
		cw=(s32)lua_tonumber(l,7);
		ch=(s32)lua_tonumber(l,8);
		if(!grd_clip(g,pb,cx,cy,cw,ch))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
		
		mode=(s32)lua_tonumber(l,9);
		trans=(u32)lua_tonumber(l,10);
		color=(u32)lua_tonumber(l,11);
		if(!grd_paint(pa,g,x,y,mode,trans,color))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
	}
	else // simnple mode without clipping
	{
		mode=(s32)lua_tonumber(l,6);
		trans=(u32)lua_tonumber(l,7);
		color=(u32)lua_tonumber(l,8);
		if(!grd_paint(pa,pb,x,y,mode,trans,color))
		{
			lua_pushboolean(l,0);
			lua_pushstring(l,g->err);
			return 2;
		}
	}

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
/*
int lua_grd_conscale (lua_State *l)
{
part_ptr p;
f32 base;
f32 scale;

	p=lua_grd_check_ptr(l,1);

	base=(f32)lua_tonumber(l,2);
	scale=(f32)lua_tonumber(l,3);

	grd_conscale(p,base,scale);

	lua_pushboolean(l,1);
	return 1;
}
*/

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_resize (lua_State *l)
{
part_ptr p;
s32 w,h,d;

	p=lua_grd_check_ptr(l,1);

	w=(s32)lua_tonumber(l,2);
	h=(s32)lua_tonumber(l,3);
	d=(s32)lua_tonumber(l,4);

	grd_resize(p,w,h,d);

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_scale (lua_State *l)
{
part_ptr p;
s32 w,h,d;

	p=lua_grd_check_ptr(l,1);

	w=(s32)lua_tonumber(l,2);
	h=(s32)lua_tonumber(l,3);
	d=(s32)lua_tonumber(l,4);

	grd_scale(p,w,h,d);

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_shrink (lua_State *l)
{
part_ptr p;
struct grd_area gc[1];

	p=lua_grd_check_ptr(l,1);

	lua_getfield(l,2,"x");	gc->x=(s32)lua_tonumber(l,-1);	lua_pop(l,1);
	lua_getfield(l,2,"y");	gc->y=(s32)lua_tonumber(l,-1);	lua_pop(l,1);	
	lua_getfield(l,2,"z");	gc->z=(s32)lua_tonumber(l,-1);	lua_pop(l,1);
	lua_getfield(l,2,"w");	gc->w=(s32)lua_tonumber(l,-1);	lua_pop(l,1);
	lua_getfield(l,2,"h");	gc->h=(s32)lua_tonumber(l,-1);	lua_pop(l,1);	
	lua_getfield(l,2,"d");	gc->d=(s32)lua_tonumber(l,-1);	lua_pop(l,1);
	
	if( !grd_shrink(p,gc) )
	{
		lua_pushboolean(l,0);
		lua_pushstring(l,p->err);
		return 2;
	}

	lua_pushnumber(l,gc->x);	lua_setfield(l,2,"x");
	lua_pushnumber(l,gc->y);	lua_setfield(l,2,"y");
	lua_pushnumber(l,gc->z);	lua_setfield(l,2,"z");
	lua_pushnumber(l,gc->w);	lua_setfield(l,2,"w");
	lua_pushnumber(l,gc->h);	lua_setfield(l,2,"h");
	lua_pushnumber(l,gc->d);	lua_setfield(l,2,"d");

	lua_pushvalue(l,1);
	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_flipx (lua_State *l)
{
part_ptr p;
	p=lua_grd_check_ptr(l,1);
	grd_flipx(p);
	lua_pushvalue(l,1);
	return 1;
}
int lua_grd_flipy (lua_State *l)
{
part_ptr p;
	p=lua_grd_check_ptr(l,1);
	grd_flipy(p);
	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// used to get/set pixel table values
//
// works with palettes of bitmaps
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_grd_pix(lua_State *l , s32 tab_idx , struct grd_info *grd , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
{
s32 xi,yi,zi;

u8* datu8;

s32 idx;

int read_tab;

	idx=1;

// snap x,y,z w,h,d to available pixels


	if(x<0)			{	x=0;			}
	if(x>grd->w-1)	{	x=grd->w-1;		}
	if(y<0)			{	y=0;			}
	if(y>grd->h-1)	{	y=grd->h-1;		}
	if(z<0)			{	z=0;			}
	if(z>grd->d-1)	{	z=grd->d-1;		}

	if(x+w>grd->w)	{	w=grd->w-x;		}
	if(y+h>grd->h)	{	h=grd->h-y;		}
	if(z+d>grd->d)	{	d=grd->d-z;		}


	if(tab_idx==0) // just fill in, dont read
	{
		lua_newtable(l);
		tab_idx=lua_gettop(l);
		read_tab=0;
	}
	else
	{
		read_tab=1;
	}


	if( (grd->fmt==GRD_FMT_U8_RGBA) || (grd->fmt==GRD_FMT_U8_RGBA_PREMULT) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(grd,x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{

					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+0);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+1);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[1]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+2);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[2]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+3);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[3]=(u8)t;
						lua_pop(l,1);

					}
					else
					{
						lua_pushnumber(l,idx+0);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+1);
						lua_pushnumber(l,datu8[1]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+2);
						lua_pushnumber(l,datu8[2]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+3);
						lua_pushnumber(l,datu8[3]);
						lua_rawset(l,tab_idx);
					}

					datu8+=4;
					idx+=4;
				}
			}
		}
	}
	else
	if( grd->fmt==GRD_FMT_U8_RGB )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(grd,x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{

					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+1);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+2);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[1]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+3);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[2]=(u8)t;
						lua_pop(l,1);
					}
					else
					{
						lua_pushnumber(l,idx+0);
						lua_pushnumber(l,255);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+1);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+2);
						lua_pushnumber(l,datu8[1]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+3);
						lua_pushnumber(l,datu8[2]);
						lua_rawset(l,tab_idx);
					}

					datu8+=3;
					idx+=4;
				}
			}
		}
	}
	else
	if( (grd->fmt==GRD_FMT_U8_INDEXED) ||  (grd->fmt==GRD_FMT_U8_LUMINANCE) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(grd,x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{
					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+0);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);
					}
					else
					{
						lua_pushnumber(l,idx);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);
					}
					datu8+=1;
					idx+=1;
				}
			}
		}
	}
	else
	{
		luaL_error(l, "unsuported format" );
	}

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// a string version of lua_grd_pix for when you dont want to bit fiddle
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_pix_str(lua_State *l , s32 str_idx , struct grd_info *grd , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
{
int i;

s32 xi,yi,zi;

u8* datu8;
u8* bufu8;

s32 idx;

int read_tab;

size_t sl=0;
const char * s=0;
struct grd_info gb[1];

	idx=1;

// snap x,y,z w,h,d to available pixels

	if(x<0)			{	x=0;			}
	if(x>grd->w-1)	{	x=grd->w-1;		}
	if(y<0)			{	y=0;			}
	if(y>grd->h-1)	{	y=grd->h-1;		}
	if(z<0)			{	z=0;			}
	if(z>grd->d-1)	{	z=grd->d-1;		}

	if(x+w>grd->w)	{	w=grd->w-x;		}
	if(y+h>grd->h)	{	h=grd->h-y;		}
	if(z+d>grd->d)	{	d=grd->d-z;		}


	grdinfo_reset(gb);

	if(str_idx==0) // just fill in, dont read
	{
		read_tab=0;
	}
	else
	{
		s=lua_tolstring(l,str_idx,&sl);
		if(sl==0) // passed in a "" as a flag
		{
			read_tab=0;
			s=0;
		}
		else
		{
			read_tab=1;
		}
	}
	
	if(read_tab)
	{
		bufu8=(u8*)s; // read from this string
		if(sl<(grd->xscan*w*h*d))
		{
			luaL_error(l, "data string too short" );
			return 0;
		}
	}
	else // allocate a tempory buffer to write into
	{
		if(!grd_info_alloc(gb,grd->fmt,w,h,d))
		{
			luaL_error(l, "grd info alloc fail" );
			return 0;
		}
		bufu8=gb->data;
	}

	for( zi=z ; zi<z+d ; zi++ )
	{
		for( yi=y ; yi<y+h ; yi++ )
		{
			datu8=grdinfo_get_data(grd,x,yi,zi);
			if(read_tab) // read
			{
				for( i=0 ; i<w*grd->xscan ; i++ )
				{
					*datu8++=*bufu8++;
				}
			}
			else // write
			{
				for( i=0 ; i<w*grd->xscan ; i++ )
				{
					*bufu8++=*datu8++;
				}
			}
		}
	}
	if(read_tab)
	{
		return 0;
	}
	else
	{
		lua_pushlstring(l,(const char *)gb->data,gb->zscan*gb->d);
		grd_info_free(gb);
		return 1;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return palette in a table
//
// possibly requests are
//
// x,w == start , numof
//
// the palette is represented by a 1d bitmap
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_palette (lua_State *l)
{
part_ptr p;

struct grd_info *grd;

s32 x;
s32 w;

	p=lua_grd_check_ptr(l,1);
	grd=p->cmap;
//	grd_getpalinfo(p,grd);

	x=(s32)lua_tonumber(l,2);
	w= (s32)lua_tonumber(l,3);

	if(x<0)			{	luaL_error(l, "x<0" );		}
	if(x>grd->w-1)	{	luaL_error(l, "x>width" );	}
	if(x+w>grd->w)	{	luaL_error(l, "w>width" );	}

	if(lua_istable(l,4))
	{
		lua_grd_pix(l,4,grd,x,0,0,w,1,1);
		return 0;
	}
	else
	if(lua_isstring(l,4))
	{
		return lua_grd_pix_str(l,4,grd,x,0,0,w,1,1);
	}
	else
	{
		lua_grd_pix(l,0,grd,x,0,0,w,1,1);
		return 1;
	}

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return pixels in a table
//
// possibly requests are
//
// x,y,z , w,h,d
// x,y,0 , w,h,1
// x,y,0 , 1,1,1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_pixels (lua_State *l)
{
part_ptr p;

struct grd_info *grd;


s32 x,y,z;
s32 w,h,d;
s32 tab_idx;

	p=lua_grd_check_ptr(l,1);
	grd=p->bmap;
//	grd_getinfo(p,grd);

	tab_idx=0;

	if( lua_isnumber(l,7) )
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=(s32)lua_tonumber(l,4);

		w=(s32)lua_tonumber(l,5);
		h=(s32)lua_tonumber(l,6);
		d=(s32)lua_tonumber(l,7);
		if( lua_istable(l,8) || lua_isstring(l,8) ) { tab_idx=8; }
	}
	else
	if( lua_isnumber(l,5) )
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=0;

		w=(s32)lua_tonumber(l,4);
		h=(s32)lua_tonumber(l,5);
		d=1;
		if( lua_istable(l,6) || lua_isstring(l,6) ) { tab_idx=6; }
	}
	else
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=0;

		w=1;
		h=1;
		d=1;
		if( lua_istable(l,4) || lua_isstring(l,4) ) { tab_idx=4; }
	}


// error on out of range


	if(x<0)			{	luaL_error(l, "x<0" );		}
	if(x>grd->w-1)	{	luaL_error(l, "x>width" );	}
	if(y<0)			{	luaL_error(l, "y<0" );		}
	if(y>grd->h-1)	{	luaL_error(l, "y>height" );	}
	if(z<0)			{	luaL_error(l, "z<0" );		}
	if(z>grd->d-1)	{	luaL_error(l, "z>depth" );	}

	if(x+w>grd->w)	{	luaL_error(l, "w>width" );	}
	if(y+h>grd->h)	{	luaL_error(l, "h>height" );	}
	if(z+d>grd->d)	{	luaL_error(l, "d>depth" );	}


	if( (tab_idx>0) && lua_isstring(l,tab_idx) )
	{
		return lua_grd_pix_str(l,tab_idx,grd,x,y,z,w,h,d);
	}
	else
	{
		lua_grd_pix(l,tab_idx,grd,x,y,z,w,h,d);
	}
	
	if(tab_idx) { return 0; }
	else		{ return 1; }
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_clear (lua_State *l)
{
part_ptr p;
u32 v;

	p=lua_grd_check_ptr(l,1);

	v=(u32)lua_tonumber(l,2);

	grd_clear(p,v);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the provided table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_info (lua_State *l)
{
part_ptr p;

	p=lua_grd_check_ptr(l,1);
	lua_grd_getinfo(l,p,2);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_wetgenes_grd_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"create"		,	lua_grd_create	},
		
		{"destroy",			lua_grd_destroy},

		{"info",			lua_grd_info},

		{"reset",			lua_grd_reset},
		{"load",			lua_grd_load},
		{"save",			lua_grd_save},

		{"copy_data",		lua_grd_copy_data},
		{"copy_data_layer",	lua_grd_copy_data_layer},

		{"convert",			lua_grd_convert},
		
		{"quant",			lua_grd_quant},

		{"pixels",			lua_grd_pixels},
		{"palette",			lua_grd_palette},

//		{"conscale",		lua_grd_conscale},

		{"scale",			lua_grd_scale},
		{"resize",			lua_grd_resize},
			
		{"flipx",			lua_grd_flipx},
		{"flipy",			lua_grd_flipy},
		{"blit",			lua_grd_blit},
		
		{"paint",			lua_grd_paint},

		{"shrink",			lua_grd_shrink},
		
		{"clear",			lua_grd_clear},

		{0,0}
	};

	const luaL_reg meta[] =
	{
		{"__gc",			lua_grd_destroy},

		{0,0}
	};

	luaL_newmetatable(l, lua_grd_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

