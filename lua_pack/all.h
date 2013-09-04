/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>



#if defined(_MSC_VER)

#include <crtdbg.h>

#else

#endif


#if !defined(__TIMESTAMP__)
#define __TIMESTAMP__ 0
#endif


#if defined(WIN32)

#include <io.h>
#include <direct.h>

#include <windows.h>
#include <windowsx.h>

#elif defined(X11)

#include <strings.h>
#include <sys/time.h>

#endif

#include "../wet/util/pstdint.h"
#include "../wet/util/wet_types.h"
//#include "../wet/util/f32_math.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#ifdef __cplusplus
};
#endif

#include "code/lua_pack.h"


