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

#include "../wet/util/win_types.h"

#elif defined(X11)

#include <strings.h>
#include <sys/time.h>

#include "../wet/util/nix_types.h"

#endif

#include "../wet/util/f32_math.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "../lib_lua/src/lua.h"
#include "../lib_lua/src/lauxlib.h"
#include "../lib_lua/src/lualib.h"

//#include "../_src/liblwo/code/lwo2.h"

#include "jpeglib.h"

#ifdef __cplusplus
};
#endif

#include "png.h"

#include "code/grd.h"
#include "code/grd_png.h"
#include "code/grd_jpg.h"
#include "code/lua_grd.h"
#include "code/neuquant32.h"


