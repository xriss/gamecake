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

#include "wet_types.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

// link with lua/hacks.c plz
extern unsigned char * lua_toluserdata (lua_State *L, int idx, size_t *len);



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Use lua to allocate some memory, lua pushes this userdata on the stack and we return an aligned pointer
// This aligned pointer may, or maynot be the same as the userdata so be careful
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
// 16 is probably enough alignment to allow things to go fast, does anything need more?

#define UDALIGN 16

// comment out the define to turn off padding and alignment, set it to enable it
// make sure to use the functions below when dealing with this aligned data

// force aligned pointer from a possibly non aligned one (safe to call again)
static unsigned char * lua_tardis_uda_ptr_align (unsigned char *p)
{
//printf("uda %08x %08x\n",p,(unsigned char *)(((intptr_t)(p-1+UDALIGN))&(-UDALIGN)));
//	return p;
#if defined(UDALIGN)
	return (unsigned char *)(((intptr_t)(p-1+UDALIGN))&(-UDALIGN)); // force alignment
#else
	return p;
#endif
}

// allocated aligned pointer and push its userdata on the lua stack
static unsigned char * lua_tardis_uda_alloc (lua_State *l, int size)
{
#if defined(UDALIGN)
	return lua_tardis_uda_ptr_align((unsigned char *)lua_newuserdata(l,size+UDALIGN-1)); // include a bit of extra buffer
#else
	return lua_newuserdata(l,size);
#endif
}

// get aligned pointer form a userdata at idx
unsigned char * lua_tardis_uda (lua_State *l, int idx)
{
#if defined(UDALIGN)
	if(!lua_isuserdata(l,idx)) { return 0; }
	return lua_tardis_uda_ptr_align(lua_touserdata(l,idx));
#else
	return lua_touserdata(l,idx);
#endif
}

// get size of allocated data form a userdata at idx
unsigned int lua_tardis_uda_length (lua_State *l, int idx)
{
size_t len=0;
#if defined(UDALIGN)
	if(!lua_isuserdata(l,idx)) { return 0; }
	lua_toluserdata(l,idx,&len);
	if(len) // may be 0, if so do not change it
	{
		len=len+1-UDALIGN;
	}
#else
	if(!lua_isuserdata(l,idx)) { return 0; }
	lua_toluserdata(l,idx,&len);
#endif
	return len;
}

// get count of numbers in this uda
unsigned int lua_tardis_uda_count (lua_State *l, int idx)
{
	return lua_tardis_uda_length(l,idx)/4;
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
	unsigned char *p=lua_tardis_uda_alloc(l,size);
	memset(p,0,size);
	
	if(lua_istable(l,2)) // also set metatable
	{
		lua_pushvalue(l,2);
		lua_setmetatable(l,-2);
	}

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
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_set (lua_State *l)
{
int count_out;
int count;
int i;
float *fa=(float *)lua_tardis_uda(l,1);
float *fb;

	if(!fa)	{ return luaL_error(l,"Bad tardis ptr"); }
	count_out=lua_tardis_uda_count(l,1);


	if(lua_istable(l,2))
	{
		for(i=0;i<count_out;i++)
		{
			lua_rawgeti(l,2,i+1);
			if(!lua_isnumber(l,-1)) { lua_pop(l,1); break; }
			fa[i]=(float)lua_tonumber(l,-1);
			lua_pop(l,1);
		}
//printf("set %08x tab %d\n"	,fa,i);
	}
	else
	if(lua_isuserdata(l,2))
	{
		count=lua_tardis_uda_count(l,2);
		if(count>count_out){count=count_out;}
		if(lua_isnumber(l,3)) { count=(int)lua_tonumber(l,3); } // need to know how many floats to set?
		if(count< 1) { count= 1; } // sanity
//		if(count>16) { count=16; } // sanity
		fb=(float *)lua_tardis_uda(l,2);
		for(i=0;i<count;i++)
		{
			fa[i]=fb[i];
		}
//printf("set %08x uda %d\n"	,fa,i);
	}
	else
	{
		for(i=0;i<count_out;i++)
		{
			if(!lua_isnumber(l,2+i)) { break; } // stop on first nil
			fa[i]=(float)lua_tonumber(l,2+i);
		}
//printf("set %08x var %d\n"	,fa,i);
	}
	lua_pushvalue(l,1); // return passed in value for chain function calls
	return 1;
}

static int lua_tardis_read (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
int	count=lua_tardis_uda_count(l,1);
int i=(int)lua_tonumber(l,2);
	if(i<1){ lua_pushnil(l); return 1; }
	if(i>count){ lua_pushnil(l); return 1; }
	lua_pushnumber(l,fa[i-1]);
	return 1;
}

static int lua_tardis_write (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
int	count=lua_tardis_uda_count(l,1);
int i=(int)lua_tonumber(l,2);
	if(i<1){ return 0; }
	if(i>count){ return 0; }
	fa[i-1]=(float)lua_tonumber(l,3);
	return 0;
}

static int lua_tardis_compare (lua_State *l)
{
int i;
float *fa=(float *)lua_tardis_uda(l,1);
float *fb=(float *)lua_tardis_uda(l,1);
int	count=lua_tardis_uda_count(l,1);

	if(!fa)
	{
		if(!fb) // both nil, so they are equal
		{
			lua_pushboolean(l,1);
			return 1;
		}
		else // not equal
		{
			lua_pushboolean(l,0);
			return 1;
		}
	}

	if(lua_isnumber(l,3)) { count=(int)lua_tonumber(l,3); } // need to know how many floats?
	if(count< 1) { count= 1; } // sanity
//	if(count>16) { count=16; } // sanity

	for(i=0;i<count;i++)
	{
		if( fa[i] != fb[i] ) // compare failed
		{
			lua_pushboolean(l,0);
			return 1;
		}
	}

	lua_pushboolean(l,1);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a new m4 (possibly an array)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_new_m4 (lua_State *l)
{
int count=(int)lua_tonumber(l,1);
	if(count<1){count=1;}
	lua_tardis_uda_alloc(l,count*4*16);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a new v4 (possibly an array)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_new_v4 (lua_State *l)
{
int count=(int)lua_tonumber(l,1);
	if(count<1){count=1;}
	lua_tardis_uda_alloc(l,count*4*4);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void raw_tardis_m4_product_m4(float *fa,float *fb,float *fc)
{
float r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16;
	
	r1 = (fb[   0]*fa[   0]) + (fb[   1]*fa[ 4+0]) + (fb[   2]*fa[ 8+0]) + (fb[   3]*fa[12+0]);
	r2 = (fb[   0]*fa[   1]) + (fb[   1]*fa[ 4+1]) + (fb[   2]*fa[ 8+1]) + (fb[   3]*fa[12+1]);
	r3 = (fb[   0]*fa[   2]) + (fb[   1]*fa[ 4+2]) + (fb[   2]*fa[ 8+2]) + (fb[   3]*fa[12+2]);
	r4 = (fb[   0]*fa[   3]) + (fb[   1]*fa[ 4+3]) + (fb[   2]*fa[ 8+3]) + (fb[   3]*fa[12+3]);
	r5 = (fb[ 4+0]*fa[   0]) + (fb[ 4+1]*fa[ 4+0]) + (fb[ 4+2]*fa[ 8+0]) + (fb[ 4+3]*fa[12+0]);
	r6 = (fb[ 4+0]*fa[   1]) + (fb[ 4+1]*fa[ 4+1]) + (fb[ 4+2]*fa[ 8+1]) + (fb[ 4+3]*fa[12+1]);
	r7 = (fb[ 4+0]*fa[   2]) + (fb[ 4+1]*fa[ 4+2]) + (fb[ 4+2]*fa[ 8+2]) + (fb[ 4+3]*fa[12+2]);
	r8 = (fb[ 4+0]*fa[   3]) + (fb[ 4+1]*fa[ 4+3]) + (fb[ 4+2]*fa[ 8+3]) + (fb[ 4+3]*fa[12+3]);
	r9 = (fb[ 8+0]*fa[   0]) + (fb[ 8+1]*fa[ 4+0]) + (fb[ 8+2]*fa[ 8+0]) + (fb[ 8+3]*fa[12+0]);
	r10= (fb[ 8+0]*fa[   1]) + (fb[ 8+1]*fa[ 4+1]) + (fb[ 8+2]*fa[ 8+1]) + (fb[ 8+3]*fa[12+1]);
	r11= (fb[ 8+0]*fa[   2]) + (fb[ 8+1]*fa[ 4+2]) + (fb[ 8+2]*fa[ 8+2]) + (fb[ 8+3]*fa[12+2]);
	r12= (fb[ 8+0]*fa[   3]) + (fb[ 8+1]*fa[ 4+3]) + (fb[ 8+2]*fa[ 8+3]) + (fb[ 8+3]*fa[12+3]);
	r13= (fb[12+0]*fa[   0]) + (fb[12+1]*fa[ 4+0]) + (fb[12+2]*fa[ 8+0]) + (fb[12+3]*fa[12+0]);
	r14= (fb[12+0]*fa[   1]) + (fb[12+1]*fa[ 4+1]) + (fb[12+2]*fa[ 8+1]) + (fb[12+3]*fa[12+1]);
	r15= (fb[12+0]*fa[   2]) + (fb[12+1]*fa[ 4+2]) + (fb[12+2]*fa[ 8+2]) + (fb[12+3]*fa[12+2]);
	r16= (fb[12+0]*fa[   3]) + (fb[12+1]*fa[ 4+3]) + (fb[12+2]*fa[ 8+3]) + (fb[12+3]*fa[12+3]);

// fc may == fa or fb so we have to cache then write.

	fc[ 0]=r1;	fc[ 1]=r2;	fc[ 2]=r3;	fc[ 3]=r4;
	fc[ 4]=r5;	fc[ 5]=r6;	fc[ 6]=r7;	fc[ 7]=r8;
	fc[ 8]=r9;	fc[ 9]=r10;	fc[10]=r11;	fc[11]=r12;
	fc[12]=r13;	fc[13]=r14;	fc[14]=r15;	fc[15]=r16;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void raw_tardis_v4_product_m4(float *fa,float *fb,float *fc)
{
float r1,r2,r3,r4;
	
	r1 = (fb[   0]*fa[   0]) + (fb[ 4+0]*fa[   1]) + (fb[ 8+0]*fa[   2]) + (fb[12+0]*fa[   3]);
	r2 = (fb[   1]*fa[   0]) + (fb[ 4+1]*fa[   1]) + (fb[ 8+1]*fa[   2]) + (fb[12+1]*fa[   3]);
	r3 = (fb[   2]*fa[   0]) + (fb[ 4+2]*fa[   1]) + (fb[ 8+2]*fa[   2]) + (fb[12+2]*fa[   3]);
	r4 = (fb[   3]*fa[   0]) + (fb[ 4+3]*fa[   1]) + (fb[ 8+3]*fa[   2]) + (fb[12+3]*fa[   3]);

// fc may == fa or fb so we have to cache then write.

	fc[ 0]=r1;	fc[ 1]=r2;	fc[ 2]=r3;	fc[ 3]=r4;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_m4_product_m4 (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
float *fb=(float *)lua_tardis_uda(l,2);
float *fc=(float *)lua_tardis_uda(l,3);
int ridx=3;

	if(!fc) { fc=fa; ridx=1; }

	raw_tardis_m4_product_m4(fa,fb,fc);

	lua_pushvalue(l,ridx); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_v4_product_m4 (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);
float *fb=(float *)lua_tardis_uda(l,2);
float *fc=(float *)lua_tardis_uda(l,3);

int ridx=3;

	if(!fc) { fc=fa; ridx=1; }

	raw_tardis_v4_product_m4(fa,fb,fc);

	lua_pushvalue(l,ridx); // return passed in value for chain function calls
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_m4_identity (lua_State *l)
{
float *fa=(float *)lua_tardis_uda(l,1);

	fa[ 0]=1.0f; fa[ 1]=0.0f; fa[ 2]=0.0f; fa[ 3]=0.0f;
	fa[ 4]=0.0f; fa[ 5]=1.0f; fa[ 6]=0.0f; fa[ 7]=0.0f;
	fa[ 8]=0.0f; fa[ 9]=0.0f; fa[10]=1.0f; fa[11]=0.0f;
	fa[12]=0.0f; fa[13]=0.0f; fa[14]=0.0f; fa[15]=1.0f;

	lua_pushvalue(l,1); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
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
int fc_idx=4;
	
	if(lua_isuserdata(l,3))
	{
		v=(float *)lua_tardis_uda(l,3);
		x=v[0];
		y=v[1];
		z=v[2];
	}
	else
	if(lua_isnumber(l,3))
	{
		x=(float)lua_tonumber(l,3);
		y=(float)lua_tonumber(l,4);
		z=(float)lua_tonumber(l,5);
		fc_idx=6;
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
	
	fa=(float *)lua_tardis_uda(l,1);
	degrees=(float)lua_tonumber(l,2);
	fc=(float *)lua_tardis_uda(l,fc_idx);
	if(!fc) { fc=fa; fc_idx=1; }
//printf("(fa,fc)=(%08x,%08x)\n",fa,fc);

	c=cosf(degrees*((float)(-M_PI/180.0f)));
	cc=1-c;
	s=sinf(degrees*((float)(-M_PI/180.0f)));
		
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

	lua_pushvalue(l,fc_idx); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_m4_scale_v3 (lua_State *l)
{
float *fa;
float *fb;
float *fc;
int fc_idx;
float x,y,z;

	if(lua_isuserdata(l,2))
	{
		fb=(float *)lua_tardis_uda(l,2);
		x=fb[0];
		y=fb[1];
		z=fb[2];
		fc_idx=3;
	}
	else
	if(lua_isnumber(l,2))
	{
		x=(float)lua_tonumber(l,2);
		y=(float)lua_tonumber(l,3);
		z=(float)lua_tonumber(l,4);
		fc_idx=5;
	}
	else
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		lua_rawgeti(l,2,1); x=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,2); y=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,3); z=(float)lua_tonumber(l,-1); lua_pop(l,1);
		fc_idx=3;
	}

	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,fc_idx);
	if(!fc) { fc=fa; fc_idx=1; }

	fc[ 0]=x*fa[ 0];		fc[ 1]=x*fa[ 1];		fc[ 2]=x*fa[ 2];		fc[ 3]=x*fa[ 3];
	fc[ 4]=y*fa[ 4];		fc[ 5]=y*fa[ 5];		fc[ 6]=y*fa[ 6];		fc[ 7]=y*fa[ 7];
	fc[ 8]=z*fa[ 8];		fc[ 9]=z*fa[ 9];		fc[10]=z*fa[10];		fc[11]=z*fa[11];
	fc[12]=fa[12];			fc[13]=fa[13];			fc[14]=fa[14];			fc[15]=fa[15];

	lua_pushvalue(l,fc_idx); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_m4_scale (lua_State *l)
{
float *fa;
float *fb;
float *fc;
int fc_idx;
float s;

	s=(float)lua_tonumber(l,2);
	fc_idx=3;

	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,fc_idx);
	if(!fc) { fc=fa; fc_idx=1; }

	fc[ 0]=s*fa[ 0];		fc[ 1]=s*fa[ 1];		fc[ 2]=s*fa[ 2];		fc[ 3]=s*fa[ 3];
	fc[ 4]=s*fa[ 4];		fc[ 5]=s*fa[ 5];		fc[ 6]=s*fa[ 6];		fc[ 7]=s*fa[ 7];
	fc[ 8]=s*fa[ 8];		fc[ 9]=s*fa[ 9];		fc[10]=s*fa[10];		fc[11]=s*fa[11];
	fc[12]=s*fa[12];		fc[13]=s*fa[13];		fc[14]=s*fa[14];		fc[15]=s*fa[15];

	lua_pushvalue(l,fc_idx); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_tardis_m4_translate (lua_State *l)
{
float *fa;
float *fb;
float *fc;
int fc_idx=4;
float x,y,z;

	if(lua_isuserdata(l,2))
	{
		fb=(float *)lua_tardis_uda(l,2);
		x=fb[0];
		y=fb[1];
		z=fb[2];
		fc_idx=3;
	}
	else
	if(lua_isnumber(l,2))
	{
		x=(float)lua_tonumber(l,2);
		y=(float)lua_tonumber(l,3);
		z=(float)lua_tonumber(l,4);
		fc_idx=5;
	}
	else
	{
		luaL_checktype(l, 2, LUA_TTABLE);
		lua_rawgeti(l,2,1); x=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,2); y=(float)lua_tonumber(l,-1); lua_pop(l,1);
		lua_rawgeti(l,2,3); z=(float)lua_tonumber(l,-1); lua_pop(l,1);
		fc_idx=3;
	}

	fa=(float *)lua_tardis_uda(l,1);
	fc=(float *)lua_tardis_uda(l,fc_idx);
	if(!fc) { fc=fa; fc_idx=1; }

	if(fa==fc) // no need to copy
	{
		fc[12]+= x*fc[0+0] + y*fc[4+0] + z*fc[8+0];
		fc[13]+= x*fc[0+1] + y*fc[4+1] + z*fc[8+1];
		fc[14]+= x*fc[0+2] + y*fc[4+2] + z*fc[8+2];
		fc[15]+= x*fc[0+3] + y*fc[4+3] + z*fc[8+3];
	}
	else
	{
		fc[ 0]=fa[ 0];
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
		fc[12]=fa[12] + x*fc[0+0] + y*fc[4+0] + z*fc[8+0];
		fc[13]=fa[13] + x*fc[0+1] + y*fc[4+1] + z*fc[8+1];
		fc[14]=fa[14] + x*fc[0+2] + y*fc[4+2] + z*fc[8+2];
		fc[15]=fa[15] + x*fc[0+3] + y*fc[4+3] + z*fc[8+3];
	}

	lua_pushvalue(l,fc_idx); // return passed in value for chain function calls
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_tardis_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"alloc",					lua_tardis_alloc},
		{"ptr",						lua_tardis_ptr},
		{"set",						lua_tardis_set},
		{"read",					lua_tardis_read},
		{"write",					lua_tardis_write},
		{"compare",					lua_tardis_compare},

		{"new_m4",					lua_tardis_new_m4},		
		{"new_v4",					lua_tardis_new_v4},

		{"m4_product_m4",			lua_tardis_m4_product_m4},
		{"v4_product_m4",			lua_tardis_v4_product_m4},

		{"m4_identity",				lua_tardis_m4_identity},
		{"m4_rotate",				lua_tardis_m4_rotate},
		{"m4_scale",				lua_tardis_m4_scale},
		{"m4_scale_v3",				lua_tardis_m4_scale_v3},
		{"m4_translate",			lua_tardis_m4_translate},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

