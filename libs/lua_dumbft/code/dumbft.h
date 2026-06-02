/*

MIT License

Copyright (c) 2026 Kriss@XIXs.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


#ifndef DFT_H
#define DFT_H

#define DFT_VERSION 1.260601

#ifdef __cplusplus
extern "C" {
#endif

// we mostly deal with arrays of 16bit signed values
#include <stdint.h>

// we need sin and cos
#include <math.h>

// fucking radians
#define DFT_TAU 6.28318530717958647692

// memcpy is probably the best way to copy memory
#ifndef DFT_MEMCPY
#include <string.h>
#define DFT_MEMCPY memcpy
#endif

// Roll your own damn allocator
#ifndef DFT_MEM
#include <stdlib.h>
#define DFT_MEM dft_mem
// call with 0,?,?,? -> alloc a ctx
// call with 1,0,0,0 -> free a ctx
// call with 1,0,?,1 -> ctx:alloc a ptr of siz
// call with 1,1,1,0 -> ctx:free a ptr of old 
// call with 1,1,1,1 -> ctx:realloc a ptr of old to siz
extern void *dft_mem(void *ctx,void *ptr,int old,int siz)
{
	if(!ctx) { return (void*)1; } // no context needed
	else
	if(ptr)
	{
		if(siz) { return realloc(ptr,siz); }
		else
		{ free(ptr); return (void*)0; }
	}
	else
	{
		if(siz) { return malloc(siz); }
	}
	return (void*)0; // no context needed
}
#endif


typedef struct dft_bucket
{
	int      wavlen;		// length of each probe and data buffers
	int      idx;			// 0 .. wavlen-1 as we push data in
	int      sin_total;		// running sin total
	int      cos_total;		// running cos total
	int16_t* sin_probe;		// sin probe
	int16_t* cos_probe;		// cos probe
	int16_t* sin_data;		// sin probe*data>>15
	int16_t* cos_data;		// cos probe*data>>15

} dft_bucket ;

typedef struct dft_state
{
	void*       memctx;				// memory context if you roll your own
	int         sizeof_mem;			// size of mem allocated
	void*       mem;				// big chunk of allocated memory
	int         numof_buckets;		// number of buckets allocated
	dft_bucket* buckets;			// array of buckets

} dft_state ;

// C interface used to parse and stringify json/djon
extern dft_state * dft_setup( int numof_buckets , int *wavlens );
extern void        dft_clean( dft_state *ds );

#ifdef __cplusplus
};
#endif

#endif


#ifdef DFT_C

#ifdef __cplusplus
extern "C" {
#endif

// compare null terminated string s, to the cs buffer
extern dft_state * dft_setup( int numof_buckets , int *wavlens )
{
	void *ctx=DFT_MEM(0,0,0,0); // we don't use a context but you can
	int sizeof_mem=0;
	
	sizeof_mem+=sizeof(dft_state);
	for( int idx=0 ; idx<numof_buckets ; idx++ )
	{
		sizeof_mem+=sizeof(dft_bucket);
		sizeof_mem+=wavlens[idx]*2*4;	// 4 word buffers
	}
	int8_t* ptr=(int8_t*)DFT_MEM(ctx,0,0,sizeof_mem); // all the mem we need
	if(!ptr) { return 0; } // fail

	dft_state *ds=(dft_state*)ptr; ptr+=sizeof(dft_state);
	ds->memctx=ctx;
	ds->sizeof_mem=sizeof_mem;
	ds->mem=ptr;
	ds->numof_buckets=numof_buckets;
	ds->buckets=(dft_bucket*)(ptr); ptr+=sizeof(dft_bucket)*numof_buckets;
	
	for( int idx=0 ; idx<numof_buckets ; idx++ )
	{
		dft_bucket *bucket=ds->buckets+idx;
		bucket->wavlen=wavlens[idx];
		bucket->idx=0;
		bucket->sin_total=0;
		bucket->cos_total=0;
		bucket->sin_probe=(int16_t*)ptr; ptr+=2*bucket->wavlen;
		bucket->cos_probe=(int16_t*)ptr; ptr+=2*bucket->wavlen;
		bucket->sin_data=(int16_t*)ptr; ptr+=2*bucket->wavlen;
		bucket->cos_data=(int16_t*)ptr; ptr+=2*bucket->wavlen;

		for( int i=0 ; i<bucket->wavlen ; i++ )
		{
			double turns=((double)(i))/((double)(bucket->wavlen));
			bucket->sin_probe[i]= (int16_t)( 32767.0 * sin( DFT_TAU*turns ) );
			bucket->cos_probe[i]= (int16_t)( 32767.0 * cos( DFT_TAU*turns ) );
			bucket->sin_data[i]=0;
			bucket->cos_data[i]=0;
		}
	}

	return ds;
}

extern void        dft_clean( dft_state *ds )
{
	void *ctx=ds->memctx; // remember ctx as it is stored in memory
	DFT_MEM(ctx,ds->mem,ds->sizeof_mem,0);	// free memory
	DFT_MEM(ctx,0,0,0);	// free ctx
}

#ifdef __cplusplus
};
#endif

#endif

