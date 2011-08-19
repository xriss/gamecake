/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
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

#include "../wet/util/win_types.h"

#elif defined(X11)

#include <strings.h>
#include <sys/time.h>

#include "../wet/util/nix_types.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "../lua/src/lua.h"
#include "../lua/src/lauxlib.h"
#include "../lua/src/lualib.h"

//#include "../_src/liblwo/code/lwo2.h"

#ifdef __cplusplus
};
#endif


#include "../lua_freetype/code/lua_freetype.h"
