/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- Time And Relative Dimensions In Space
--
-- Best, 3dmathlib, name, ever.
--
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "../wet/util/wet_types.h"
#include "../lib_lua/src/lua.h"
#include "../lib_lua/src/lauxlib.h"
#include "../lib_lua/src/lualib.h"



// link with lua/hacks.c plz
extern unsigned char * lua_toluserdata (lua_State *L, int idx, size_t *len);



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Use lua to allocate some memory, lua pushes this userdata on the stack and we return an aligned pointer
// This aligned pointer may, or maynot be the same as the userdata so be careful
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
// this is probably enough alignment to make things go fast, does anything need more?
#define UDALIGN 16

// force aligned pointer from a possibly non aligned one (safe to call again)
static unsigned char * lua_tardis_uda_ptr_align (unsigned char *p)
{
	return (unsigned char *)(((intptr_t)(p-1+UDALIGN))&(-UDALIGN)); // force alignment
}

// allocated aligned pointer and push its userdata on the lua stack
static unsigned char * lua_tardis_uda_alloc (lua_State *l, int size)
{
	return lua_tardis_uda_ptr_align((unsigned char *)lua_newuserdata(l,size+UDALIGN-1)); // include a bit of extra buffer
}

// get aligned pointer form a userdata at idx
static unsigned char * lua_tardis_uda (lua_State *l, int idx)
{
	return lua_tardis_uda_ptr_align(lua_touserdata(l,idx));
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate some aligned memory on thelua stack, use lua_tardis_uda to get this ptr again
//
// probably best to ask for multiples of 16 but this is not enforced
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_alloc (lua_State *l)
{
	int size=luaL_checknumber(l,1);
	lua_tardis_uda_alloc(l,size);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get a light userdata built from a userdata (or another light userdata) by adding a number to it
//
// EG allocate an array then use this to pull out a pointer to a single item.
//
// please be careful not to be deleting the orignal userdata whilst you use this lightuserdata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_ptr (lua_State *l)
{
	unsigned char *p=lua_tardis_uda(l,1);
	int a=luaL_checknumber(l,2);
	lua_pushlightuserdata(l,p+a);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set some members, please not to supply more numbers than there is memory as no bounds checking is performed
// also only a maximum of 16 numbers may be set at once
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_set (lua_State *l)
{
int len;
int i;
float *fa=(float *)lua_tardis_uda(l,1);
float *fb;

	if(lua_istable(l,2))
	{
		for(i=0;i<16;i++)
		{
			lua_rawgeti(l,2,i+1);
			if(lua_isnil(l,-1)) { lua_pop(l,1); break; }
			fa[i]=(float)lua_tonumber(l,-1);
			lua_pop(l,1);
		}
	}
	else
	if(lua_isuserdata(l,2))
	{
		lua_toluserdata(l,2,&len);
		fb=(float *)lua_tardis_uda(l,2);
		for(i=0;i<((len+1-UDALIGN)/4);i++)
		{
			fa[i]=fb[i];
		}
	}
	else
	{
		for(i=0;i<16;i++)
		{
			if(lua_isnil(l,2+i)) { break; }
			fa[i]=(float)lua_tonumber(l,2+i);
		}
	}
	return 0;
}

static int lua_tardis_read (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
int i=(int)lua_tonumber(l,2);
	lua_pushnumber(l,fa[i-1]);
	return 1;
}

static int lua_tardis_write (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
int i=(int)lua_tonumber(l,2);
	fa[i-1]=(float)lua_tonumber(l,3);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a new m4 and set its members
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_new_m4 (lua_State *l)
{
	lua_tardis_uda_alloc(l,4*16);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a new v4 and set its members
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_new_v4 (lua_State *l)
{
	lua_tardis_uda_alloc(l,4*4);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a new v4 and set its members
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void raw_tardis_m4_product_m4(float *fa,float *fb,float *fc)
{
float r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16;

	if(!fc)
	{
		fc=fb;
	}
	
	r1 = (fa[   0]*fb[   0]) + (fa[   1]*fb[ 4+0]) + (fa[   2]*fb[ 8+0]) + (fa[   3]*fb[12+0]);
	r2 = (fa[   0]*fb[   1]) + (fa[   1]*fb[ 4+1]) + (fa[   2]*fb[ 8+1]) + (fa[   3]*fb[12+1]);
	r3 = (fa[   0]*fb[   2]) + (fa[   1]*fb[ 4+2]) + (fa[   2]*fb[ 8+2]) + (fa[   3]*fb[12+2]);
	r4 = (fa[   0]*fb[   3]) + (fa[   1]*fb[ 4+3]) + (fa[   2]*fb[ 8+3]) + (fa[   3]*fb[12+3]);
	r5 = (fa[ 4+0]*fb[   0]) + (fa[ 4+1]*fb[ 4+0]) + (fa[ 4+2]*fb[ 8+0]) + (fa[ 4+3]*fb[12+0]);
	r6 = (fa[ 4+0]*fb[   1]) + (fa[ 4+1]*fb[ 4+1]) + (fa[ 4+2]*fb[ 8+1]) + (fa[ 4+3]*fb[12+1]);
	r7 = (fa[ 4+0]*fb[   2]) + (fa[ 4+1]*fb[ 4+2]) + (fa[ 4+2]*fb[ 8+2]) + (fa[ 4+3]*fb[12+2]);
	r8 = (fa[ 4+0]*fb[   3]) + (fa[ 4+1]*fb[ 4+3]) + (fa[ 4+2]*fb[ 8+3]) + (fa[ 4+3]*fb[12+3]);
	r9 = (fa[ 8+0]*fb[   0]) + (fa[ 8+1]*fb[ 4+0]) + (fa[ 8+2]*fb[ 8+0]) + (fa[ 8+3]*fb[12+0]);
	r10= (fa[ 8+0]*fb[   1]) + (fa[ 8+1]*fb[ 4+1]) + (fa[ 8+2]*fb[ 8+1]) + (fa[ 8+3]*fb[12+1]);
	r11= (fa[ 8+0]*fb[   2]) + (fa[ 8+1]*fb[ 4+2]) + (fa[ 8+2]*fb[ 8+2]) + (fa[ 8+3]*fb[12+2]);
	r12= (fa[ 8+0]*fb[   3]) + (fa[ 8+1]*fb[ 4+3]) + (fa[ 8+2]*fb[ 8+3]) + (fa[ 8+3]*fb[12+3]);
	r13= (fa[12+0]*fb[   0]) + (fa[12+1]*fb[ 4+0]) + (fa[12+2]*fb[ 8+0]) + (fa[12+3]*fb[12+0]);
	r14= (fa[12+0]*fb[   1]) + (fa[12+1]*fb[ 4+1]) + (fa[12+2]*fb[ 8+1]) + (fa[12+3]*fb[12+1]);
	r15= (fa[12+0]*fb[   2]) + (fa[12+1]*fb[ 4+2]) + (fa[12+2]*fb[ 8+2]) + (fa[12+3]*fb[12+2]);
	r16= (fa[12+0]*fb[   3]) + (fa[12+1]*fb[ 4+3]) + (fa[12+2]*fb[ 8+3]) + (fa[12+3]*fb[12+3]);

	fc[ 0]=r1;	fc[ 1]=r2;	fc[ 2]=r3;	fc[ 3]=r4;
	fc[ 4]=r5;	fc[ 5]=r6;	fc[ 6]=r7;	fc[ 7]=r8;
	fc[ 8]=r9;	fc[ 9]=r10;	fc[10]=r11;	fc[11]=r12;
	fc[12]=r13;	fc[13]=r14;	fc[14]=r15;	fc[15]=r16;
}

static int lua_tardis_m4_product_m4 (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
float *fb=(float *)lua_tardis_uda(l,2);
float *fc=(float *)lua_tardis_uda(l,3);

	raw_tardis_m4_product_m4(fa,fb,fc);

	return 0;
}

static int lua_tardis_m4_rotate (lua_State *l)
{
const char *str;
float *fa;
float fb[16];
float *fc;

float *v;
float vv[3];

float c,cc,s,x,y,z,d,dd;

float degrees;


	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,4);
	if(!fc) { fc=fa; }
	
	if(lua_isuserdata(l,3))
	{
		v=(float *)lua_tardis_uda(l,3);
		x=v[0];
		y=v[1];
		z=v[2];
	}
	else
	if(lua_isstring(l,3))
	{
		str=lua_tostring(l, 3);
		if(str[0]=='x')
		{
			x=1.0f;y=0.0f;z=0.0f;
		}
		else
		if(str[0]=='y')
		{
			x=0.0f;y=1.0f;z=0.0f;
		}
		else // just assume z
		{
			x=0.0f;y=0.0f;z=1.0f;
		}
	}
	else // table
	{
		luaL_checktype(l, 3, LUA_TTABLE);
		lua_rawgeti(l,3,1); x=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,3,2); y=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,3,3); z=(float)lua_tonumber(l,-1); lua_pop(l,1);
	}

	c=cosf(degrees*((float)(M_PI/180.0f)));
	cc=1-c;
	s=sinf(degrees*((float)(M_PI/180.0f)));
		
	// make sure we have a unit vector
	dd=x*x + y*y + z*z;
	if( dd < (1023.0f/1024.0f) || dd > (1025.0f/1024.0f) )
	{
		d=sqrtf(dd);
		x=x/d;
		y=y/d;
		z=z/d;
	}

	fb[ 0]=x*x*cc+c;	fb[ 1]=x*y*cc-z*s;	fb[ 2]=x*z*cc+y*s;	fb[ 3]=0.0f;
	fb[ 4]=x*y*cc+z*s;	fb[ 5]=y*y*cc+c;	fb[ 6]=y*z*cc-x*s;	fb[ 7]=0.0f;
	fb[ 8]=x*z*cc-y*s;	fb[ 9]=y*z*cc+x*s;	fb[10]=z*z*cc+c;	fb[11]=0.0f;
	fb[12]=0.0f;		fb[13]=0.0f;		fb[14]=0.0f;		fb[15]=1.0f;

	raw_tardis_m4_product_m4(fa,fb,fc);

	return 0;
}

static int lua_tardis_m4_scale_v3 (lua_State *l)
{
float *fa;
float *fb;
float *fc;
float x,y,z;

	if(lua_isuserdata(l,2))
	{
		fb=(float *)lua_tardis_uda(l,2);
		x=fb[0];
		y=fb[1];
		z=fb[2];
	}
	else
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		lua_rawgeti(l,2,1); x=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,2); y=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,3); z=(float)lua_tonumber(l,-1); lua_pop(l,1);
	}

	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,3);
	if(!fc) { fc=fa; }

	fc[ 0]=x*fa[ 0];		fc[ 1]=x*fa[ 1];		fc[ 2]=x*fa[ 2];		fc[ 3]=x*fa[ 3];
	fc[ 4]=y*fa[ 4];		fc[ 5]=y*fa[ 5];		fc[ 6]=y*fa[ 6];		fc[ 7]=y*fa[ 7];
	fc[ 8]=z*fa[ 8];		fc[ 9]=z*fa[ 9];		fc[10]=z*fa[10];		fc[11]=z*fa[11];
	fc[12]=fa[12];			fc[13]=fa[13];			fc[14]=fa[14];			fc[15]=fa[15];

	return 0;
}

static int lua_tardis_m4_translate (lua_State *l)
{
float *fa;
float *fb;
float *fc;
float x,y,z;

	if(lua_isuserdata(l,2))
	{
		fb=(float *)lua_tardis_uda(l,2);
		x=fb[0];
		y=fb[1];
		z=fb[2];
	}
	else
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		lua_rawgeti(l,2,1); x=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,2); y=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,3); z=(float)lua_tonumber(l,-1); lua_pop(l,1);
	}

	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,3);
	if(!fc) { fc=fa; }

	if(fa==fc)
	{
		fc[12]+=x;
		fc[13]+=y;
		fc[14]+=z;
	}
	else
	{
		fc[ 1]=fa[ 1];
		fc[ 2]=fa[ 2];
		fc[ 3]=fa[ 3];
		fc[ 4]=fa[ 4];
		fc[ 5]=fa[ 5];
		fc[ 6]=fa[ 6];
		fc[ 7]=fa[ 7];
		fc[ 8]=fa[ 8];
		fc[ 9]=fa[ 9];
		fc[10]=fa[10];
		fc[11]=fa[11];
		fc[12]=fa[12]+x;
		fc[13]=fa[13]+y;
		fc[14]=fa[14]+z;
		fc[15]=fa[15];
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_tardis_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"alloc",					lua_tardis_alloc},
		{"ptr",						lua_tardis_ptr},
		{"set",						lua_tardis_set},
		{"read",					lua_tardis_read},
		{"write",					lua_tardis_write},

		{"new_m4",					lua_tardis_new_m4},		
		{"new_v4",					lua_tardis_new_v4},

		{"m4_product_m4",			lua_tardis_m4_product_m4},
		{"m4_rotate",				lua_tardis_m4_rotate},
		{"m4_scale_v3",				lua_tardis_m4_scale_v3},
		{"m4_translate",			lua_tardis_m4_translate},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

