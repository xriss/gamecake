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

//#include <crtdbg.h>

#if !defined(__TIMESTAMP__)
#define __TIMESTAMP__ 0
#endif

#include <GL3/gl3w.h>


#include <io.h>
#include <direct.h>

#include <windows.h>
#include <windowsx.h>



#ifdef __cplusplus
extern "C" {
#endif

#include "../lib_lua/src/lua.h"
#include "../lib_lua/src/lauxlib.h"
#include "../lib_lua/src/lualib.h"

#ifdef __cplusplus
};
#endif

#include "../wet/util/wet_types.h"


#include "code/lua_win.h"
