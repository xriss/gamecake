/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
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

#include "wet_types.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

//#include "../_src/liblwo/code/lwo2.h"

#include "jpeglib.h"
#include "jerror.h"

#ifdef __cplusplus
};
#endif

#include "png.h"

#include "code/grd.h"
#include "code/grd_png.h"
#include "code/grd_jpg.h"
#include "code/grd_gif.h"
#include "code/lua_grd.h"
#include "code/neuquant32.h"


