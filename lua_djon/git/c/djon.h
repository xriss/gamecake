/*

MIT License

Copyright (c) 2024 Kriss@XIXs.com

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


#ifndef DJON_H
#define DJON_H

#define DJON_VERSION 1.251031

#ifdef __cplusplus
extern "C" {
#endif

// maximum path size including null
#ifndef DJON_MAX_PATH
#define DJON_MAX_PATH (1024)
#endif

// maximum stack to use, when parsing
// set to 0 to disable our internal stack checking
#ifndef DJON_MAX_STACK
#define DJON_MAX_STACK (256*1024)
#endif

// number of values to realloc when parsing
#ifndef DJON_VALUE_CHUNK_SIZE
#define DJON_VALUE_CHUNK_SIZE (16384)
#endif

// number of bytes to realloc when building strings
#ifndef DJON_STRING_CHUNK_SIZE
#define DJON_STRING_CHUNK_SIZE (0x10000)
#endif

// include file (fopen fread etc) functions
#ifndef DJON_FILE
#define DJON_FILE (1)
#endif
#if DJON_FILE
#include <stdio.h>
#endif

// need some maths for parsing numbers
#if !(defined(DJON_POW10)&&defined(DJON_LOG10)&&defined(DJON_FLOOR))
#include <math.h>
#endif
#ifndef DJON_POW10 
#define DJON_POW10(e) pow(10.0,(double)(e))
#endif
#ifndef DJON_LOG10
#define DJON_LOG10(x) log10((double)(x))
#endif
#ifndef DJON_FLOOR
#define DJON_FLOOR(x) floor((double)(x))
#endif

// memcpy is probably the best way to copy memory
#ifndef DJON_MEMCPY
#include <string.h>
#define DJON_MEMCPY memcpy
#endif

// Roll your own damn allocator
#ifndef DJON_MEM
#include <stdlib.h>
#define DJON_MEM djon_mem
// call with 0,?,?,? -> alloc a ctx ( siz can be a hint of future sizes)
// call with 1,0,?,0 -> free a ctx
// call with 1,0,?,1 -> alloc a ptr in given ctx
// call with 1,1,?,0 -> free a ptr in given ctx
// call with 1,1,?,1 -> realloc a ptr in given ctx
extern void *djon_mem(void *ctx,void *ptr,int old,int siz);
void *djon_mem(void *ctx,void *ptr,int old,int siz)
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
#define DJON_MEM_MALLOC(ds,siz)          DJON_MEM(ds->memctx,0,0,siz)
#define DJON_MEM_FREE(ds,ptr,old)        DJON_MEM(ds->memctx,ptr,old,0)
#define DJON_MEM_REALLOC(ds,ptr,old,siz) DJON_MEM(ds->memctx,ptr,old,siz)

typedef enum djon_enum
{
	DJON_NULL     = 0x0001,
	DJON_NUMBER   = 0x0002,
	DJON_BOOL     = 0x0003,
	DJON_STRING   = 0x0004,
	DJON_ARRAY    = 0x0005,
	DJON_OBJECT   = 0x0006,
	DJON_COMMENT  = 0x0007,

	DJON_ESCAPED  = 0x0100, // this string contains \n \t \" etc
	DJON_LONG     = 0x0200, // this string is long
	DJON_KEY      = 0x0400, // this string is a key

	DJON_FREE     = 0x0000, // this value is free and can be allocated

	DJON_TYPEMASK = 0x00ff, // base types are in lower byte
	DJON_FLAGMASK = 0xff00, // additional flags are in upper byte

	DJON_MASK     = 0xFFFF,
} djon_enum ;

typedef struct djon_value
{
// data
	int    typ; // the type of data contained in this structure
	char * str; // start of string ( points into djon_state.data )
	int    len; // length of string
	double num; // number or bool
// pointers to relative values
	int    nxt; // next value if this is part of a list
	int    prv; // prev value if this is part of a list
	int    com; // linked list of comments for this value
	int    lst; // child keys for object or child values for array or child value for a key
	int    par; // parent array/object/key
// array index
	int    idx; // index into array if parent is an array

} djon_value ;

typedef struct djon_state
{
	// input data string
	char *data;
	int   data_len;

	// output values
	djon_value * values;
	int          values_len; // used buffer size
	int          values_inc; // reuse test index
	int          values_reset; // reset test index
	int          values_siz; // allocated buffer size

	// current parse state
	int   parse_idx;
	int   parse_value; // first output value
	char *parse_stack; // starting stack so we can play stack overflow chicken
	int   parse_com;   // a list of comments, cache before we hand it off to a value.
	int   parse_com_last; // end of comment chain so we can easily add another one

	const char *error_string; // if this is not 0 we have an error
	int   error_idx;    // char in buffer
	int   error_char;   // char in line
	int   error_line;   // line in file

#if !DJON_FILE
	void *fp; // we do not know what a file is
#else
	FILE *fp; // where to write output
#endif
	int   small; // what is considered small ( 128 ) set to 0 to disable
	int   compact; // compact output flag
	int   strict; // json bitch mode input flag
	char *write_data; // string output
	int   write_size;
	int   write_len;
	int (*write)(struct djon_state *ds, const char *cp, int len); // ptr to output function

	int   path_len; // path size when constructing path ( built backwards in buf )

	void *memctx; // memory allocator ctx
	char buf[DJON_MAX_PATH]; // small very tempory buffer used for generating text

} djon_state ;

// C interface used to parse and stringify json/djon
extern djon_state * djon_setup();
extern void         djon_clean(       djon_state *ds);
extern int          djon_load_file(   djon_state *ds, const char *fname);
extern int          djon_parse(       djon_state *ds);
extern void         djon_set_error(   djon_state *ds, const char *error);
extern int          djon_write_fp(    djon_state *ds, const char *ptr, int len );
extern int          djon_write_data(  djon_state *ds, const char *ptr, int len );
extern void         djon_write_json(  djon_state *ds, int idx);
extern void         djon_write_djon(  djon_state *ds, int idx);
extern int          djon_alloc(       djon_state *ds);
extern void         djon_free(        djon_state *ds, int idx);
extern djon_value * djon_get(         djon_state *ds, int idx);
extern int          djon_idx(         djon_state *ds, djon_value *dv);
extern int          djon_parse_value( djon_state *ds);
extern int          djon_check_stack( djon_state *ds);
extern void         djon_sort_object( djon_state *ds, int idx );

// simplish path based C interface for create delete and find of json data used in this library
extern int          djon_value_newkey(   djon_state *ds, int base_idx, const char *path, const char *key);
extern int          djon_value_newindex( djon_state *ds, int base_idx, const char *path, int index);
extern void         djon_value_delete(   djon_state *ds, int base_idx, const char *path);
extern void         djon_value_dechild(  djon_state *ds, int base_idx, const char *path);
extern int          djon_value_by_path(  djon_state *ds, int base_idx, const char *path, const char **lastkey);
extern int          djon_value_by_index( djon_state *ds, int base_idx, int index);
extern int          djon_value_all(      djon_state *ds, int base_idx, int di);
extern const char * djon_value_to_path(  djon_state *ds, int base_idx, int di);
// simplish C interface for getting,setting and iterating
extern int          djon_value_copy_str(   djon_state *ds, int di, char *dest, int siz);
extern const char * djon_value_get_str(    djon_state *ds, int di);
extern int          djon_value_get_len(    djon_state *ds, int di);
extern double       djon_value_get_num(    djon_state *ds, int di);
extern int          djon_value_get_typ(    djon_state *ds, int di);
extern int          djon_value_get_nxt(    djon_state *ds, int di);
extern int          djon_value_get_prv(    djon_state *ds, int di);
extern int          djon_value_get_first(  djon_state *ds, int di);
extern int          djon_value_get_parent( djon_state *ds, int di);
extern void         djon_value_set(        djon_state *ds, int, djon_enum typ, double num, int len, const char *str);


// deal with comments round tripping in pure json using vca values
extern int djon_value_to_vca(djon_state *ds,int idx);
extern int djon_vca_to_value(djon_state *ds,int idx);

#define DJON_IS_WHITESPACE(c) ( ((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r')  )
#define DJON_IS_STRUCTURE(c)  ( ((c)=='{') || ((c)=='}') || ((c)=='[') || ((c)==']') || ((c)==':') || ((c)=='=') || ((c)==',') )
#define DJON_IS_DELIMINATOR(c) ( ((c)==0) || DJON_IS_WHITESPACE(c) || ((c)=='/') || DJON_IS_STRUCTURE(c) )
// note that '/' is the start of /* or // comments and 0 will be found at the EOF
#define DJON_IS_QUOTE(c) ( ((c)=='\'') || ((c)=='"') || ((c)=='`') )
#define DJON_IS_NUMBER_START(c) ( (((c)<='9')&&((c)>='0')) || ((c)=='.') || ((c)=='+') || ((c)=='-') )
// a number will start with one of these chars

#ifdef __cplusplus
};
#endif

#endif


#ifdef DJON_C

#ifdef __cplusplus
extern "C" {
#endif

// compare null terminated string s, to the cs buffer
int djon_compare_string(const char *cs,int len,const char *s)
{
	const char *ce=cs+len;
	const char *sp;
	const char *cp;
	char c;
	for( cp=cs,sp=s ; *sp ; cp++,sp++ )
	{
		if(cp>=ce) { return 0; } // out of buffer but not out of string
		c=*cp;
		if( c!=*sp ) { return 0; } // no match
	}
	if(cp==ce) { return 1; } // both strings are of same length
	return 0; // buffer is longer
}

// compare null terminated string s, to the start of the cs buffer
// returns true if buffer begins with the s string.
int djon_starts_with_string(const char *cs,int len,const char *s)
{
	const char *ce=cs+len;
	const char *sp;
	const char *cp;
	char c;
	for( cp=cs,sp=s ; *sp ; cp++,sp++ )
	{
		if(cp>=ce) { return 0; } // out of buffer but not out of string
		c=*cp;
		if( c!=*sp ) { return 0; } // no match
	}
	return 1; // cs starts with s
}

// check that a quote does not occur in the string, returns 1 if it does not
int djon_check_quote( const char *cs , int len , const char *quote )
{
	const char *cq;
	const char *ct;
	const char *cp;
	const char *ce;
	char c;
	for( cp=cs,ce=cs+len ; cp<ce ; cp++ ) // scan buffer
	{
		for( cq=quote , ct=cp ; *cq && (ct<=ce); cq++ , ct++ ) // step ct one char more
		{
			if( ct==ce ) // past end of input
			{
				c='`'; // fake end quote
			}
			else
			{
				c=*ct;
			}
			if( *cq!=c ) { break; }
		}
		if(*cq==0) { return 0; } // found quote string
	}
	return 1; // not found
}

// Pick a ` quote that does not occur in the string,
// this will be written into buf as it may be more than one char
// there are so many possibilities that although this could technically fail
// the string to cause this failure would have to be many gigabytes
char * djon_pick_quote( char *cs , int len , char *buf )
{
// single back tick
	buf[0]='`';buf[1]=0;

// empty string, is single back tick
	if(len<=0)
	{
		return buf;
	}
	
	unsigned int bs;
	unsigned int bm;
	char *cp;

// check single back tick
	int count_back=0;
	int count_other=0;
	for( cp=cs ; cp<cs+len ; cp++ )
	{
		switch(*cp)
		{
			case '\'': break;
			case '"' : break;
			case '`' : count_back++;   break;
			default  : count_other++;  break;
		}
	}
	// we need 0 backticks and chars other than single or double quotes
	if( count_back==0 && count_other>0 )
	{
		return buf;
	}
	
	
// check 2^32 more
	for(bs=0;bs<=0xffffffff;bs++)
	{
		cp=buf;
		*cp++='`'; // first `
		for( bm=0x80000000 ; bm>0x00000001 ; bm=bm>>1 ) // slide bit right
		{
			if( bm<=bs ) { break; } // until we find the top bit
		}
		while(bm>0)
		{
			if(bs&bm) { *cp++='\''; }
			else      { *cp++='"'; } // this will mostly be the first char after `
			bm=bm>>1; // keep sliding
		}
		*cp++='`'; // final `
		*cp++=0;
		if(djon_check_quote(cs,len,buf))
		{
			return buf;
		}
	}
	// we should never reach here
	return 0;
}

// replace /n /t /uXXXX etc with utf8 chars, return new length of string
// new string should always be shorter due to encoding inefficiencies
int djon_unescape( char *cs , int len )
{
	char *cp2;
	char *cp=cs;     // input
	char *co=cs;     // output
	char *ce=cs+len; // end of input
	char c;
	int i;
	int t; // 16bit utf char
	int t2;
	while( cp<ce && co<ce ) // check all the chars and do not overrun
	{
		c=*cp;
		if(c=='\\')
		{
			cp++; // skip backslash
			c=*cp; // get char after
			cp++; // which we always consume
			switch(c)
			{
				case 'b' : *co++='\b'; break; // special chars
				case 'f' : *co++='\f'; break;
				case 'n' : *co++='\n'; break;
				case 'r' : *co++='\r'; break;
				case 't' : *co++='\t'; break;
				default  : *co++=c;    break; // everything else
				case 'u' :
					t=0;
					// note that we may read less than 4 chars here if they are not what we expect
					for(i=0;i<4;i++) // grab *upto* four hex chars next
					{
						c=*cp;
						if( c>='0' && c<='9' )
						{
							t=t*16+(c-'0');
							cp++;
						}
						else
						if( c>='a' && c<='f' )
						{
							t=t*16+(c-'a'+10);
							cp++;
						}
						else
						if( c>='A' && c<='F' )
						{
							t=t*16+(c-'A'+10);
							cp++;
						}
					}
					
					// need to check for surrogate pair
					if( (t&0xFC00)==0xD800 ) // we got the first part, try and parse the second
					{
						if( (cp[0]=='\\') && (cp[1]=='u') )
						{
							cp2=cp; // might need to undo
							cp+=2;
							t2=0;
							for(i=0;i<4;i++) // grab *upto* four hex chars next
							{
								c=*cp;
								if( c>='0' && c<='9' )
								{
									t2=t2*16+(c-'0');
									cp++;
								}
								else
								if( c>='a' && c<='f' )
								{
									t2=t2*16+(c-'a'+10);
									cp++;
								}
								else
								if( c>='A' && c<='F' )
								{
									t2=t2*16+(c-'A'+10);
									cp++;
								}
							}
							if( (t2&0xFC00)==0xDC00 ) // we got the second part
							{
								t= 0x10000 + ((t&0x03FF)*0x0400) + (t2&0x03FF); // 21bitish
							}
							else // undo second part, number was not part of a pair
							{
								cp=cp2;
							}
						}
					}
					
					if(t<=0x007f)
					{
						*co++=t; // 7bit ascii
					}
					else
					if(t<=0x07ff)
					{
						*co++=0xc0|((t>>6) &0x1f); // top 5 bits
						*co++=0x80|( t     &0x3f); // last 6 bits
					}
					else
					if(t<=0xffff)
					{
						*co++=0xe0|((t>>12)&0x0f); // top 4 bits
						*co++=0x80|((t>>6) &0x3f); // next 6 bits
						*co++=0x80|( t     &0x3f); // last 6 bits
					}
					else // 1FFFFF
					{
						*co++=0xf0|((t>>18)&0x03); // top 3 bits
						*co++=0x80|((t>>12)&0x3f); // next 6 bits
						*co++=0x80|((t>>6) &0x3f); // next 6 bits
						*co++=0x80|( t     &0x3f); // last 6 bits
					}
				break;
			}
		}
		else // copy as is
		{
			*co++=c;
			cp++; // consume
		}
	}
	return co-cs; // number of chars written
}

// this will remove the DJON_ESCAPED bit from a string and fix the estring
// it is safe to call multiple times
int djon_unescape_string( djon_value * v )
{
	// must exist
	if(!v) { return 0; }

	// must be string
	if((v->typ&DJON_TYPEMASK)!=DJON_STRING) { return 0; }

	// remove escaped
	if( v->typ & DJON_ESCAPED )
	{

		v->len=djon_unescape(v->str,v->len); // perform unescape

		v->typ&=~DJON_ESCAPED; // remove flag

		return 1; // we changed the string
	}
	else // remove long string flag
	if( v->typ & DJON_LONG )
	{
		// when dealing with multi-line strings it feels more natural
		// to ignore the first newline
		if( v->len>0 && v->str[0]=='\n' ) // remove first \n
		{
			v->len--;
			v->str++;
		}

		v->typ&=~DJON_LONG; // remove flag

		return 1; // we changed the string
	}

	return 0;
}

// can this string be naked
int djon_is_naked_string( const char *cp , int len )
{
	if(len<=0) { return 0; } // string may not empty
	const char *ce=cp+len;
	char c=*cp;
	// check starting char is not part of djon structure or whitespace
	if( DJON_IS_STRUCTURE(c) || DJON_IS_WHITESPACE(c) || DJON_IS_QUOTE(c) || DJON_IS_NUMBER_START(c) )
	{
		return 0; //  can not be structure or white space or look like a number
	}
	// check for json keywords that will trip us up
	// a naked string may not begin with these words because json
	if( djon_starts_with_string(cp,len,"true") ) { return 0; }
	if( djon_starts_with_string(cp,len,"True") ) { return 0; }
	if( djon_starts_with_string(cp,len,"TRUE") ) { return 0; }
	if( djon_starts_with_string(cp,len,"false") ) { return 0; }
	if( djon_starts_with_string(cp,len,"False") ) { return 0; }
	if( djon_starts_with_string(cp,len,"FALSE") ) { return 0; }
	if( djon_starts_with_string(cp,len,"null") ) { return 0; }
	if( djon_starts_with_string(cp,len,"Null") ) { return 0; }
	if( djon_starts_with_string(cp,len,"NULL") ) { return 0; }
	// and a naked string can not be the start of a comment but can be a /
	// which is needed for file paths
	if( djon_starts_with_string(cp,len,"//") ) { return 0; }
	if( djon_starts_with_string(cp,len,"/*") ) { return 0; }
	// string can not end with whitespace as it would be stripped
	c=*(cp+len-1);
	if( DJON_IS_WHITESPACE(c) ) { return 0; } // ends in whitespace
	// finally need to make sure that string down not contain any \n
	while( cp<ce )
	{
		if(*cp=='\n') { return 0; } // found a \n
		cp++;
	}
	return 1; // all chars OK
}

// can this key be naked
int djon_is_naked_key( const char *cp , int len )
{
	if(len<=0) { return 0; } // may not empty
	const char *ce=cp+len;
	const char *cs=cp;
	while( cs<ce ) // check all the chars do not contain a deliminator
	{
		char c=*cs++;
		if( DJON_IS_DELIMINATOR(c) ) { return 0; }
	}
	return 1;
}

// write len chars into buf ( each char is 4 bits ) 
void djon_int_to_hexstr( int num , int len , char * buf )
{
	char *cp=buf;
	for(int i=0 ; i<len ; i++ )
	{
		int t=0xf&(num>>(4*(len-(i+1))));
		if(t>=10) // letter
		{
			*cp++='A'+(t-10);
		}
		else
		{
			*cp++='0'+t;
		}
	}
	*cp++=0;
}

// write into buf, return length of string writen excluding null terminator
// this should write a maximum of 27 including null terminator to the buffer
// so please supply at least a 32 char buffer to write into.
int djon_double_to_str_internal( double _num , char * buf , int first_call)
{
// maximum precision of digits ( chosen for stable doubles precision )
#define DJON_DIGIT_PRECISION 15
// amount of zeros to include before/after decimal point before we switch to exponents
#define DJON_DIGIT_ZEROS 9
// these two numbers with '-' at the start including a decimal '.' and a '\0' at the end is the worst case
#define DJON_DIGIT_LEN 27

	char *cp=buf;
	double num=_num;

	if( isnan(num) ) // A nan , so we write null
	{
		*cp++='n';
		*cp++='u';
		*cp++='l';
		*cp++='l';
		*cp=0; // null
		return cp-buf;
	}

	int negative=0;
	if( signbit(num) )
	{
		negative=1; // remember
		*cp++='-';
		num=-num; // remove sign
	}

	if(num==0.0) // zero
	{
		cp-=negative; // remove "-" so we never "-0"
		*cp++='0';
		*cp=0; // null
		return cp-buf;
	}

	if(isinf(num)) //inf
	{
		*cp++='9';
		*cp++='e';
		*cp++='9';
		*cp++='9';
		*cp++='9';
		*cp=0; // null
		return cp-buf;
	}

	double t; // divide by this to get current decimal
	int e=(int)DJON_LOG10(num); // find exponent
	if(e<0) { e=e-1; } // we want to start with 0.
	
	if(e<-307) // give up when we get too close to min
	{
		cp-=negative; // remove "-" so we never "-0"
		*cp++='0';
		*cp=0; // null
		return cp-buf;
	}


	int i;
	int j;
	int d;

	int digits=DJON_DIGIT_PRECISION;
	if( (e<=0) && (e>=-DJON_DIGIT_ZEROS) ) // special case when close to 0 dont add an e until we get really small
	{
		digits=DJON_DIGIT_PRECISION+1-e;
		e=0; // we want a 0.0000001234 style number when it is near 0
	}

	int ti=e; // current power of 10 exponent
	if(e>0)
	{
		e=e+1-digits; // the e we will be when we print all the digits
		if(e<0) { e=0; } // goes from +e to -e so force 0
	}
	if(e<0) // start with a "0." when the number goes tiny.
	{
		e=e+1;
		*cp++='0';
		*cp++='.';
	}
 
	int z=0; // run of zeros we will want to undo
	int dz=0; // flag decimal point
	for(i=0;i<digits;i++) // probably 15 digits
	{
		t=DJON_POW10(ti); // next digit
		if( ti == -1 ) { *cp++='.'; z=1; dz=1; } // decimal point, reset out count of zeros and include this "."
		d=(int)((num)/t); //auto floor converting to int
		if(d<0){d=0;} //clamp digits because floating point is fun
		if(d>9){d=9;}
		num-=((double)d)*t;
		ti--; // next digit
		if(d==0) { z++; } else { z=0; } // count zeros at end
		*cp++='0'+d;
	}
	t=DJON_POW10(ti); // next digit
	d=(int)((num)/t); //auto floor converting to int
	if( (d>=5) && (z<3) && first_call )// start again and round up unless we are looking at multiple trailing 0s
	{
		if(negative) { t=-t; }
		return djon_double_to_str_internal( _num + (t*10.0), buf , 0 );
	}


	if( (e>0) && (e<=DJON_DIGIT_ZEROS) ) // we want to keep all these zeros and add some more
	{
		for(i=0;i<e;i++) { *cp++='0'; }
		e=0; // we want a 1234000000 style number when it is near 0
	}
	else
	if(z>0) // undo trailing zeros
	{
		cp=cp-z; // remove zeros
		if(e>=0) // adjust e number
		{
			if(dz==0) { e=e+z; } // e needs fixing only if no decimal point
		}
	}

	if(e!=0)
	{
		*cp++='e'; // Lowercase 'e' as it is more visible in a string of digits
		if(e<0)
		{
			*cp++='-';
			e=-e;
		}
		for( i= (e>=100) ? 100 : (e>=10) ? 10 : 1 ; i>=1 ; i=i/10 )
		{
			d=e/i;
			e-=d*i;
			*cp++='0'+d;
		}
	}

	*cp=0; // null
	return cp-buf;
}

// locales are a cursed so we need our own basic functions ?
// strtod replacement ?
double djon_str_to_double(char *cps,char **endptr)
{
	int gotdata=0;

	char c;
	char *cp=cps;

	c=*cp;
	double sign=1.0;
	if(c=='-') { sign=-1.0; cp++; } // maybe negative
	else
	if(c=='+') { cp++; } // allow

	double d=0.0;
	for( c=*cp ; (c>='0') && (c<='9') ; c=*++cp )// 0 or more integer parts
	{
		d=(d*10.0)+(c-'0');
		gotdata++;
	}

	double m=1.0;
	if( *cp=='.' ) // a decimal point
	{
		cp++;
		for( c=*cp ; (c>='0') && (c<='9') ; c=*++cp )// and 0 or more integer parts
		{
			m=m*0.1;
			d+=((c-'0')*m);
			gotdata++;
		}
	}

	if(gotdata==0){goto error;} // require some numbers before or after decimal point

	c=*cp;
	if( (c=='e') || (c=='E') )
	{
		cp++;
		c=*cp;

		gotdata=0; // reset

		double esign=1.0;
		if(c=='-') { esign=-1.0; cp++; } // maybe negative
		else
		if(c=='+') { cp++; } // allow

		double e=0.0;
		for( c=*cp ; (c>='0') && (c<='9') ; c=*++cp )// and 0 or more exponent parts
		{
			m=m*0.1;
			e=(e*10.0)+(c-'0');
			gotdata++;
		}
		d*=DJON_POW10(e*esign); // apply exponent

		if(gotdata==0){goto error;} // require some numbers after the e
	}
	d*=sign; // apply sign

	// final check, number must be deliminator by something to be valid
	c=*cp;
	if( ! DJON_IS_DELIMINATOR(c) ){goto error;}

	if(endptr){*endptr=cp;} // we used this many chars
	return d; // and parsed this number

error:
	if(endptr){*endptr=cps;} // 0 chars used
	return NAN;
}

int djon_double_to_str( double _num , char * buf )
{
	djon_double_to_str_internal( _num , buf , 1 );	// write
	double d=djon_str_to_double(buf,0);						// read
	return djon_double_to_str_internal( d , buf , 1 );		// write ( now stable )
}

double djon_str_to_hex(char *cps,char **endptr)
{
	int gotdata=0;

	char *cp=cps;
	char c=*cp;

	double sign=1.0;
	if(c=='-') { sign=-1.0; c=*++cp; } // maybe negative
	else
	if(c=='+') { c=*++cp; } // allow

	double d=0.0;

	if(!( (cp[0]=='0') && ( (cp[1]=='x') || (cp[1]=='X') ) )){goto error;}
	cp+=2; // skip 0x

	for( c=*cp ; c ; c=*++cp )
	{

		if( (c>='0') && (c<='9') )
		{
			d=(d*16.0)+(double)(c-'0');
			gotdata++;
		}
		else
		if( (c>='a') && (c<='f') )
		{
			d=(d*16.0)+(double)(c-'a'+10);
			gotdata++;
		}
		else
		if( (c>='A') && (c<='F') )
		{
			d=(d*16.0)+(double)(c-'A'+10);
			gotdata++;
		}
		else
		{
			break;
		}
	}
	if(gotdata==0){goto error;} // require some numbers

	// final check, number must be deliminator by something to be valid
	if( ! DJON_IS_DELIMINATOR(c) ){goto error;}

	d*=sign; // apply sign

	if(endptr){*endptr=cp;} // we used this many chars
	return d; // and parsed this number

error:
	if(endptr){*endptr=cps;} // 0 chars used
	return NAN;
}

double djon_str_to_number(djon_state *ds, char *cp,char **endptr)
{
	if	(
			( (cp[0]=='0') && ( (cp[1]=='x') || (cp[1]=='X') ) )
			||
			( ( (cp[0]=='+') || (cp[0]=='-') ) && (cp[1]=='0') && ( (cp[2]=='x') || (cp[2]=='X') ) )
		)
	{
		if(ds->strict)
		{
			djon_set_error(ds,"0x numbers not allowed in strict mode");
			return 0;
		}
		return djon_str_to_hex(cp,endptr);
	}
	else
	{
		return djon_str_to_double(cp,endptr);
	}
}

// set an error string and calculate the line/char that we are currently on
void djon_set_error(djon_state *ds, const char *error)
{
	if(error==0) // just clear everything
	{
		ds->error_string=0;
		ds->error_idx=0;
		ds->error_char=0;
		ds->error_line=0;
		return;
	}
	
	if( ds->error_string ) { return; } // keep first error

	ds->error_string=error;
	ds->error_idx=0;
	ds->error_char=0;
	ds->error_line=0;

	if(!ds->data){ return; } // must have some parsing data to locate

	ds->error_idx=ds->parse_idx;

	int x=0;
	int y=0;
	char * cp;
//	char * cp_start = ds->data;
	char * cp_end   = ds->data+ds->data_len;
	char * cp_error = ds->data+ds->parse_idx;

	y++; // 1st line
	for( cp = ds->data ; cp<=cp_end ; cp++ )
	{
		x++;
		if( cp >= cp_error ) // found it
		{
			ds->error_char=x;
			ds->error_line=y;
			return;
		}
		if(*cp=='\n') // next line
		{
			x=0;
			y++;
		}
	}
	// error is at end of file
	ds->error_char=x;
	ds->error_line=y;
	return;
}

// allocate a new parsing state
djon_state * djon_setup()
{
	void *memctx=DJON_MEM(0,0,0,0); // get allocator ctx
	if(!memctx){ return 0; }

	djon_state *ds;

	ds=(djon_state *)DJON_MEM( memctx , 0 , 0 , sizeof(djon_state) );
	if(!ds) { return 0; }
	ds->memctx=memctx; // can now use DJON_MEM_*(ds,...)

	ds->data=0;
	ds->data_len=0;
	ds->parse_stack=0;
	ds->parse_idx=0;
	ds->parse_com=0;
	ds->parse_com_last=0;

	ds->error_string=0;
	ds->error_idx=0;
	ds->error_char=0;
	ds->error_line=0;

	ds->write=&djon_write_fp; // default write function to file
	ds->fp=0;

//	ds->write=&djon_write_data; // or write to string
	ds->write_data=0;
	ds->write_size=0;
	ds->write_len=0;

	ds->strict=0;
	ds->compact=0;
	ds->small=128;

	ds->values_len=1; // first value is used as a null so start at 1
	ds->values_reset=0; // reset search
	ds->values_inc=0; // reset search
	ds->values_siz=(DJON_VALUE_CHUNK_SIZE);
	ds->values=(djon_value *)DJON_MEM_MALLOC(ds, ds->values_siz * sizeof(djon_value) );
	if(!ds->values) { djon_clean(ds); return 0; }

	return ds;
}
// free a new parsing state
void djon_clean(djon_state *ds)
{
	if(!ds) { return; }
void *memctx=ds->memctx;
	if(ds->data)       { DJON_MEM_FREE(ds, ds->data       , ds->data_len   ); }
	if(ds->values)     { DJON_MEM_FREE(ds, ds->values     , ds->values_siz * sizeof(djon_value) ); }
	if(ds->write_data) { DJON_MEM_FREE(ds, ds->write_data , ds->write_size ); }
	DJON_MEM_FREE(ds,ds,sizeof(djon_state)); // free our structure
	DJON_MEM( memctx , 0 , 0 , 0 ); // free the ctx we allocated at the start
}

// get a index from pointer
int djon_idx(djon_state *ds,djon_value *dv)
{
	if(!dv){return 0;}
	return dv - ds->values;
}

// get a value by index and check if index is valid
// returns 0 for invalid index
djon_value * djon_get(djon_state *ds,int idx)
{
	if( idx <= 0 )              { return 0; }
	if( idx >= ds->values_siz ) { return 0; }
	return ds->values + idx ;
}


void djon_value_set(djon_state *ds, int di, djon_enum typ, double num, int len, const char *str)
{
	djon_value_dechild(ds,di,0); // in case this was an object or array
	djon_value *dv=djon_get(ds,di);
	if( (len<=0) && str ) // auto length
	{
		len=0;
		for( const char *cp=str; *cp ; cp++ ){ len++; }
	}
	if(!dv) { return; } dv->typ=typ; dv->num=num; dv->len=len; dv->str=(char*)str;
}

// copy string into dest array which has a sizeof siz
// return length of string now in dest, assume clipped if this is siz-1
int djon_value_copy_str(djon_state *ds,  int di , char *dest , int siz )
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return -1; }
	int len=dv->len;
	if(len>(siz-1)) { len=siz-1; } // overflow
	for( char *ca=dest, *cb=(dest+len), *cc=dv->str ; ca<cb ; ca++,cc++ ) { *ca=*cc; } // copy
	dest[len]=0; // terminate
	return len;
}

const char * djon_value_get_str(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return ""; } return dv->str;
}
int djon_value_get_len(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->len;
}
double djon_value_get_num(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0.0; } return dv->num;
}
int djon_value_get_typ(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->typ;
}
int djon_value_get_nxt(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->nxt;
}
int djon_value_get_prv(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->prv;
}
int djon_value_get_first(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->lst;
}
int djon_value_get_parent(djon_state *ds,  int di)
{
	djon_value *dv=djon_get(ds,di);
	if(!dv) { return 0; } return dv->par;
}

void djon_path_push_string(djon_state *ds, char *str, int len)
{
	int i;
	
	if( (ds->path_len+len) > (DJON_MAX_PATH-1) )
	{ return; }
	
	for(i=0;i<len;i++)
	{
		ds->buf[ ((DJON_MAX_PATH-1)-ds->path_len)+i-len ] = str[i];
	}
	ds->path_len+=len;
}

void djon_path_push_escaped_string(djon_state *ds, char *str, int len)
{
	char buf[DJON_MAX_PATH]; // copy/escape into this buffer
		
	char c;
	int cr;
	int cw;
	
	int need_to_escape=0;
	for( cr=0 , cw=0 ; cr<len ; cr++ , cw++ ) // try and copy string without escaping
	{
		c = str[cr] ;
		if( (c=='"') || (c=='\'') || (c=='.') || (c=='[') || (c==']') ) // need to be inside "
		{
			need_to_escape=1;
			break; // give up
		}
		buf[cw] = c ;
		if( cw > DJON_MAX_PATH-1 )	{ return; }
	}
	if(need_to_escape) // copy again but escape it this time
	{
		cw=0;
		buf[cw++] = '"' ;
		if( cw > DJON_MAX_PATH-1 )	{ return; }
		
		for( cr=0 ; cr<len ; cr++ ) // copy string escaping it
		{
			c = str[cr] ;
			if( (c=='"') || (c=='\\') ) // must escape
			{
				buf[cw++] = '\\' ;
				if( cw > DJON_MAX_PATH-1 )	{ return; }
			}
			buf[cw++] = c ;
			if( cw > DJON_MAX_PATH-1 )	{ return; }
		}

		buf[cw++] = '"' ;
		if( cw > DJON_MAX_PATH-1 )	{ return; }
	}
	djon_path_push_string(ds,buf,cw);
}

// get a path from a value
// pointer returned will be ds->buf so you do not have to free it
// but you do have to dupe its data if you want to keep it around.
// It should be considered invalid after any call to a djon function.
// we may set an error so be sure to check ds->error_string if we return a 0
const char * djon_value_to_path(djon_state *ds, int base_idx , int idx)
{
	djon_value *sub_val;
	djon_value *val;
	int val_idx;
	
	char num_buf[32];
	int num_len;
	
	char buf[DJON_MAX_PATH];

	ds->path_len=0; // start path
	ds->buf[(DJON_MAX_PATH-1)]=0; // null

	val_idx=idx;
	val=djon_get(ds,val_idx);
	if(!val) { return 0; }
	while( val && (val_idx!=base_idx) )
	{
		if( ! val->par ) { break; } // reached the top
		sub_val=val;
		val_idx=val->par;
		val=djon_get(ds,val_idx);
		if(!val) { return 0; }
		if((val->typ&DJON_TYPEMASK)==DJON_OBJECT)
		{
			djon_path_push_escaped_string(ds,sub_val->str,sub_val->len);
			if(ds->error_string){return 0;}

			if( val->par && (val->par!=base_idx) ) // need another seperator
			{
				djon_path_push_string(ds,".",1); // seperator
				if(ds->error_string){return 0;}
			}
		}
		else
		if((val->typ&DJON_TYPEMASK)==DJON_ARRAY)
		{
			num_len = djon_double_to_str_internal( ((double)(sub_val->idx)) , num_buf , 1 );

			djon_path_push_string(ds,num_buf,num_len);
			if(ds->error_string){return 0;}

			if( val->par && (val->par!=base_idx) ) // need another seperator
			{
				djon_path_push_string(ds,".",1);
				if(ds->error_string){return 0;}
			}
		}

	}
	
	return ds->buf+((DJON_MAX_PATH-1)-ds->path_len);
}

// get array(base_idx)[index] as a given index , 0 is the first etc..
int djon_value_by_index(djon_state *ds, int base_idx , int index)
{
	if((djon_value_get_typ(ds,base_idx)&DJON_TYPEMASK)!=DJON_ARRAY)	{ return 0; }
	for( int i=0 , ai=djon_value_get_first(ds,base_idx) ; ai ; i++ , ai=djon_value_get_nxt(ds,ai) )
	{
		if( i==index ) { return ai; } // found it
	}
	return 0;
}

// next idx may be a child or sibling, used to iterate of a recursive set
int djon_value_all(djon_state *ds, int base_idx, int di )
{
	if(di==0){ return 0; } // sanity

	int fi=djon_value_get_first(ds,di); // children
	if(fi) { return fi; }

	int ni=djon_value_get_nxt(ds,di); // siblings
	if(ni) { return ni; }
	
	while(di)
	{
		if( di == base_idx ) { return 0; } // do not recurse upwards past base_idx
		int pi=djon_value_get_parent(ds,di);
		if(!pi) { return 0; }
		di=pi; // parent
		int ni=djon_value_get_nxt(ds,di); // return next sibling only
		if(ni) { return ni; }
	}

	return 0;
}

// get a value index by path, path must be less than DJON_MAX_PATH including its null.
// path may be "part.part[part]" and each part may be wrapped in " or ' with /escapes 
// the empty string "" gets you the base value and "." gets you  base[""][""] value
// if lastpart is set then we return the idx for the part before last and
// write a char * into lastpart which will be the last part of the path string
int djon_value_by_path(djon_state *ds, int base_idx , const char *path, const char **lastkey)
{
	if(!lastkey) // cant shortcut a lastkey request
	{
		// shortcut so a null path or "" path returns base_idx
		if(!path){return base_idx;}
		if(!path[0]){return base_idx;}
	}

	djon_value *val;
	djon_value *key;
	int val_idx=0;
	int key_idx=0;
	int idx=0;
	int i=0;

	char buf[DJON_MAX_PATH]; // need buf to escape strings
	int buf_len=0;
	
	int brackets=0;
	char mode='.';
	int fetch=0;
	int len;
	char c;
	const char *cp;
	const char *oldcp;
	for(cp=path,len=0;*cp;cp++){len++;} // find length
	if( len>(DJON_MAX_PATH-1) ) { return 0; } // path too big

	val_idx=base_idx;
	val=djon_get(ds,val_idx);
	oldcp=path;
	for( cp=path; cp<=(path+len); )
	{
		c=*cp++; // probably one at a time, but sometimes two as we deal with escapes
		switch(mode)
		{
			case '"': // part in quotes
			case '\'': // part in quote
				if(c==mode) { mode='.'; } // end with mode quote
				else
				if(c=='\\') { buf[buf_len++]=*cp++; }  // escape, next char is raw (maybe 0 term)
				else
				{ buf[buf_len++]=c; } // this char
			break;

			case '.': // raw part?
				if( c==']' ) // closing brackets?
				{
					if( brackets!=0 ) // if we are looking for them
					{
						brackets=0;
						fetch=1;
					}
					else
					{
						return 0;
					}
				}
				else
				if(c=='"') { mode='"'; } // start quotes
				else
				if(c=='\'') { mode='\''; } // start quote
				else
				if( c=='[' ) // new part in brackets
				{
					brackets=1;
					if(buf_len>0) // might be the end of a previous part
					{
						fetch=1; // do fetch
					}
				}
				else
				if( c=='.' || c==0 ) // part seperator
				{
					fetch=1; // do fetch
				}
				else // raw
				{
					buf[buf_len++]=c;
				}
			break;
		}
		
		if(fetch && (brackets==0) )
		{
			buf[buf_len]=0; // null term
			if(!val) // invalid value
			{
				return 0;
			}

			if(cp==(path+len+1)) // last part
			{
				if(lastkey)
				{
					*lastkey=oldcp;
					return val_idx; // first part
				}
			}
			oldcp=cp;

			if((val->typ&DJON_TYPEMASK)==DJON_OBJECT)
			{
				key_idx=val->lst;
				while(key_idx)
				{
					key=djon_get(ds,key_idx);
					if( djon_compare_string( key->str , key->len , buf ) ) // found key
					{
						val_idx=key->lst;
						val=djon_get(ds,val_idx);
						break;
					}
					else
					{
						key_idx=key->nxt;
					}
				}
				if(key_idx==0) // not found
				{
					return 0;
				}
			}
			else
			if((val->typ&DJON_TYPEMASK)==DJON_ARRAY)
			{
				i=(int)djon_str_to_number(ds,buf,0);

				val_idx=val->lst;
				while(val_idx)
				{
					val=djon_get(ds,val_idx);
					if(val->idx==i)
					{ break; } // found it
					else
					{ val_idx=val->nxt; }
				}
				if(val_idx==0) // not found
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}

			fetch=0;
			buf_len=0;
		}
	}
	if( brackets!=0 ) // if we are still looking
	{
		return 0;
	}
	return val_idx;
}

int djon_value_newkey( djon_state *ds, int base_idx, const char *path, const char *key)
{
	int oi=djon_value_by_path(ds,base_idx,path,0);
	djon_value *ov=djon_get(ds,oi);
	if(!ov){ return 0; }

	if((ov->typ&DJON_TYPEMASK)!=DJON_OBJECT) // must be an object
	{
		return 0;
	}
	
	// first see if we can find the key
	int fi=djon_value_by_path(ds,oi,key,0);
	if(fi) // replace this value
	{
		return fi;
	}
	
	// add a new key and value to this object

	int ki=djon_alloc(ds); // allocate key
	if(!ki){return 0;}
	int vi=djon_alloc(ds); // allocate value
	if(!vi){djon_free(ds,ki);return 0;}
	djon_value *vv=djon_get(ds,vi);
	vv->par=ki;
	djon_value *kv=djon_get(ds,ki);
	kv->par=oi;
	kv->typ=DJON_KEY;
	kv->lst=vi;
	kv->str=(char*)key;
	kv->len=0;
	for(const char *cp=key;*cp;cp++){kv->len++;} // length of key

	ov=djon_get(ds,oi); // must refresh after allocation
	if( !ov->lst )
	{
		ov->lst=ki;
	}
	else
	{
		int ci=0;
		for( ci=ov->lst ; djon_get(ds,ci)->nxt ; ci=djon_get(ds,ci)->nxt  ){}
		djon_value *cv=djon_get(ds,ci);
		cv->nxt=ki;
		djon_value *kv=djon_get(ds,ki);
		kv->prv=ci;
	}

	return vi;
}

int djon_value_newindex( djon_state *ds, int base_idx, const char *path, int index)
{
	int oi=djon_value_by_path(ds,base_idx,path,0);
	djon_value *ov=djon_get(ds,oi);
	if(!ov){ djon_free(ds,oi); return 0; }

	if((ov->typ&DJON_TYPEMASK)!=DJON_ARRAY) // must be an array
	{
		return 0;
	}
	
	// first see if we can find the key
	int fi=djon_value_by_index(ds,oi,index);
	if(fi) // replace this value
	{
		return fi;
	}
	
	// add a new values to this array

	ov=djon_get(ds,oi); // must refresh after allocation
	int i=0;
	int ai=ov->lst;
	if(!ai) { ai=djon_alloc(ds); if(!ai){return 0;} }
	djon_get(ds,ai)->idx=i;
	djon_get(ds,ai)->par=oi;
	djon_get(ds,oi)->lst=ai; // first
	for( int ni=0,ci=0 ; ( (i<=index) || (index==-1) ) ; i++ , ai=ni )
	{
		int li=ai;
		if(i==index) // done
		{
			return ai; // return the array element we allocated
		}
		ni=djon_value_get_nxt(ds,ai);
		if(!ni) { ni=djon_alloc(ds); if(!ni){return 0;} ci=ni; }
		djon_get(ds,ni)->prv=ai;
		djon_get(ds,ai)->nxt=ni; // next
		djon_get(ds,ai)->idx=i;
		djon_get(ds,ai)->par=oi;
		if( (index==-1) && ci ) // return first index we allocate ( added to end of array )
		{
			return ai;
		}
	}
	return 0;
}


// delete all children and their comments of this item ( but not comments on *this* item )
// this prepares a value for its type to be safely changed, eg to a string from an object
void djon_value_dechild(   djon_state *ds, int base_idx, const char *path)
{
	int vi=djon_value_by_path(ds,base_idx,path,0);
	djon_value *vv=djon_get(ds,vi);
	if(!vv){return;}

	if(	((vv->typ&DJON_TYPEMASK)==DJON_ARRAY)   ||
		((vv->typ&DJON_TYPEMASK)==DJON_OBJECT)  ||
		((vv->typ&DJON_TYPEMASK)==DJON_KEY)     ) // possible value in lst
	{
		for(  int ci=vv->lst , cv_nxt  ;  ci  ;  ci=cv_nxt  )
		{
			djon_value *cv=djon_get(ds,ci);
			cv_nxt=cv->nxt; // get next vefore we delete
			djon_value_delete(ds,ci,0);
		}
		vv->lst=0;
	}
	return;
}
// delete this item and all children and all comments
void djon_value_delete(   djon_state *ds, int base_idx, const char *path)
{
	// remove children recursivly
	djon_value_dechild(ds,base_idx,path);

	int vi=djon_value_by_path(ds,base_idx,path,0);
	djon_value *vv=djon_get(ds,vi);
	if(!vv){return;}

	// delete comments
	for(  int ci=vv->com , cv_com  ;  ci  ;  ci=cv_com  )
	{
		djon_value *cv=djon_get(ds,ci);
		cv_com=cv->com; // get next before we free
		djon_free(ds,ci);
	}
	vv->com=0;

	djon_free(ds,vi);
	return;
}

// guess the output size of outputting data so we can use compact format for small
// arrays and objects, small is your character count idea of small, bucket 
// must start at 0 and is returned after adding the size
int djon_value_is_small(djon_state *ds,int idx,int small,int bucket)
{
	if(bucket>small){return bucket;}

	int vi=idx;
	djon_value *vv=djon_get(ds,vi);
	if(!vv){return bucket;}

	if(	vv->com && (bucket>0) ) { return bucket+small; } // give up on any sub comments

	if(	vv->lst )
	{
		bucket+=2; // add minimum array/string size
		if((vv->typ&DJON_TYPEMASK)==DJON_STRING) // its a key
		{
			bucket+=vv->len;
		}
		if(bucket>small){return bucket;}
		for( int ci=vv->lst ; ci ; ci=djon_value_get_nxt(ds,ci) )
		{
			bucket=djon_value_is_small(ds,ci,small,bucket);
			if(bucket>small){return bucket;}
		}
	}
	else
	if((vv->typ&DJON_TYPEMASK)==DJON_STRING)
	{
		bucket+=2+vv->len;
	}
	else
	if((vv->typ&DJON_TYPEMASK)==DJON_NUMBER)
	{
		bucket+=10; // average number size dont bother with precision
	}
	else
	if((vv->typ&DJON_TYPEMASK)==DJON_BOOL)
	{
		bucket+=vv->num?5:6;
	}
	else
	if((vv->typ&DJON_TYPEMASK)==DJON_NULL)
	{
		bucket+=4;
	}
	return bucket;
}

// unallocate unused values at the end of a parse
void djon_shrink(djon_state *ds)
{
	djon_value * v;
	v=(djon_value *)DJON_MEM_REALLOC(ds, (void*)ds->values , ds->values_siz * sizeof(djon_value) , (ds->values_len) * sizeof(djon_value) );
	if(!v) { return; } //  fail but we can ignore
	ds->values_siz=ds->values_len;
}

// mark this value as free, it may be reused next time we allocate one
void djon_free(djon_state *ds,int idx)
{
	if(idx<ds->values_reset) { ds->values_reset=idx; } // reset reset
	djon_value * v=djon_get(ds,idx);
	if( v )
	{
		v->typ=DJON_FREE; // flag as free
	}
}

// allocate a new value and return its index, 0 on error
// note that everytime we call this function all djon_value ptrs are invalidated.
int djon_alloc(djon_state *ds)
{
	djon_value * v=0;
	for(int i=0;i<16;i++) // first do a quick search for previously freed values
	{
		if(ds->values_inc==ds->values_reset) { ds->values_reset++; } // auto advance reset
		ds->values_inc++; // simple wrapping search pointer
		if( ds->values_inc >= ds->values_len ) // reset and give up
		{
			ds->values_inc=ds->values_reset;
			v=0;
			break;
		}
		else
		{
			v=djon_get(ds,ds->values_inc);
			if(v->typ==DJON_FREE) { break; } // found one
			v=0;
		}
	}
	if(!v) // need another one, last one didnt do me no good
	{
		if( ds->values_len+1 >= ds->values_siz ) // check for space
		{
			v=(djon_value *)DJON_MEM_REALLOC(ds, (void*)ds->values , ds->values_siz * sizeof(djon_value) , (ds->values_siz+(DJON_VALUE_CHUNK_SIZE)) * sizeof(djon_value) );
			if(!v) { djon_set_error(ds,"out of memory"); return 0; }
			ds->values_siz=ds->values_siz+(DJON_VALUE_CHUNK_SIZE);
			ds->values=v; // might change pointer
		}
		v=djon_get(ds,ds->values_len++);
	}
	
	// clear value data
	v->typ=DJON_NULL;
	v->str=0;
	v->len=0;
	v->num=0.0;
	v->nxt=0;
	v->prv=0;
	v->com=0;
	v->lst=0;
	v->par=0;
	v->idx=0;

	return djon_idx(ds,v);
}

// apply current cached comment chain to this value
void djon_apply_comments(djon_state *ds, int idx)
{
	djon_value * v;
	if( ds->parse_com ) // we have a comment to save
	{
		v=djon_get(ds,idx);
		while(v && v->com) // need to append to end of comment list
		{
			v=djon_get(ds,v->com);
		}
		if(v)
		{
			v->com=ds->parse_com;
			ds->parse_com=0;
			ds->parse_com_last=0;
		}
	}
}

// write to fp
int djon_write_fp(djon_state *ds, const char *ptr, int len )
{
#if !DJON_FILE
	return 0;
#else
	fwrite(ptr, 1, len, ds->fp);
	return 0;
#endif
}

// write to realloced string buffer
int djon_write_data(djon_state *ds, const char *ptr, int len )
{
	if(ds->error_string){return -1;} // already have error
	
	if(!ds->write_data) // first alloc
	{
		ds->write_data = (char*) DJON_MEM_MALLOC(ds,DJON_STRING_CHUNK_SIZE); // 64k chunks
		if(!ds->write_data) { djon_set_error(ds,"out of memory"); return -1; }
		ds->write_size=DJON_STRING_CHUNK_SIZE;
		ds->write_len=0;
	}
	int idx=ds->write_len;
	while( ds->write_len + len + 1 > ds->write_size ) // realloc
	{
		ds->write_data=(char*)DJON_MEM_REALLOC(ds,ds->write_data,ds->write_size,ds->write_size+(DJON_STRING_CHUNK_SIZE));
		if(!ds->write_data) { djon_set_error(ds,"out of memory"); return -1; }
		ds->write_size+=DJON_STRING_CHUNK_SIZE;
	}
	
	// copy into data buffer
	if(ptr)
	{
		DJON_MEMCPY(ds->write_data+ds->write_len,ptr,len);
	}
	ds->write_len+=len;
	ds->write_data[ds->write_len]=0; // keep null terminated

	return idx;
}


void djon_write_it(djon_state *ds, const char *ptr, int len )
{
	if(ds->write)
	{
		(*ds->write)(ds,ptr,len);
	}
}

void djon_write_char(djon_state *ds, char c)
{
	if(ds->write)
	{
		(*ds->write)(ds,&c,1);
	}
}

// find length of a null term string and write it
void djon_write_string(djon_state *ds, const char *ptr)
{
	int len=0;
	const char *cp=ptr;
	while( *cp ){ len++; cp++; }
	if(ds->write)
	{
		(*ds->write)(ds,ptr,len);
	}
}

// write this char (16bit maybe) as a string escape sequence
void djon_write_string_escape(djon_state *ds,int c)
{
	char b[16];
	switch(c)
	{
		case '\\' : djon_write_string(ds,"\\\\"); break;
		case '\b' : djon_write_string(ds,"\\b" ); break;
		case '\f' : djon_write_string(ds,"\\f" ); break;
		case '\n' : djon_write_string(ds,"\\n" ); break;
		case '\r' : djon_write_string(ds,"\\r" ); break;
		case '\t' : djon_write_string(ds,"\\t" ); break;
		case '\'' : djon_write_string(ds,"\\'" ); break;
		case '"'  : djon_write_string(ds,"\\\""); break;
		case '`'  : djon_write_string(ds,"\\`" ); break;
		default:
			b[0]='\\';
			b[1]='u';
			djon_int_to_hexstr(c&0xffff,4,b+2);
			djon_write_string(ds,b);
		break;
	}
}

int djon_write_indent(djon_state *ds,int indent)
{
	if(indent<0) { return -indent; } // skip first indent
	else
	{
		if(!ds->compact)
		{
			int i;
			for( i=0 ; i<indent ; i++)
			{
				djon_write_char(ds,' ');
			}
		}
	}
	return indent;
}
// write json with indent state
void djon_write_json_indent(djon_state *ds,int idx,int indent,char *coma)
{
	if( ds->error_string ) { return; } // error

	djon_value *v=djon_get(ds,idx);
	djon_value *key;
	djon_value *val;
	int key_idx;
	int val_idx;
	int len;
	char *cp;
	char *ce;
	char c;
	if(v)
	{
		if(ds->compact)
		{
			if(!coma){coma=(char*)(v->nxt?",":"");} // auto coma
		}
		else
		{
			if(!coma){coma=(char*)(v->nxt?" ,":"");} // auto coma
		}
		if((v->typ&DJON_TYPEMASK)==DJON_ARRAY)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"[");
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
			val_idx=v->lst; val=djon_get(ds,val_idx);
			while(val)
			{
				djon_write_json_indent(ds,val_idx,indent+1,0);
				val_idx=val->nxt; val=djon_get(ds,val_idx);
			}
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"]");
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_OBJECT)
		{
			djon_sort_object(ds,idx); // sort
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"{");
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
			key_idx=v->lst; key=djon_get(ds,key_idx);
			while(key)
			{
				indent=djon_write_indent(ds,indent+1)-1;
				djon_write_string(ds,"\"");
				for( cp=key->str,ce=key->str+key->len ; cp<ce ; cp++ )
				{
					c=*cp;
					if( ( (c>=0x00)&&(c<=0x1F) ) || (c=='"') || (c=='\\') ) // must escape
					{
						djon_write_string_escape(ds,c);
					}
					else
					{
						djon_write_char(ds,c);
					}
				}
				if(ds->compact)
				{
					djon_write_string(ds,"\":");
					djon_write_json_indent(ds,key->lst,-(indent+1),(char*)(key->nxt?",":""));
				}
				else
				{
					djon_write_string(ds,"\" : ");
					djon_write_json_indent(ds,key->lst,-(indent+1),(char*)(key->nxt?" ,":""));
				}
				key_idx=key->nxt; key=djon_get(ds,key_idx);
			}
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"}");
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_STRING)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"\"");
			for( cp=v->str,ce=v->str+v->len ; cp<ce ; cp++ )
			{
				c=*cp;
				if( ( (c>=0x00)&&(c<=0x1F) ) || (c=='"') || (c=='\\') ) // must escape
				{
					djon_write_string_escape(ds,c);
				}
				else
				{
					djon_write_char(ds,c);
				}
			}
			djon_write_string(ds,"\"");
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_NUMBER)
		{
			indent=djon_write_indent(ds,indent);
			len=djon_double_to_str(v->num,ds->buf);
			djon_write_it(ds,ds->buf,len);
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_BOOL)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,(char*)(v->num?"true":"false"));
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_NULL)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"null");
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
		else // should not get here
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"null");
			djon_write_string(ds,coma);
			if(!ds->compact)
			{
				djon_write_string(ds,"\n");
			}
		}
	}
}
// write json to the write function
void djon_write_json(djon_state *ds,int idx)
{
	djon_set_error(ds,0);// clear error state
	djon_write_json_indent(ds,idx,0,0);
}

// write djon to the given file handle
void djon_write_djon_indent(djon_state *ds,int idx,int indent)
{
	if( ds->error_string ) { return; } // error

	djon_value *v=djon_get(ds,idx);
	djon_value *key;
	djon_value *val;
	djon_value *com;
	int key_idx;
	int val_idx;
	int com_idx;
	int len;
	char *qs;
	char *cp;
	char c;
	if(v)
	{
		if( ((v->typ&DJON_TYPEMASK)!=DJON_COMMENT) && (v->com) )
		{
			key=djon_get(ds,v->par);
			if( key && ( ((key->typ&DJON_FLAGMASK)==DJON_KEY) || ((key->typ&DJON_TYPEMASK)==DJON_ARRAY) ) )
			{
				// already printed
			}
			else
			{
				djon_write_djon_indent(ds,v->com,indent);
			}
		}

		if((v->typ&DJON_TYPEMASK)==DJON_COMMENT)
		{
			len=0;
			for( com_idx=idx ; (com=djon_get(ds,com_idx)) ; com_idx=com->com )
			{
				len+=com->len;
			}
			if(len>0) // ignore a completely empty comment chain
			{
				for( com_idx=idx ; (com=djon_get(ds,com_idx)) ; com_idx=com->com )
				{
					indent=djon_write_indent(ds,indent);
					djon_write_string(ds,"// ");
					for( cp=com->str ; cp<com->str+com->len ; cp++ )
					{
						c=*cp;
						if(c=='\n')
						{
							djon_write_string(ds,"\n");
							indent=djon_write_indent(ds,indent);
							djon_write_string(ds,"// ");
						}
						else
						{
							djon_write_char(ds,c);
						}
					}
					djon_write_string(ds,"\n");
				}
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_ARRAY)
		{
			indent=djon_write_indent(ds,indent);
			int ds_compact=ds->compact; // save
			if(djon_value_is_small(ds,idx,ds->small,0)<=ds->small) { ds->compact=1; }
			djon_write_string(ds,"[");
			djon_write_string(ds,ds->compact?" ":"\n");
			val_idx=v->lst; val=djon_get(ds,val_idx);
			while(val)
			{
				if(val->com)
				{
					djon_write_djon_indent(ds,val->com,indent+1);
				}
				djon_write_djon_indent(ds,val_idx,indent+1);
				val_idx=val->nxt; val=djon_get(ds,val_idx);
			}
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"]");
			ds->compact=ds_compact; // unsave
			djon_write_string(ds,ds->compact?" ":"\n");
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_OBJECT)
		{
			indent=djon_write_indent(ds,indent);
			int ds_compact=ds->compact; // save
			if(djon_value_is_small(ds,idx,ds->small,0)<=ds->small) { ds->compact=1; }
			djon_sort_object(ds,idx); // sort
			djon_write_string(ds,"{");
			djon_write_string(ds,ds->compact?" ":"\n");
			key_idx=v->lst; key=djon_get(ds,key_idx);
			while(key)
			{
				val=djon_get(ds,key->lst);
				if(key->com)
				{
					djon_write_djon_indent(ds,key->com,indent+1);
				}
				if(val->com)
				{
					djon_write_djon_indent(ds,val->com,indent+1);
				}
				indent=djon_write_indent(ds,indent+1)-1;
				if( djon_is_naked_key(key->str,key->len) )
				{
					djon_write_it(ds,key->str,key->len);
					if(ds->compact)
					{
						djon_write_string(ds,"=");
					}
					else
					{
						djon_write_string(ds," = ");
					}
				}
				else
				{
					qs=djon_pick_quote(key->str,key->len,ds->buf);
					if(!qs){ djon_set_error(ds,"quote attack"); return; }
					djon_write_string(ds,qs);
					if(key->len>0 && ( ( key->str[0]=='\n' ) || ( key->str[key->len-1]=='\n' ) ) )
					{
						djon_write_string(ds,"\n");
					}
					djon_write_it(ds,key->str,key->len);
					djon_write_string(ds,qs);
					if(ds->compact)
					{
						djon_write_string(ds,"=");
					}
					else
					{
						djon_write_string(ds," = ");
					}
				}
				djon_write_djon_indent(ds,key->lst,-(indent+1));
				key_idx=key->nxt; key=djon_get(ds,key_idx);
			}
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"}");
			ds->compact=ds_compact; // unsave
			djon_write_string(ds,ds->compact?" ":"\n");
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_STRING)
		{
			// no naked strings in compact djon
			if( (!ds->compact) && djon_is_naked_string(v->str,v->len) )
			{
				indent=djon_write_indent(ds,indent);
				djon_write_it(ds,v->str,v->len);
				djon_write_string(ds,"\n");
			}
			else
			{
				indent=djon_write_indent(ds,indent);
				qs=djon_pick_quote(v->str,v->len,ds->buf);
				if(!qs){ djon_set_error(ds,"quote attack"); return; }
				djon_write_string(ds,qs);
				// a multiline string begins or ends with '\n'
				// add an extra '\n' at the front for presentation which will be removed when parsed.
				if(v->len>0 && ( ( v->str[0]=='\n' ) || ( v->str[v->len-1]=='\n' ) ) )
				{
					djon_write_string(ds,"\n");
				}
				djon_write_it(ds,v->str,v->len);
				djon_write_string(ds,qs);
				djon_write_string(ds,ds->compact?" ":"\n");
			}
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_NUMBER)
		{
			indent=djon_write_indent(ds,indent);
			len=djon_double_to_str(v->num,ds->buf);
			djon_write_it(ds,ds->buf,len);
			djon_write_string(ds,ds->compact?" ":"\n");
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_BOOL)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,(char*)(v->num?"TRUE":"FALSE"));
			djon_write_string(ds,ds->compact?" ":"\n");
		}
		else
		if((v->typ&DJON_TYPEMASK)==DJON_NULL)
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"NULL");
			djon_write_string(ds,ds->compact?" ":"\n");
		}
		else
		{
			indent=djon_write_indent(ds,indent);
			djon_write_string(ds,"NULL");
			djon_write_string(ds,ds->compact?" ":"\n");
		}
	}
}
// write djon to the write function
void djon_write_djon(djon_state *ds,int idx)
{
	djon_set_error(ds,0);// clear error state
	djon_write_djon_indent(ds,idx,0);
}

int djon_dupe_value(djon_state *ds,int idx)
{
	int val_idx=djon_alloc(ds);
	djon_value *old=djon_get(ds,idx); if(!old) { return 0; }
	djon_value *val=djon_get(ds,val_idx); if(!val) { return 0; }
	val->typ=old->typ;
	val->str=old->str;
	val->len=old->len;
	val->num=old->num;
	val->idx=old->idx;
	return val_idx;
}

// recursivly convert a value into a vca
// a vca is an array wrapper where the first value is the
// value and the rest are string comments for this value
// returns new value idx to replace old value with
int djon_value_to_vca(djon_state *ds,int idx)
{
	int lst_idx;
	int new_idx;
	int old_idx;
	djon_value *old;
	int key_idx;
	djon_value *key;
	int val_idx;
	djon_value *val;
	int arr_idx;
	djon_value *arr;
	int com_idx;
	djon_value *com;
	int i;
	
	old=djon_get(ds,idx);
	if(!old) { return 0; }
	djon_value *par=djon_get(ds,old->par); // parent maybe key and might have comments

	arr_idx=0;
	if( (old->com) || ((old->typ&DJON_TYPEMASK)==DJON_ARRAY) || (par&&((par->typ&DJON_FLAGMASK)==DJON_KEY)&&par->com) ) // turn value into array with comments
	{
		com_idx=old->com ? old->com : ( par ? par->com : 0 );
		arr_idx=djon_alloc(ds); // this invalidates pointers
		if(!arr_idx) { return 0; }
		val_idx=djon_dupe_value(ds,idx); // this invalidates pointers
		if(!val_idx) { return 0; }

		// first item
		arr=djon_get(ds,arr_idx);
		arr->typ=DJON_ARRAY;
		arr->lst=val_idx;
		
		val=djon_get(ds,val_idx);
		val->par=arr_idx;
		val->idx=0;
		lst_idx=val_idx;
		
		// loop comments
		for( i=1 ; com_idx ; i++ , com_idx=djon_get(ds,com_idx)->com )
		{
			new_idx=djon_dupe_value(ds,com_idx); // this invalidates pointers
			if(!new_idx) { return 0; }
			val=djon_get(ds,new_idx);
			val->typ=DJON_STRING; // convert dupe from comment to string
			val->par=arr_idx;
			val->prv=lst_idx;
			val->idx=i;
			djon_get(ds,lst_idx)->nxt=new_idx;
			lst_idx=new_idx;
		}
	}
	else // just dupe
	{
		val_idx=djon_dupe_value(ds,idx); // this invalidates pointers
		if(!val_idx) { return 0; }
	}

	old=djon_get(ds,idx); // check for recursing
	if((old->typ&DJON_TYPEMASK)==DJON_ARRAY)
	{
		for( lst_idx=0 , i=0 , old_idx=old->lst ; old_idx ; i++ , old_idx=djon_get(ds,old_idx)->nxt )
		{
			new_idx=djon_value_to_vca(ds, old_idx);
			if(!new_idx) { return 0; }
			djon_get(ds,new_idx)->prv=lst_idx;
			djon_get(ds,new_idx)->idx=i;
			djon_get(ds,new_idx)->par=val_idx;
			if(!lst_idx) { djon_get(ds,val_idx)->lst=new_idx; } // first
			else         { djon_get(ds,lst_idx)->nxt=new_idx; }
			lst_idx=new_idx;
		}
	}
	else
	if((old->typ&DJON_TYPEMASK)==DJON_OBJECT)
	{
		for( lst_idx=0 , old_idx=old->lst ; old_idx ; old_idx=djon_get(ds,old_idx)->nxt )
		{
			key_idx=djon_dupe_value(ds, old_idx);
			if(!key_idx) { return 0; }
			djon_get(ds,key_idx)->par=val_idx;
			if(!lst_idx) { djon_get(ds,val_idx)->lst=key_idx; } // first
			else         { djon_get(ds,lst_idx)->nxt=key_idx; }
			new_idx=djon_value_to_vca(ds, djon_get(ds,old_idx)->lst );
			if(!new_idx) { return 0; }
			djon_get(ds,key_idx)->lst=new_idx;
			djon_get(ds,key_idx)->prv=lst_idx;
			djon_get(ds,new_idx)->par=key_idx;
			lst_idx=key_idx;
		}
	}
	
	return arr_idx ? arr_idx : val_idx ;
}

// recursivly move the comments back into the ->com from a vca
// returns new value idx to replace old value with
int djon_vca_to_value(djon_state *ds,int idx)
{
	int lst_idx;
	int new_idx;
	
	int old_idx;
	djon_value *old;
	int key_idx;
	djon_value *key;
	int newkey_idx;
	djon_value *newkey;
	int newval_idx;
	djon_value *newval;
	int newcom_idx;
	djon_value *newcom;
	int com_idx;
	djon_value *com;
	int i;
	
	old=djon_get(ds,idx);
	if(!old) { return 0; }

	if((old->typ&DJON_TYPEMASK)==DJON_ARRAY) // need to map back into comments
	{
		old_idx=old->lst;
		new_idx=djon_dupe_value(ds,old_idx); // this invalidates pointers
		if(!new_idx) { return 0; }
		
		for( lst_idx=new_idx , com_idx=djon_get(ds,old_idx)->nxt ; com_idx ; com_idx=djon_get(ds,com_idx)->nxt )
		{
			newcom_idx=djon_dupe_value(ds,com_idx); // this invalidates pointers
			if(!newcom_idx) { return 0; }
			newcom=djon_get(ds,newcom_idx);
			newcom->typ=DJON_COMMENT;
			djon_get(ds,lst_idx)->com=newcom_idx;
			lst_idx=newcom_idx;
		}
	}
	else // just dupe
	{
		old_idx=idx;
		new_idx=djon_dupe_value(ds,old_idx); // this invalidates pointers
		if(!new_idx) { return 0; }
	}
	old=djon_get(ds,old_idx); // check for recursing
	if((old->typ&DJON_TYPEMASK)==DJON_ARRAY)
	{
		for( lst_idx=0 , i=0 , key_idx=old->lst ; key_idx ; i++ , key_idx=djon_get(ds,key_idx)->nxt )
		{
			newkey_idx=djon_vca_to_value(ds, key_idx);
			if(!newkey_idx) { return 0; }
			if(!lst_idx) { djon_get(ds,new_idx)->lst=newkey_idx; } // first
			else         { djon_get(ds,lst_idx)->nxt=newkey_idx; }
			djon_get(ds,newkey_idx)->prv=lst_idx;
			djon_get(ds,newkey_idx)->idx=i;
			djon_get(ds,newkey_idx)->par=new_idx;
			lst_idx=newkey_idx;
		}
	}
	else
	if((old->typ&DJON_TYPEMASK)==DJON_OBJECT)
	{
		for( lst_idx=0 , i=0 , key_idx=old->lst ; key_idx ; i++ , key_idx=djon_get(ds,key_idx)->nxt )
		{
			newkey_idx=djon_dupe_value(ds, key_idx);
			if(!newkey_idx) { return 0; }
			if(!lst_idx) { djon_get(ds,new_idx)->lst=newkey_idx; } // first
			else         { djon_get(ds,lst_idx)->nxt=newkey_idx; }
			newval_idx=djon_vca_to_value(ds, djon_get(ds,key_idx)->lst );
			if(!newval_idx) { return 0; }
			djon_get(ds,newkey_idx)->par=new_idx;
			djon_get(ds,newkey_idx)->lst=newval_idx;
			djon_get(ds,newkey_idx)->prv=lst_idx;
			djon_get(ds,newval_idx)->par=newkey_idx;
			lst_idx=newkey_idx;
		}
	}
	return new_idx;
}

// load a new file or possibly from stdin , pipes allowed
int djon_load_file(djon_state *ds,const char *fname)
{
#if !DJON_FILE
	return 0;
#else
	const int chunk=DJON_STRING_CHUNK_SIZE; // read this much at once

	FILE * fp=0;
    char * temp;
    char * data=0;

    int size=0;
    int used=0;

	ds->data=0;
	ds->data_len=0;

	// first alloc
	size = used+chunk;
	data = (char*)DJON_MEM_MALLOC(ds,size); if(!data) { goto error; }

	// open file or use stdin
	if(fname)
	{
		fp = fopen( fname , "rb" ); if(!fp) { djon_set_error(ds,"file error"); goto error; }
	}
	else
	{
		fp=stdin;
	}

    while( !feof(fp) ) // read all file
    {
		// extend buffer
		while(used+chunk > size)
		{
			temp = (char*) DJON_MEM_REALLOC(ds, data, used,used+chunk); if(!temp) { djon_set_error(ds,"out of memory"); goto error; }
			size = used+chunk;
			data=temp;
		}
		used += fread( data+used , 1 , chunk, fp );
		if(ferror(fp)) { djon_set_error(ds,"file error"); goto error; }
    }


	temp = (char*) DJON_MEM_REALLOC(ds, data, used, used+1); if(!temp) { djon_set_error(ds,"out of memory"); goto error; }
	size = used+1; // this may size up or down
    data = temp;
    data[used] = 0; // null term

	ds->data=data;
	ds->data_len=used;

    return 1;

error:
	if(fname)
	{
		if(fp) { fclose(fp); }
	}
	if(data) { DJON_MEM_FREE(ds,data,size); }
    return 0;
#endif
}

// peek for whitespace or comments
int djon_peek_white(djon_state *ds)
{
	char c1=ds->data[ ds->parse_idx ];
	char c2=ds->data[ ds->parse_idx+1 ];
	if( DJON_IS_WHITESPACE(c1) )
	{
		return 1;
	}
	else
	if( (c1=='/') && (c2=='/'))
	{
		return 1;
	}
	else
	if( (c1=='/') && (c2=='*'))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// peek for punct
int djon_peek_punct(djon_state *ds,const char *punct)
{
	char c=ds->data[ ds->parse_idx ];
	const char *cp;
	for( cp=punct ; *cp ; cp++ )
	{
		if( c==*cp ) // a punctuation char
		{
			return 1;
		}
	}
	return 0;
}

// peek for a lowercase string eg, true false null
int djon_peek_string(djon_state *ds,const char *s)
{
	const char *sp;
	const char *dp;
	char d;
	for( sp=s , dp=ds->data+ds->parse_idx ; *sp && *dp ; sp++ , dp++ )
	{
		d=*dp;
		if( d>='A' && d<='Z' ) { d=d+('a'-'A'); } // lowercase
		if( d != *sp ) { return 0; } // no match
	}
	if( *sp==0 ) // we got to end of test string
	{
		d=*dp; // and text must be followed by some kind of deliminator
		if( DJON_IS_DELIMINATOR(d) )
		{
			return 1;
		}
	}
	return 0;
}

// allocate a new comment in the current comment chain
int djon_alloc_comment(djon_state *ds)
{
	// allocate new comment and place it in chain
	int com_idx=djon_alloc(ds);
	djon_value * com=djon_get(ds,com_idx);
	if(!com){return 0;}

	if( ds->parse_com_last ) // append
	{
		djon_value * v=djon_get(ds,ds->parse_com_last);
		v->com=com_idx;
		ds->parse_com_last=com_idx;
	}
	else // start a new one
	{
		ds->parse_com=com_idx;
		ds->parse_com_last=com_idx;
	}
	com->typ=DJON_COMMENT;
	com->str=ds->data+ds->parse_idx;
	com->len=0;

	return com_idx;
}

void djon_trim_comment(djon_state *ds,int idx)
{
	char c;
	djon_value *com=djon_get(ds,idx);
	if(com)
	{
		for( c=*com->str ; ( com->len>0 ) && ( DJON_IS_WHITESPACE(c) ) ; c=*com->str )
		{
			com->str++;
			com->len--;
		}
		for( c=com->str[com->len-1] ; ( com->len>0 ) && ( DJON_IS_WHITESPACE(c) ) ; c=com->str[com->len-1] )
		{
			com->len--;
		}
	}
}
// skip whitespace and comments return amount skipped
int djon_skip_white(djon_state *ds)
{
	int start=ds->parse_idx;
	int com_idx;
	djon_value * com;

	while( ds->parse_idx < ds->data_len )
	{
		char c1=ds->data[ ds->parse_idx ];
		char c2=ds->data[ ds->parse_idx+1 ];
		if( (c1==' ') || (c1=='\t') || (c1=='\n') || (c1=='\r') )
		{
			ds->parse_idx++;
		}
		else
		if( (c1=='/') && (c2=='/'))
		{
			ds->parse_idx+=2;

			// allocate new comment and place it in chain
			com_idx=djon_alloc_comment(ds);

			while( ds->parse_idx < ds->data_len )
			{
				c1=ds->data[ ds->parse_idx ];
				if(c1=='\n')
				{
					djon_trim_comment(ds,com_idx);
					ds->parse_idx++;
					break;
				}
				else
				{
					ds->parse_idx++;
					if(com_idx)
					{
						com=djon_get(ds,com_idx);
						if(com)
						{
							com->len++;
						}
					}
				}
			}
			// file ending counts as a \n so this is always OK
		}
		else
		if( (c1=='/') && (c2=='*'))
		{
			ds->parse_idx+=2;

			// allocate new comment and place it in chain
			com_idx=djon_alloc_comment(ds);
			int closed_comment=0;
			while( ds->parse_idx < ds->data_len )
			{
				c1=ds->data[ ds->parse_idx ];
				c2=ds->data[ ds->parse_idx+1 ];
				if( (c1=='*') && (c2=='/') )
				{
					djon_trim_comment(ds,com_idx);
					ds->parse_idx+=2;
					closed_comment=1;
					break;
				}
				else
				{
					ds->parse_idx++;
					if(c1=='\n') // chain a new comment at new line
					{
						djon_trim_comment(ds,com_idx);
						com_idx=djon_alloc_comment(ds);
					}
					else
					{
						if(com_idx)
						{
							com=djon_get(ds,com_idx);
							if(com)
							{
								com->len++;
							}
						}
					}
				}
			}
			if(!closed_comment)
			{
				djon_set_error(ds,"missing */");
			}
		}
		else
		{
			return ds->parse_idx-start;
		}
	}
	return ds->parse_idx-start;
}

// skip these punct chars only, counting thme
int djon_skip_punct(djon_state *ds,const char *punct)
{
	const char *cp;
	int ret=0;
	if( ds->parse_idx < ds->data_len )
	{
		char c=ds->data[ ds->parse_idx ];
		for( cp=punct ; *cp ; cp++ )
		{
			if( c==*cp ) // a punctuation char we wish to ignore
			{
				ds->parse_idx++;
				ret++; // count punctuation
				break;
			}
		}
	}
	return ret;
}
// skip these punct chars or white space , counting how many punct chars we found.
int djon_skip_white_punct(djon_state *ds,const char *punct)
{
	int ret=0;
	int progress=-1;

	while( ( progress != ds->parse_idx ) && ( ds->parse_idx < ds->data_len ) )
	{
		progress = ds->parse_idx;

		djon_skip_white(ds);
		ret+=djon_skip_punct(ds,punct);
		djon_skip_white(ds);

		// repeat until progress stops changing
	}

	return ret;
}

// allocate 0 length string value pointing at the next char
int djon_parse_next(djon_state *ds)
{
	if( ds->parse_idx >= ds->data_len ) { return 0; } // EOF

	djon_value *v;
	int idx=0;

	idx=djon_alloc(ds);
	v=djon_get(ds,idx);
	if(v==0) { return 0; }

	v->typ=DJON_STRING;
	v->str=ds->data+ds->parse_idx;
	v->len=0;

	djon_apply_comments(ds,idx); // apply any comments

	return idx;
}

int djon_parse_string(djon_state *ds,const char termchar)
{
	int str_idx=djon_parse_next(ds);
	djon_value *str=djon_get(ds,str_idx);
	if(!str) { return 0; }

	char c;
	const char *term=&termchar;
	const char *cp;
	const char *tp;
	int term_len=1; // assume 1 char

	if( *term=='`' ) // need to find special terminator
	{
		term=str->str;
		cp=term+1; // skip first char which is a `
		c=*cp; // next char
		if( (c=='\'') || (c=='"') )// not a single opener
		{
			while(1)
			{
				c=*cp++;
				if( (c=='\'') || (c=='"') ) { term_len++; } // allowable
				else
				if(c=='`') { term_len++; break; } // final
				else
				{
					term_len=1; // single ` only
					break;
				}
			}
		}
		str->typ=DJON_LONG|DJON_STRING; // maybe skip first/last newline
	}
	else
	if( *term!='\n' ) // need to escape non naked strings only
	{
		str->typ=DJON_ESCAPED|DJON_STRING; // deal with backslash escapes
	}

	if( *term != '\n' ) // need to skip opening quote if not \n terminated
	{
		ds->parse_idx+=term_len;
		str->str+=term_len;
		str->len=0;
	}

	while( ds->parse_idx < ds->data_len ) // while data
	{
		cp=ds->data+ds->parse_idx;
		c=*cp; // get next char

		if( str->typ==(DJON_ESCAPED|DJON_STRING) ) // we need to check for back slashes
		{
			if(c=='\\') // skip next char whatever it is
			{
				str->len+=2; // grow string
				ds->parse_idx+=2; // advance
				continue;
			}
		}

		for( tp=term ; tp<term+term_len ; tp++,cp++ ) // check for term
		{
			if(*tp!=*cp) { break; } // not found
			if(tp==term+term_len-1) // found full terminator
			{
				ds->parse_idx+=term_len; // advance
				djon_unescape_string(str); // fix string and remove escape bit
				return str_idx; // done
			}
		}

		str->len++; // grow string
		ds->parse_idx++; // advance
	}
	if(*term=='\n')
	{
		djon_unescape_string(str); // fix string and remove escape bit
		return str_idx; // accept end of file as a \n
	}

	if(*term=='\'')
	{
		djon_set_error(ds,"missing '");
	}
	else
	if(*term=='"')
	{
		djon_set_error(ds,"missing \"");
	}
	else
	{
		djon_set_error(ds,"missing string terminator");
	}
	return 0;
}

int djon_parse_number(djon_state *ds)
{
	char *cps=ds->data+ds->parse_idx;
	char *cpe;

//	double d=strtod(cps,&cpe);
	double d=djon_str_to_number(ds,cps,&cpe);
	int len=cpe-cps;
	if( len > 0 ) // valid number
	{
		int num_idx=djon_parse_next(ds);
		djon_value *num=djon_get(ds,num_idx);
		if(!num){return 0;}

		ds->parse_idx+=len;

		num->typ=DJON_NUMBER;
		num->num=d;

		return num_idx;
	}
	if(!ds->strict) // invalid numbers become naked strings
	{
		return djon_parse_string(ds,'\n');
	}
	djon_set_error(ds,"invalid number");
	return 0;
}

int djon_parse_key(djon_state *ds)
{
	djon_skip_white(ds);

	djon_value *key=0;
	int key_idx=0;
	char term=0;
	char c;

	c=ds->data[ ds->parse_idx ];
	if( c=='\'' || c=='"' || c=='`') // reuse string parsing
	{
		if((ds->strict)&&(c!='"')){ djon_set_error(ds,"keys require double quotes in strict mode"); goto error; }
		key_idx=djon_parse_string(ds,c);
		key=djon_get(ds,key_idx);
		if(!key){return 0;} // out of data
		key->typ=DJON_KEY|DJON_STRING; // this is a key
		return key_idx;
	}
	if(ds->strict){ djon_set_error(ds,"naked keys not allowed in strict mode"); goto error; }


	key_idx=djon_parse_next(ds);
	key=djon_get(ds,key_idx);
	if(!key){return 0;} // out of data
	key->typ=DJON_KEY|DJON_STRING; // this is a key with no escape

	while( ds->parse_idx < ds->data_len ) // while data
	{
		if( djon_peek_white(ds) ) { return key_idx; } // stop at whitespace
		if( djon_peek_punct(ds,"=:") ) { return key_idx; } // stop at punct
		c=ds->data[ ds->parse_idx ];
		if( DJON_IS_DELIMINATOR(c) ) // a naked key may not contain any deliminator
		{ djon_set_error(ds,"invalid naked key"); goto error; }

		key->len++; // grow string
		ds->parse_idx++; // advance
	}

	djon_set_error(ds,"missing :");

error:
	return 0;
}




// remove from list
void djon_list_remove(djon_state *ds, djon_value *v)
{
	djon_value *p;
	p = djon_get(ds,v->nxt); if(p){ p->prv=v->prv; }
	p = djon_get(ds,v->prv); if(p){ p->nxt=v->nxt; }
	v->nxt=0;
	v->prv=0;
}

// swap a and b
void djon_list_swap(djon_state *ds, djon_value *a , djon_value *b )
{
	djon_value *p;
	int aidx=djon_idx(ds,a);
	int bidx=djon_idx(ds,b);
	
	// cache
	int anxt=a->nxt;
	int aprv=a->prv;
	int bnxt=b->nxt;
	int bprv=b->prv;
	
	// relink
	p = djon_get(ds,aprv); if(p){ p->nxt=bidx; }
	p = djon_get(ds,anxt); if(p){ p->prv=bidx; }
	p = djon_get(ds,bprv); if(p){ p->nxt=aidx; }
	p = djon_get(ds,bnxt); if(p){ p->prv=aidx; }

	// swap and fix for when ab are adjacent
	a->nxt = (bnxt==aidx) ? bidx : bnxt ;
	a->prv = (bprv==aidx) ? bidx : bprv ;
	b->nxt = (anxt==bidx) ? aidx : anxt ;
	b->prv = (aprv==bidx) ? aidx : aprv ;
}


// return a<b
int djon_sort_compare( djon_value *a , djon_value *b )
{
	int i=0;
	while( i<a->len && i<b->len )
	{
		if(a->str[i]<b->str[i]) { return 1; }
		if(a->str[i]>b->str[i]) { return 0; }
		i++;
	}
	if(a->len<b->len) { return 1; }
	return 0;
}
// dumb sort
void djon_sort_part(djon_state *ds, djon_value *s, djon_value *e )
{
	djon_value *t,*i,*j;
	for( i=s ; i!=e ; i=djon_get(ds,i->nxt) ) // start to end-1
	{
		for( j=djon_get(ds,i->nxt) ; j!=e ; j=djon_get(ds,j->nxt) ) // i+1 to end-1
		{
			if( djon_sort_compare(j,i) ) // should j come before i
			{
				djon_list_swap(ds,i,j);
				t=i; i=j; j=t; // swap i/j
			}
		}
		if( djon_sort_compare(e,i) ) // should e come before i
		{
			djon_list_swap(ds,i,e);
			t=i; i=e; e=t; // swap i/e
		}
	}
}

// force sort this object by its keys
void djon_sort_object(djon_state *ds, int idx )
{
	djon_value *obj=djon_get(ds,idx);
	if(!obj) { return; }
	if(!obj->lst) { return; }
	
	djon_value *s=djon_get(ds,obj->lst);
	djon_value *e;
	for( e=s ; e && e->nxt ; e=djon_get(ds,e->nxt) ) {;}

	djon_sort_part(ds, s, e );
	while( s && s->prv ) { s = djon_get(ds,s->prv); } // find new start
	obj->lst=djon_idx(ds,s); // save new start
}


// return a==b
int djon_clean_compare( djon_value *a , djon_value *b )
{
	int i=0;
	while( i<a->len && i<b->len )
	{
		if(a->str[i]!=b->str[i]) { return 0; }
		i++;
	}
	if(a->len!=b->len) { return 0; }
	return 1;
}

// remove any duplicate keys keeping the last one
void djon_clean_object(djon_state *ds, int idx )
{
	djon_value *obj=djon_get(ds,idx);
	if(!obj) { return; }
	if(!obj->lst) { return; }
	
	djon_value *s=djon_get(ds,obj->lst);
	djon_value *e;
	for( e=s ; e && e->nxt ; e=djon_get(ds,e->nxt) ) {;}

	djon_value *i;
	djon_value *j;
	int ji;
	for( i=e ; i ; i=djon_get(ds,i->prv) ) // loop backwards checking
	{
		ji=i->prv;
		while( ( j=djon_get(ds,ji) ) ) // loop backwards deleting
		{
			ji=j->prv; // remember so we can move j
			if(djon_clean_compare(i,j)) // dupe
			{
				djon_list_remove(ds,j);
			}
		}
	}
	// end will be the same but start may have changed so
	for( s=e ; s && s->prv ; s=djon_get(ds,s->prv) ) {;}
	obj->lst=djon_idx(ds,s);
}

int djon_parse_object(djon_state *ds)
{
	int obj_idx=djon_parse_next(ds);
	djon_value *obj=djon_get(ds,obj_idx);
	if(!obj){return 0;}

	djon_value *key;
	djon_value *val;

	ds->parse_idx++; // skip opener
	obj->typ=DJON_OBJECT;

	int lst_idx=0;
	int key_idx=0;
	int val_idx=0;

	while(1)
	{
		djon_skip_white(ds);
		if( ds->data[ds->parse_idx]=='}' ) // found closer
		{
			djon_apply_comments(ds,val_idx?val_idx:obj_idx); // apply any final comments to the last key or the object
			ds->parse_idx++;
			djon_clean_object(ds,obj_idx); // remove duplicate keys
			return obj_idx;
		}

		key_idx=djon_parse_key(ds);
		if(!key_idx) { djon_set_error(ds,"missing }"); return 0; }
		if( djon_skip_white_punct(ds,ds->strict ? ":" : "=:") != 1 ) { djon_set_error(ds,"missing :"); return 0; } // required
		val_idx=djon_parse_value(ds); if(!val_idx){ djon_set_error(ds,"missing value"); return 0; }
		djon_apply_comments(ds,val_idx); // apply any middle comments to the key

		obj=djon_get(ds,obj_idx); // realloc safe
		if( obj->lst==0) // first
		{
			obj->lst=key_idx;
			val=djon_get(ds,val_idx);
			val->par=key_idx;
			key=djon_get(ds,key_idx);
			key->par=obj_idx;
			key->lst=val_idx; //  remember val for this key
			lst_idx=key_idx;
		}
		else // chain
		{
			val=djon_get(ds,val_idx);
			val->par=key_idx;
			key=djon_get(ds,lst_idx); // last key
			key->nxt=key_idx;
			key=djon_get(ds,key_idx);
			key->par=obj_idx;
			key->prv=lst_idx;
			key->lst=val_idx; //  remember val for this key
			lst_idx=key_idx;
		}

		if( 0 == djon_skip_white(ds) ) // check for whitespace after value
		{
			if( ds->parse_idx+1 < ds->data_len ) // EOF?
			{
				char c=ds->data[ds->parse_idx]; // if not white then must be one of these chars
				if( ! ( ( c==',' ) || ( c=='}' ) ) )
				{
					djon_set_error(ds,"missing ,"); return 0;
				}
			}
		}
		if( djon_skip_white_punct(ds,",") > 1 ) // only skip upto one coma
		{
			djon_set_error(ds,"multiple ,"); return 0;
		}

	}

	return 0;
}

int djon_parse_array(djon_state *ds)
{
	int arr_idx=djon_parse_next(ds);
	djon_value *arr=djon_get(ds,arr_idx);
	if(!arr){return 0;}

	djon_value *val=0;

	ds->parse_idx++; // skip opener
	arr->typ=DJON_ARRAY;

	int lst_idx=0;
	int val_idx=0;

	int idx=0; // 0 based index into array
	while(1)
	{
		djon_skip_white(ds);
		if( ds->data[ds->parse_idx]==']' )  // found closer
		{
			djon_apply_comments(ds,val_idx?val_idx:arr_idx); // apply any final comments to the last value or the array
			ds->parse_idx++;
			return arr_idx;
		}

		val_idx=djon_parse_value(ds);
		if(!val_idx) { djon_set_error(ds,"missing ]"); return 0; } // no value, probably missed a ]

		arr=djon_get(ds,arr_idx); // realloc safe
		if( arr->lst==0) // first
		{
			val=djon_get(ds,val_idx);
			val->par=arr_idx;
			val->idx=idx++;
			arr->lst=val_idx;
			lst_idx=val_idx;
		}
		else // chain
		{
			val=djon_get(ds,val_idx);
			val->par=arr_idx;
			val->idx=idx++;
			val->prv=lst_idx;
			djon_get(ds,lst_idx)->nxt=val_idx;
			lst_idx=val_idx;
		}
		if( 0 == djon_skip_white(ds) ) // check for whitespace after value
		{
			if( ds->parse_idx < ds->data_len ) // EOF?
			{
				char c=ds->data[ds->parse_idx]; // if not white then must be one of these chars
				if( ! ( ( c==',' ) || ( c==']' ) ) )
				{
					djon_set_error(ds,"missing ,"); return 0;
				}
			}
		}
		if( djon_skip_white_punct(ds,",") > 1 ) // only skip upto one coma
		{
			djon_set_error(ds,"multiple ,"); return 0;
		}

	}

	return arr_idx;
}

int djon_parse_value(djon_state *ds)
{
	int idx;
	djon_value *v;

	if(ds->error_string){ return 0; }
	if(!djon_check_stack(ds)){ return 0; }

	djon_skip_white(ds);

// check for special strings lowercase/camel/upper only
	if( djon_peek_string(ds,"true" ) || ( (!ds->strict) && ( djon_peek_string(ds,"True" ) || djon_peek_string(ds,"TRUE" ) ) ) )
	{
		idx=djon_alloc(ds);
		v=djon_get(ds,idx);
		if(v==0) { return 0; }
		v->typ=DJON_BOOL;
		v->num=1.0;
		ds->parse_idx+=4;
		return idx;
	}
	else
	if( djon_peek_string(ds,"false" ) || ( (!ds->strict) && ( djon_peek_string(ds,"False" ) || djon_peek_string(ds,"FALSE" ) ) ) )
	{
		idx=djon_alloc(ds);
		v=djon_get(ds,idx);
		if(v==0) { return 0; }
		v->typ=DJON_BOOL;
		v->num=0.0;
		ds->parse_idx+=5;
		return idx;
	}
	else
	if( djon_peek_string(ds,"null" ) || ( (!ds->strict) && ( djon_peek_string(ds,"Null" ) || djon_peek_string(ds,"NULL" ) ) ) )
	{
		idx=djon_alloc(ds);
		v=djon_get(ds,idx);
		if(v==0) { return 0; }
		v->typ=DJON_NULL;
		ds->parse_idx+=4;
		return idx;
	}

	char c=ds->data[ds->parse_idx]; // peek next char

	switch( c )
	{
		case '}' :
			djon_set_error(ds,"unexpected }");
			return 0;
		break;
		case ']' :
			djon_set_error(ds,"unexpected ]");
			return 0;
		break;
		case ',' :
			djon_set_error(ds,"unexpected ,");
			return 0;
		break;
		case ':' :
			djon_set_error(ds,"unexpected :");
			return 0;
		break;
		case '=' :
			djon_set_error(ds,"unexpected =");
			return 0;
		break;
		case '{' :
			return djon_parse_object(ds);
		break;
		case '[' :
			return djon_parse_array(ds);
		break;
		case '\'' :
			if(ds->strict)
			{
				djon_set_error(ds,"single quote string not allowed in strict mode");
				return 0;
			}
			return djon_parse_string(ds,'\'');
		break;
		case '"' :
			return djon_parse_string(ds,'"');
		break;
		case '`' :
			if(ds->strict)
			{
				djon_set_error(ds,"back quote string not allowed in strict mode");
				return 0;
			}
			return djon_parse_string(ds,'`');
		break;
		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		case '8' :
		case '9' :
		case '.' :
		case '+' :
		case '-' :
			if( (ds->strict) && ( ( c=='+' ) || ( c=='.' ) ) )
			{
				djon_set_error(ds,"numbers may not begin with a . or + in strict mode");
				return 0;
			}
			return djon_parse_number(ds);
		break;
	}
	
	if(ds->strict)
	{
		djon_set_error(ds,"naked string not allowed in strict mode");
		return 0;
	}

	return djon_parse_string(ds,'\n');
}


// allocate a new value and return its index, 0 on error
int djon_check_stack(djon_state *ds)
{
	int stack=0xdeadbeef;
	if((DJON_MAX_STACK)<=0) { return 1; } // no stack check
	if(ds->parse_stack) // check stack flag
	{
		int stacksize = ds->parse_stack - ((char*)&stack); // oh yeah stack grows backwards
		if ( stacksize > (DJON_MAX_STACK) ) // 256k stack burper, give up if we use too much
		{
			djon_set_error(ds,"stack overflow");
			return 0;
		}
	}
	return 1;
}
int djon_parse(djon_state *ds)
{
	int stack=0xdeadbeef;
	ds->parse_stack=(char*)&stack;
	ds->parse_idx=0;
	ds->parse_value=0;

	ds->error_string=0;
	ds->error_char=0;
	ds->error_line=0;

	ds->parse_value=djon_parse_value(ds); // read one value

	djon_skip_white(ds); // after this we must be at end of file
	djon_apply_comments(ds,ds->parse_value); // apply any final comments

	if( ds->parse_idx < ds->data_len ) // not EOF
	{
		djon_set_error(ds,"garbage at end of file");
		goto error;
	}

	if(ds->parse_value==0)
	{
		djon_set_error(ds,"no data");
		goto error;
	}

	djon_shrink(ds);
	return ds->parse_value;
error:
	ds->parse_stack=0;
	return 0;
}

#ifdef __cplusplus
};
#endif

#endif

