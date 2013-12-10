/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>


#include "../wet/util/pstdint.h"
#include "../wet/util/wet_types.h"
#include "../lib_lua/src/lua.h"
#include "../lib_lua/src/lauxlib.h"
#include "../lib_lua/src/lualib.h"

#include INCLUDE_GLES_GL

#include "code/lua_gamecake.h"


// link with lua/hacks.c plz
extern unsigned char * lua_toluserdata (lua_State *L, int idx, size_t *len);

#define SIZEOF_VB (6*5*4*256)
static unsigned char vb[SIZEOF_VB]; // vertex buffer space


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert lua vars into a C side userdata structure allocating it if we need to.
// This should be called after making changes to the lua side data in order to send
// these changes over to the C side, the C side is represented by a userdatablob in [0]
// of the lua table.
//
// Once synced you may then use this table in the C side optimised calls.
//
// This will either work or raise an error, no state is returned
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gamecake_fontdata_sync (lua_State *l)
{
struct gamecake_fontdata *ud;
size_t dlen=0;
int i;
int image=0;

	lua_rawgeti(l,1,0); // get our userdata if it already exists
	ud=(struct gamecake_fontdata *)lua_toluserdata(l,-1,&dlen);
	lua_pop(l,1);
	if(!ud) // need to allocate
	{
		dlen=sizeof(struct gamecake_fontdata);
		ud=(struct gamecake_fontdata *)lua_newuserdata(l,dlen);
		lua_rawseti(l,1,0);
	}
	
//printf("fontdata %d\n",dlen);
	
// only suport one gl image id for now...
	lua_getfield(l,1,"images");
	lua_rawgeti(l,-1,1);
	image=lua_tonumber(l,-1);
	lua_pop(l,2);

	lua_getfield(l,-1,"size"); ud->size=(float)lua_tonumber(l,-1); lua_pop(l,1);

	lua_getfield(l,1,"chars");
	
	for(i=0;i<=255;i++)
	{
		struct gamecake_fontdata_char *c=ud->chars+i;
		lua_rawgeti(l,-1,i);

		if(lua_isnil(l,-1))
		{
			lua_pop(l,1);
			lua_rawgeti(l,-1,32); // expect the space to always exist and use it as the default
		}
		
		c->image=image;
		
		lua_getfield(l,-1,"add"); c->add=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"x"  ); c->x  =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"y"  ); c->y  =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"w"  ); c->w  =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"h"  ); c->h  =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"u1" ); c->u1 =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"u2" ); c->u2 =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"v1" ); c->v1 =(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,-1,"v2" ); c->v2 =(float)lua_tonumber(l,-1); lua_pop(l,1);
		
		lua_pop(l,1);
	}
	lua_pop(l,1);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// font sync function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gamecake_canvas_font_sync (lua_State *l)
{
struct gamecake_canvas_font *ud;
size_t dlen=0;
int i;

	lua_rawgeti(l,1,0); // get our userdata if it already exists
	ud=(struct gamecake_canvas_font *)lua_toluserdata(l,-1,&dlen);
	lua_pop(l,1);
	if(!ud) // need to allocate
	{
		dlen=sizeof(struct gamecake_canvas_font);
		ud=(struct gamecake_canvas_font *)lua_newuserdata(l,dlen);
		lua_rawseti(l,1,0);
//printf("font %d\n",dlen);
	}
	
	lua_getfield(l,1,"dat"); // fontdata
	lua_rawgeti(l,-1,0);
	ud->fontdata=(struct gamecake_fontdata *)lua_toluserdata(l,-1,0);
	lua_pop(l,2);
		
	lua_getfield(l,1,"x"   ); ud->x   =(float)lua_tonumber(l,-1); lua_pop(l,1);
	lua_getfield(l,1,"y"   ); ud->y   =(float)lua_tonumber(l,-1); lua_pop(l,1);
	lua_getfield(l,1,"size"); ud->size=(float)lua_tonumber(l,-1); lua_pop(l,1);
	lua_getfield(l,1,"add" ); ud->add =(float)lua_tonumber(l,-1); lua_pop(l,1);

	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// font draw function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gamecake_canvas_font_draw (lua_State *l)
{
struct gamecake_canvas_font *ud;
int i;
unsigned char *buf=0;
int len=0;

unsigned char *s;
unsigned char c;
struct gamecake_fontdata_char *ch;

float x;
float y;
float size;
float add;

float vx,vxp,vy,vyp;

float *fp;
int fp_len;

int vid,tid;

// hacks
	vid=(int)luaL_checknumber(l,3);
	tid=(int)luaL_checknumber(l,4);

	lua_rawgeti(l,1,0); // get our userdata if it already exists
	ud=(struct gamecake_canvas_font *)lua_toluserdata(l,-1,0);
	lua_pop(l,1);
	
	s=(unsigned char *)luaL_checkstring(l,2);
	
	buf=vb;
	len=SIZEOF_VB;
	
	fp=(float *)buf;
	fp_len=0;
	
	size=ud->size/ud->fontdata->size;
	add=ud->add;
	x=ud->x;
	y=ud->y;

	while(c=*s++)
	{

		ch=ud->fontdata->chars+c;
		
		vx=x+(ch->x*size);
		vxp=ch->w*size;
		vy=y+(ch->y*size);
		vyp=ch->h*size;
		
//printf("%d %1.1s (%f,%f) (%f,%f) \n",c,s-1,ch->x,ch->y,ch->u1,ch->v1);

//		fp[(0*5)+0]=vx;		fp[(0*5)+1]=vy;		fp[(0*5)+2]=0;		fp[(0*5)+3]=ch->u1;		fp[(0*5)+4]=ch->v1;

		fp[(0*5)+0]=vx;		fp[(0*5)+1]=vy;		fp[(0*5)+2]=0;		fp[(0*5)+3]=ch->u1;		fp[(0*5)+4]=ch->v1;
		fp[(1*5)+0]=vx+vxp;	fp[(1*5)+1]=vy;		fp[(1*5)+2]=0;		fp[(1*5)+3]=ch->u2;		fp[(1*5)+4]=ch->v1;
		fp[(2*5)+0]=vx;		fp[(2*5)+1]=vy+vyp;	fp[(2*5)+2]=0;		fp[(2*5)+3]=ch->u1;		fp[(2*5)+4]=ch->v2;

		fp[(3*5)+0]=vx;		fp[(3*5)+1]=vy+vyp;	fp[(3*5)+2]=0;		fp[(3*5)+3]=ch->u1;		fp[(3*5)+4]=ch->v2;
		fp[(4*5)+0]=vx+vxp;	fp[(4*5)+1]=vy;		fp[(4*5)+2]=0;		fp[(4*5)+3]=ch->u2;		fp[(4*5)+4]=ch->v1;
		fp[(5*5)+0]=vx+vxp;	fp[(5*5)+1]=vy+vyp;	fp[(5*5)+2]=0;		fp[(5*5)+3]=ch->u2;		fp[(5*5)+4]=ch->v2;

//		fp[(5*5)+0]=vx+vxp;	fp[(5*5)+1]=vy+vyp;	fp[(5*5)+2]=0;		fp[(5*5)+3]=ch->u2;		fp[(5*5)+4]=ch->v2;

		fp+=6*5;
		fp_len+=6*5;
		
		x=x+(ch->add*size)+add;

		if( (fp_len+(6*5))*4 >= len)
		{
			break;
		}
	}

// assume stuff has been setup

//	glBufferData(GL_ARRAY_BUFFER,fp_len*4,(float *)buf,GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER,fp_len*4,(float *)buf,GL_STREAM_DRAW);

	glVertexAttribPointer(vid,3,GL_FLOAT,GL_FALSE,5*4,(void*)(0));
	glEnableVertexAttribArray(vid);
	
	glVertexAttribPointer(tid,2,GL_FLOAT,GL_FALSE,5*4,(void*)(3*4));
	glEnableVertexAttribArray(tid);

//	printf(" %d ",fp_len/5);
	glDrawArrays(GL_TRIANGLES,0,fp_len/5);
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_gamecake_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"fontdata_sync",			lua_gamecake_fontdata_sync},

		{"canvas_font_sync",		lua_gamecake_canvas_font_sync},
		{"canvas_font_draw",		lua_gamecake_canvas_font_draw},


		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

