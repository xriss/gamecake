/*

(C) Kriss@XIXs.com 2017 and released under the https://opensource.org/licenses/MIT license.

*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>



#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
};
#endif


#include "glslang/glslang/Public/ShaderLang.h"
#include "glslang/glslang/Include/glslang_c_interface.h"

#include "code/lua_glslang.h"


