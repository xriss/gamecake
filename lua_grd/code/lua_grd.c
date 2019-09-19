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
extern u8 * lua_toluserdata (lua_State *L, int idx, size_t *len);
extern void * luaL_wetestudata(lua_State *L, int index, const char *tname);


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_grd_get_ptr (lua_State *l, int idx)
{
part_ptr *p=0;

	p = ((part_ptr *)luaL_wetestudata(l, idx , lua_grd_ptr_name));

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

	if( (p == 0) || (*p == 0) )
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
			lua_pushliteral(l,"colors");	lua_pushnumber(l,p->cmap->w);				lua_rawset(l,tab);
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

	if(p)
	{
		if(*p)
		{
			grd_free(*p);
		}
		(*p)=0;
	}
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
	if(!p) { lua_grd_check_ptr(l,1); } // force error

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
	if(!p) { lua_grd_check_ptr(l,1); } // force error


	if(! lua_istable(l,2) )
	{
		lua_pushstring(l,"grd load needs options table");
		lua_error(l);
	}
	
	lua_getfield(l,2,"fmt");
	if(lua_isnumber(l,-1)) { fmt=lua_tonumber(l,-1); }
	lua_pop(l,1);
	
	lua_getfield(l,2,"filename");
	if(lua_isstring(l,-1)) { filename=lua_tostring(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"data");
	if(lua_isstring(l,-1))   { data=(const u8*)lua_tolstring(l,-1,&data_len); }
	if(lua_isuserdata(l,-1)) { data=lua_toluserdata(l,-1,&data_len); }
	lua_pop(l,1);

	if(filename)
	{
		new_p=grd_load_file(filename,fmt,0);
	}
	else
	if(data)
	{
		new_p=grd_load_data(data,data_len,fmt,0);
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

		lua_pushvalue(l,1);

		if(new_p->data) // also loaded some special json+undo chunks so return that too?
		{
			lua_newtable(l);
			u32 *td=(u32*)(new_p->data);
			while( td[0] != 0 ) // null terminated
			{
				lua_pushlstring(l,(char*)(td+1),4);
				lua_pushlstring(l,(char*)(td+2),td[0]-8); // skip header8
				lua_settable(l,-3);
				td+=((td[0]+3)>>2); // next tag round size up to 4 bytes
			}

//			lua_pushstring(l,new_p->data);
			free(new_p->data);
			new_p->data=0;
			new_p->data_sizeof=0;
			return 2;
		}
		
		return 1; // just the result
	}

// should not get here

	lua_pushnil(l);
	lua_pushstring(l,"no grd allocated");
	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_save (lua_State *l)
{
part_ptr p;
const char *s=0;
s32 n=0;

size_t len;

u32 tags[16];

	tags[0]=3<<2;
	tags[1]=GRD_TAG_DEF('Q','U','A','L');
	tags[2]=100;

	tags[3]=3<<2;
	tags[4]=GRD_TAG_DEF('S','P','E','D');
	tags[5]=80; // speed in ms

	tags[6]=4<<2;
	tags[7]=GRD_TAG_DEF('J','S','O','N');
	tags[8]=0; //just a pointer but we
	tags[9]=0; //need 16 bytes for 64bit build

	tags[10]=5<<2;
	tags[11]=GRD_TAG_DEF('U','N','D','O');
	tags[12]=0; //length
	tags[13]=0; //just a pointer but we
	tags[14]=0; //need 16 bytes for 64bit build

	tags[15]=0;


	p=lua_grd_check_ptr(l,1);

	if(! lua_istable(l,2) )
	{
		lua_pushstring(l,"grd save needs options table");
		lua_error(l);
	}

	lua_getfield(l,2,"filename");
	if(lua_isstring(l,-1)) { s=lua_tostring(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"fmt");
	if(lua_isnumber(l,-1)) { n=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"quality");
	if(lua_isnumber(l,-1)) { tags[2]=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"speed");
	if(lua_isnumber(l,-1)) { tags[5]=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"json");
	if(lua_isstring(l,-1)) { *((const char**)(tags+8))=lua_tostring(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"undo");
	if(lua_isstring(l,-1)) { *((const char**)(tags+13))=lua_tolstring(l,-1,&len); tags[12]=len; }
	lua_pop(l,1);
	
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
	

	if(s)
	{
		if(! grd_save_file(p,s,n,tags) )
		{
			lua_pushnil(l);
			lua_pushstring(l,p->err ? p->err : "failed to save");
			return 2;
		}
	}
	else
	{
		struct grd_io_info d[1]={0}; d->tags=tags;
		if(! grd_save_data(p,d,n) )
		{
			if(d->data) { free(d->data); }
			lua_pushnil(l);
			lua_pushstring(l,p->err ? p->err : "failed to save");
			return 2;
		}
		if(d->data)
		{ 
			lua_pushlstring(l,(const char *)d->data,d->data_len); // just return the data string
			free(d->data);
		}
		return 1;
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
s32 dither;

	p=lua_grd_check_ptr(l,1);

	num=(s32)lua_tonumber(l,2);
	dither=(s32)lua_tonumber(l,3);
	
	if(num<2) { num=2; }
	if(num>256) { num=256; }
	
	grd_quant(p,num,dither);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_attr_redux (lua_State *l)
{
part_ptr p;
s32 bak,num,cw,ch,sub;

	p=lua_grd_check_ptr(l,1);

	cw=(s32)lua_tonumber(l,2);
	ch=(s32)lua_tonumber(l,3);
	num=(s32)lua_tonumber(l,4);
	sub=(s32)lua_tonumber(l,5);
	bak=(s32)lua_tonumber(l,6);
	
	if(num<2) { num=2; }
	if(num>256) { num=256; }

	if(bak>255) { bak=255; }
	
	grd_attr_redux(p,cw,ch,num,sub,bak);

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_remap (lua_State *l)
{
part_ptr pa;
part_ptr pb;
int colors=0; // get number of colors from palette
int dither=4;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	if(lua_isnumber(l,3)) { colors=lua_tonumber(l,3); }
	if(lua_isnumber(l,4)) { dither=lua_tonumber(l,4); }

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_RGBA)
	{
		lua_pushnil(l);
		lua_pushstring(l,"from format must be rgba");
		return 2;
	}

	if((pb->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_INDEXED)
	{
		lua_pushnil(l);
		lua_pushstring(l,"to format must be indexed");
		return 2;
	}

	if(pa->bmap->w!=pb->bmap->w)
	{
		lua_pushnil(l);
		lua_pushstring(l,"remap failed on width");
		return 2;
	}

	if(pa->bmap->h!=pb->bmap->h)
	{
		lua_pushnil(l);
		lua_pushstring(l,"remap failed on height");
		return 2;
	}
	if(pa->bmap->d!=pb->bmap->d)
	{
		lua_pushnil(l);
		lua_pushstring(l,"remap failed on depth");
		return 2;
	}
	

		
	grd_remap(pa,pb,colors,dither);

	lua_pushvalue(l,2);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_sort_cmap (lua_State *l)
{
part_ptr pa;

	pa=lua_grd_check_ptr(l,1);

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_INDEXED)
	{
		lua_pushnil(l);
		lua_pushstring(l,"format must be indexed");
		return 2;
	}
		
	grd_sort_cmap(pa);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_adjust_hsv (lua_State *l)
{
part_ptr pa;
float h,s,v;

	pa=lua_grd_check_ptr(l,1);
	h=(float)lua_tonumber(l,2);
	s=(float)lua_tonumber(l,3);
	v=(float)lua_tonumber(l,4);

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_RGBA)
	{
		lua_pushnil(l);
		lua_pushstring(l,"format must be rgba");
		return 2;
	}
		
	grd_adjust_hsv(pa,h,s,v);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_adjust_rgb (lua_State *l)
{
part_ptr pa;
float r,g,b;

	pa=lua_grd_check_ptr(l,1);
	r=(float)lua_tonumber(l,2);
	g=(float)lua_tonumber(l,3);
	b=(float)lua_tonumber(l,4);

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_RGBA)
	{
		lua_pushnil(l);
		lua_pushstring(l,"format must be rgba");
		return 2;
	}
		
	grd_adjust_rgb(pa,r,g,b);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_adjust_contrast (lua_State *l)
{
part_ptr pa;
float sub,con;

	pa=lua_grd_check_ptr(l,1);
	sub=(float)lua_tonumber(l,2);
	con=(float)lua_tonumber(l,3);

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_RGBA)
	{
		lua_pushnil(l);
		lua_pushstring(l,"format must be rgba");
		return 2;
	}
		
	grd_adjust_contrast(pa,(int)sub,con);

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

int lua_grd_create_convert (lua_State *l)
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
int lua_grd_normal (lua_State *l)
{
part_ptr p;
s32 fmt;

	p=lua_grd_check_ptr(l,1);

	if(! grd_sobelnormal(p) )
	{
		lua_pushnil(l);
		lua_pushstring(l,"failed to convert");
		return 2;
	}

//	lua_grd_getinfo(l,p,1);

	lua_pushvalue(l,1);
	return 1;
}

int lua_grd_create_normal (lua_State *l)
{
part_ptr p;
part_ptr *pp;
part_ptr p2;
s32 fmt;

	p=lua_grd_check_ptr(l,1);

	p2=grd_duplicate_sobelnormal(p);

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
int lua_grd_clip (lua_State *l)
{
part_ptr *g;
part_ptr p;
s32 cx;
s32 cy;
s32 cz;
s32 cw;
s32 ch;
s32 cd;

	p=lua_grd_check_ptr(l,1);
	cx=(s32)lua_tonumber(l,2);
	cy=(s32)lua_tonumber(l,3);
	cz=(s32)lua_tonumber(l,4);
	cw=(s32)lua_tonumber(l,5);
	ch=(s32)lua_tonumber(l,6);
	cd=(s32)lua_tonumber(l,7);

	g=lua_grd_create_ptr(l);
	(*g)=grd_create(GRD_FMT_U8_RGBA,0,0,0);

	if(!grd_clip(*g,p,cx,cy,cz,cw,ch,cd))
	{
		lua_pop(l,1); // remove result from stack and replace with error
		lua_pushboolean(l,0);
		lua_pushstring(l,(*g)->err);
		return 2;
	}
	
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

	if( lua_isnumber(l,5) ) // clip the from grd,  pass a false if no need to clip from
	{
		cx=(s32)lua_tonumber(l,5);
		cy=(s32)lua_tonumber(l,6);
		cw=(s32)lua_tonumber(l,7);
		ch=(s32)lua_tonumber(l,8);
		if(!grd_clip(g,pb,cx,cy,0,cw,ch,1))
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
		if(!grd_clip(g,pb,cx,cy,0,cw,ch,1))
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
int lua_grd_xor (lua_State *l)
{
part_ptr pd;
part_ptr pa;

	pd=lua_grd_check_ptr(l,1);
	pa=lua_grd_check_ptr(l,2);

	if( !grd_xor(pd,pa) )
	{
		lua_pushboolean(l,0);
		lua_pushstring(l,pd->err);
		return 2;
	}

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
int lua_grd_slide (lua_State *l)
{
part_ptr p;
int dx,dy,dz;

	p=lua_grd_check_ptr(l,1);
	dx=lua_tonumber(l,2);
	dy=lua_tonumber(l,3);
	dz=lua_tonumber(l,4);
	grd_slide(p,dx,dy,dz);

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
int lua_grd_pix(lua_State *l , s32 tab_idx , struct grd_info *dst , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
{
s32 xi,yi,zi;

u8* datu8;

s32 idx;

int read_tab;

	idx=1;

// snap x,y,z w,h,d to available pixels


	if(x<0)			{	w+=x;x=0;		} // clip position in destination
	if(x>dst->w-1)	{	return 0;		}
	if(y<0)			{	h+=y;y=0;		}
	if(y>dst->h-1)	{	return 0;		}
	if(z<0)			{	d+=z;z=0;		}
	if(z>dst->d-1)	{	return 0;		}

	if(x+w>dst->w)	{	w=dst->w-x;		} // clip size to destination
	if(y+h>dst->h)	{	h=dst->h-y;		}
	if(z+d>dst->d)	{	d=dst->d-z;		}
	
	if( (w<=0) || (h<=0) || (d<=0) ) { return 0; } 


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


	if( (dst->fmt==GRD_FMT_U8_RGBA) || (dst->fmt==GRD_FMT_U8_RGBA_PREMULT) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(dst,x,yi,zi);

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
	if( dst->fmt==GRD_FMT_U8_RGB )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(dst,x,yi,zi);

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
					}

					datu8+=3;
					idx+=3;
				}
			}
		}
	}
	else
	if( (dst->fmt==GRD_FMT_U8_INDEXED) ||  (dst->fmt==GRD_FMT_U8_LUMINANCE) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grdinfo_get_data(dst,x,yi,zi);

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
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// a string version of lua_grd_pix for when you dont want to bit fiddle
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_pix_str(lua_State *l , s32 str_idx , struct grd_info *dst , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
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

	if(x<0)			{	w+=x;x=0;		} // clip position in destination
	if(x>dst->w-1)	{	return 0;		}
	if(y<0)			{	h+=y;y=0;		}
	if(y>dst->h-1)	{	return 0;		}
	if(z<0)			{	d+=z;z=0;		}
	if(z>dst->d-1)	{	return 0;		}

	if(x+w>dst->w)	{	w=dst->w-x;		} // clip size to destination
	if(y+h>dst->h)	{	h=dst->h-y;		}
	if(z+d>dst->d)	{	d=dst->d-z;		}
	
	if( (w<=0) || (h<=0) || (d<=0) ) { return 0; } 


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
		if(sl<(dst->xscan*w*h*d))
		{
			luaL_error(l, "data string too short" );
			return 0;
		}
	}
	else // allocate a tempory buffer to write into
	{
		if(!grd_info_alloc(gb,dst->fmt,w,h,d))
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
			datu8=grdinfo_get_data(dst,x,yi,zi);
			if(read_tab) // read
			{
				for( i=0 ; i<w*dst->xscan ; i++ )
				{
					*datu8++=*bufu8++;
				}
			}
			else // write
			{
				for( i=0 ; i<w*dst->xscan ; i++ )
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
// a grd version of lua_grd_pix for when you dont want to deal with writing string data :)
// you can not read with this format, just write one grd into another
// use clip to limit the source grd if you dont want all of it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_pix_grd(lua_State *l , struct grd_info *src , struct grd_info *dst , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
{

s32 xi,yi,zi;
s32 xscan;

// sanity clipping, so we don't trash random memory super easily.

	if(x<0)			{	w+=x;x=0;		} // clip position in destination
	if(x>dst->w-1)	{	return 0;		}
	if(y<0)			{	h+=y;y=0;		}
	if(y>dst->h-1)	{	return 0;		}
	if(z<0)			{	d+=z;z=0;		}
	if(z>dst->d-1)	{	return 0;		}
	
	if(x+w>dst->w)	{	w=dst->w-x;		} // clip size to destination
	if(y+h>dst->h)	{	h=dst->h-y;		}
	if(z+d>dst->d)	{	d=dst->d-z;		}
	
	if( (w<=0) || (h<=0) || (d<=0) ) { return 0; } 

	if(w>src->w)	{	w=src->w-x;		} // clip size to source 
	if(h>src->h)	{	h=src->h-y;		}
	if(d>src->d)	{	d=src->d-z;		}
	
	xscan=dst->xscan; // prefer destination xscan0
	if( xscan>src->xscan ) { xscan=src->xscan; } // but also clip so we don't read from bad memory

	for( zi=0 ; zi<d ; zi++ )
	{
		for( yi=0 ; yi<h ; yi++ )
		{
			memcpy(
				grdinfo_get_data(dst,x,y+yi,z+zi), // dst
				grdinfo_get_data(src,0,yi,zi), // src
				w*xscan); // xscan deals with all formats
		}
	}
	return 0;
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

	if(!p->cmap->data) { p->err="no palette"; return 0; } // no palette
	
	if(lua_isnil(l,2)) // get palette size
	{
		lua_pushnumber(l,p->cmap->w);
		return 1;
	}

	if(lua_isnil(l,3)) // set palette size
	{
		w=(s32)lua_tonumber(l,2);
		p->cmap->w=w;
		lua_pushnumber(l,p->cmap->w);
		return 1;
	}

	x=(s32)lua_tonumber(l,2);
	w=(s32)lua_tonumber(l,3);

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
	if(lua_grd_get_ptr(l,4))
	{
		return lua_grd_pix_grd(l,lua_grd_check_ptr(l,4)->cmap,grd,x,0,0,w,1,1);
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
		if( lua_istable(l,8) || lua_isstring(l,8) || lua_grd_get_ptr(l,8) ) { tab_idx=8; }
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
		if( lua_istable(l,6) || lua_isstring(l,6) || lua_grd_get_ptr(l,6) ) { tab_idx=6; }
	}
	else
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=0;

		w=1;
		h=1;
		d=1;
		if( lua_istable(l,4) || lua_isstring(l,4) || lua_grd_get_ptr(l,4) ) { tab_idx=4; }
	}


	if( (tab_idx>0) && lua_isstring(l,tab_idx) )
	{
		return lua_grd_pix_str(l,tab_idx,grd,x,y,z,w,h,d);
	}
	else
	if( (tab_idx>0) && lua_grd_get_ptr(l,tab_idx) )
	{
		return lua_grd_pix_grd(l,lua_grd_check_ptr(l,tab_idx)->bmap,grd,x,y,z,w,h,d);
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
// save an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_stream_open (lua_State *l)
{
part_ptr p;
const char *s=0;
s32 n=0;

u32 tags[16];

	tags[0]=3<<2;
	tags[1]=GRD_TAG_DEF('Q','U','A','L');
	tags[2]=100;

	tags[3]=3<<2;
	tags[4]=GRD_TAG_DEF('S','P','E','D');
	tags[5]=80; // speed in ms

	tags[6]=4<<2;
	tags[7]=GRD_TAG_DEF('J','S','O','N');
	tags[8]=0; //just a pointer but we
	tags[9]=0; //need 16 bytes for 64bit build

	tags[10]=0;


	p=lua_grd_check_ptr(l,1);

	if(! lua_istable(l,2) )
	{
		lua_pushstring(l,"grd stream needs options table");
		lua_error(l);
	}

	lua_getfield(l,2,"filename");
	if(lua_isstring(l,-1)) { s=lua_tostring(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"fmt");
	if(lua_isnumber(l,-1)) { n=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"quality");
	if(lua_isnumber(l,-1)) { tags[2]=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"speed");
	if(lua_isnumber(l,-1)) { tags[5]=(u32)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"json");
	if(lua_isstring(l,-1)) { *((const char**)(tags+8))=lua_tostring(l,-1); }
	lua_pop(l,1);

	
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
	
	
	struct grd_io_gif *sgif = (struct grd_io_gif *)lua_newuserdata(l, sizeof(struct grd_io_gif));
	memset(sgif,0,sizeof(struct grd_io_gif)); // make sure it is all set to 0

	sgif->inf->tags=tags;
	sgif->inf->file_name=s;

	grd_gif_save_stream_open(sgif,p);

	return 1;
}

int lua_grd_stream_write(lua_State *l)
{
part_ptr p;
struct grd_io_gif *sgif;

	sgif=((struct grd_io_gif *)lua_touserdata(l, 1 ));
	p=lua_grd_check_ptr(l,2);

	grd_gif_save_stream_write(sgif,p);

	return 0;
}

int lua_grd_stream_close(lua_State *l)
{
part_ptr p;
struct grd_io_gif *sgif;

	sgif=((struct grd_io_gif *)lua_touserdata(l, 1 ));

	grd_gif_save_stream_close(sgif);

	if(sgif->inf->data)
	{ 
		lua_pushlstring(l,(const char *)sgif->inf->data,sgif->inf->data_len); // just return the data string
		free(sgif->inf->data);
		return 1;
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_fillmask (lua_State *l)
{
part_ptr pa;
part_ptr pb;
int seedx=0;
int seedy=0;

	pa=lua_grd_check_ptr(l,1);
	pb=lua_grd_check_ptr(l,2);
	if(lua_isnumber(l,3)) { seedx=lua_tonumber(l,3); }
	if(lua_isnumber(l,4)) { seedy=lua_tonumber(l,4); }

	if((pa->bmap->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_INDEXED)
	{
		lua_pushnil(l);
		lua_pushstring(l,"from format must be indexed");
		return 2;
	}

	if( ! (pb->bmap->xscan==1) )
	{
		lua_pushnil(l);
		lua_pushstring(l,"to format must be alpha");
		return 2;
	}

	if(pa->bmap->w!=pb->bmap->w)
	{
		lua_pushnil(l);
		lua_pushstring(l,"fillmask failed on width");
		return 2;
	}

	if(pa->bmap->h!=pb->bmap->h)
	{
		lua_pushnil(l);
		lua_pushstring(l,"fillmask failed on height");
		return 2;
	}
	if(pa->bmap->d!=pb->bmap->d)
	{
		lua_pushnil(l);
		lua_pushstring(l,"fillmask failed on depth");
		return 2;
	}
	

		
	grd_fillmask(pa,pb,seedx,seedy,0);

	lua_pushvalue(l,2);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_wetgenes_grd_core (lua_State *l)
{
	const luaL_Reg lib[] =
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
		{"create_convert",	lua_grd_create_convert},

		{"create_normal",	lua_grd_create_normal},
		
		{"quant",			lua_grd_quant},
		{"attr_redux",		lua_grd_attr_redux},
		{"remap",			lua_grd_remap},
		{"sort_cmap",		lua_grd_sort_cmap},

		{"adjust_hsv",		lua_grd_adjust_hsv},
		{"adjust_rgb",		lua_grd_adjust_rgb},
		{"adjust_contrast",	lua_grd_adjust_contrast},

		{"pixels",			lua_grd_pixels},
		{"palette",			lua_grd_palette},

		{"scale",			lua_grd_scale},
		{"resize",			lua_grd_resize},
			
		{"slide",			lua_grd_slide},
		{"flipx",			lua_grd_flipx},
		{"flipy",			lua_grd_flipy},
		{"blit",			lua_grd_blit},
		
		{"paint",			lua_grd_paint},

		{"fillmask",		lua_grd_fillmask},

		{"xor",				lua_grd_xor},
		{"shrink",			lua_grd_shrink},
		
		{"clear",			lua_grd_clear},
		
		{"clip",			lua_grd_clip}, // be careful with this, you must stop the master data from getting GCd

// simple API (hack) to stream gif animations one frame at a time
// don't use. may break any time now :)
// GIFs are so slow anyhow, just ended up using a series of PNG files...
		{"stream_open",		lua_grd_stream_open},
		{"stream_write",	lua_grd_stream_write},
		{"stream_close",	lua_grd_stream_close},

		{0,0}
	};

	const luaL_Reg meta[] =
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

