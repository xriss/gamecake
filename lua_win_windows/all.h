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

//#include <crtdbg.h>

#if !defined(__TIMESTAMP__)
#define __TIMESTAMP__ 0
#endif

//#include <GL3/gl3w.h>


#include <io.h>
#include <direct.h>

#include <windows.h>
#include <windowsx.h>



#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
};
#endif

#include "../lib_hacks/code/pstdint.h"
#include "../lib_hacks/code/wet_types.h"


#include "code/lua_win.h"
