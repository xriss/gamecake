/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

#include "flite.h"
#include "cst_cg.h"

extern cst_voice *register_cmu_us_slt();
extern cst_cg_db cmu_us_slt_cg_db;

//extern cst_voice *register_cmu_us_kal();

static int init=0;
static cst_voice *voice=0;

// allocated junk 
static u16 **idxs;
static int   idxs_size;

// setup stuff if we must
static int lua_speak_setup (lua_State *l)
{
u16  *data=0;
size_t   data_size=0;

u32  *offs=0;
size_t   offs_size=0;
int i;
	
	if(!init)
	{
		
// data passed in from lua should be kept live so as not to GC it, be careful		
	if(lua_isstring(l,1))
	{
		data=(void*)luaL_checklstring(l,1,&data_size);
	}
	else
	if(lua_isuserdata(l,1))
	{
		data=(void*)lua_toluserdata(l,1,&data_size);
	}
		
	if(lua_isstring(l,2))
	{
		offs=(void*)luaL_checklstring(l,2,&offs_size);
	}
	else
	if(lua_isuserdata(l,1))
	{
		offs=(void*)lua_toluserdata(l,2,&offs_size);
	}

// allocate	pointers (64 bit compat), one time init
	idxs=calloc(offs_size,sizeof(u16 *));
	idxs_size=offs_size;
	

	for(i=0;i<idxs_size;i++)
	{
		idxs[i]=data + (offs[i]/2);
	}

// store in struct
	cmu_us_slt_cg_db.model_vectors0=(const unsigned short * const * )idxs;
		
		flite_init();
		voice=register_cmu_us_slt();
//		voice=register_cmu_us_kal();

//    flite_feat_set_float(voice->features,"int_f0_target_mean",95.0);
//    flite_feat_set_float(voice->features,"int_f0_target_stddev",11.0*1);
//    flite_feat_set_float(voice->features,"duration_stretch",1.1);

		init=1;
	}
		
	return 0;
}


// render some text using the current voice
static int lua_speak_text (lua_State *l)
{
cst_wave *sound;
s16 *data;
const char *text;
int len;

s16 *p;
s16 *pend;

int d;


s16 *p_start;
s16 *p_stop;
int cbeg,cend;
int clips=0;

//	lua_speak_setup(l);

	text=luaL_checkstring(l,1);
	sound = flite_text_to_wave(text, voice);
		
	data=sound->samples;
	pend=data+sound->num_samples;
	
//	int mp=1;
//	int ph=32;
//	int m=1;
	for(p=data;p<pend;p++)
	{
		d=*p;
		d=d*3;//m/ph;
		
//		if(d>0x7fff) { d=0xffff-d; clips++; }
//		if(d<-0x7fff) { d=-0xffff+d; clips++; }

		if(d>0x7fff) { d=0x7fff; }
		if(d<-0x7fff) { d=-0x7fff; }

		*p=(s16)d;
		
//		m=m+mp;
//		if(m==1) {mp=1;}
//		if(m==ph*16) {mp=-1;}
	}
//printf("clips %d\n",clips);


#define samptrim (1024)
	p_start=0;
	p_stop=data-1;
	for(p=data;p<pend;p++)
	{
		d=*p;
		if((d>samptrim)||(d<-samptrim))
		{
			if(!p_start) { p_start=p; } // first non zero sample
		}
	}
	if(!p_start) { p_start=p; } // first non zero sample
	
	p_stop=pend-1;
/*	
	for(p=pend-1;p>=data;p--)
	{
		d=*p;
		if((d>samptrim)||(d<-samptrim))
		{
			p_stop=p; // last non zero sample
			break;
		}
	}
*/
	
	cbeg=p_start-data;
	cend=pend-(p_stop+1);

//printf("blank space %d %d / %d\n",cbeg,cend,sound->num_samples);

	
	len=(p_stop+1-p_start)*2;
	if(len<1) {len=1;p_start=data;}
	data = (u16*)lua_newuserdata(l,len);
	memcpy(data,p_start,len);
	lua_pushnumber(l,len);

/*
	data = (u16*)lua_newuserdata(l,sound->num_samples*2);
	memcpy(data,sound->samples,sound->num_samples*2);
	lua_pushnumber(l,sound->num_samples*2);
*/

	delete_wave(sound);

// return userdata,size
	return 2;
}

//        feat_set_float(voice->features,"int_f0_target_mean", pitch);
 //       feat_set_float(voice->features,"int_f0_target_stddev",variance);
  //      feat_set_float(voice->features,"duration_stretch",speed); 
  
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set the current voice
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_speak_voice (lua_State *l)
{
float mean=100; // 300
float stddev=10; // 100
float stretch=1; // 0.74

//	lua_speak_setup(l);
	
	if( lua_istable(l,1) )
	{

		lua_getfield(l,1,"mean");
		if(lua_isnumber(l,-1)) { mean=(float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,1,"stddev");
		if(lua_isnumber(l,-1)) { stddev=(float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,1,"stretch");
		if(lua_isnumber(l,-1)) { stretch=(float)lua_tonumber(l,-1); }
		lua_pop(l,1);
	}
	
    flite_feat_set_float(voice->features,"int_f0_target_mean",mean);
    flite_feat_set_float(voice->features,"int_f0_target_stddev",stddev);
    flite_feat_set_float(voice->features,"duration_stretch",stretch);

	return 0;
}
  
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// speach library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_speak_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"setup",			lua_speak_setup},
		
		{"test",			lua_speak_text},
		
		{"text",			lua_speak_text},
		{"voice",			lua_speak_voice},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

