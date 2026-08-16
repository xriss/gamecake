/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Blank 2013 http://xixs.com and MIT licensed to you.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#ifndef wet_types_h
#define wet_types_h

#include <stdint.h>


#define U32_ID4(a,b,c,d) ((u32)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))


typedef double					f64;
typedef float					f32;


typedef  int64_t				s64;
typedef uint64_t				u64;
typedef  int32_t				s32;
typedef uint32_t				u32;
typedef  int16_t				s16;
typedef uint16_t				u16;
typedef  int8_t					s8;
typedef uint8_t					u8;


// random crap fix
#if defined(WIN32)

#define stricmp strcasecmp

#else

#define strcasecmp stricmp

#endif



#endif
