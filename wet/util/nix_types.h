/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define U32_ID4(a,b,c,d) ((u32)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))


typedef double						f64;

typedef long long						s64;
typedef unsigned long long			u64;

typedef float						f32;

typedef int							s32;
typedef unsigned int				u32;

typedef short						s16;
typedef unsigned short				u16;

typedef char						s8;
typedef unsigned char				u8;



typedef const double				cf64;

typedef const long long				cs64;
typedef const unsigned long long		cu64;

typedef const float					cf32;

typedef const int					cs32;
typedef const unsigned int			cu32;

typedef const short					cs16;
typedef const unsigned short		cu16;

typedef const char					cs8;
typedef const unsigned char			cu8;



typedef volatile double				vf64;

typedef volatile long long			vs64;
typedef volatile unsigned long long	vu64;

typedef volatile float				vf32;

typedef volatile int				vs32;
typedef volatile unsigned int		vu32;

typedef volatile short				vs16;
typedef volatile unsigned short		vu16;

typedef volatile char				vs8;
typedef volatile unsigned char		vu8;


// map caseless string compares in win to nix 
#define stricmp strcasecmp
