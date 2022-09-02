/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include "kiss_fftr.h"


#ifdef __cplusplus
extern "C" {
#endif

// simple memory management, eats 64k on use

#define KISSFFTDAT_LEN_MAX 16384
typedef struct
{
	int len;    // upto KISSFFTDAT_LEN_MAX
	int nfreqs; // len/2+1
	int count; // count inputs
	
	float din[KISSFFTDAT_LEN_MAX]; // sample data input
	float dot[(KISSFFTDAT_LEN_MAX/2)+1]; // freq data output

	kiss_fft_cpx tmp[(KISSFFTDAT_LEN_MAX/2)+1]; // freq data output

	kiss_fftr_cfg cfg;

} kissfftdat;


LUALIB_API int luaopen_kissfft_core (lua_State *l);

#ifdef __cplusplus
};
#endif

