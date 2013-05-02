#include "wstring.h"

#if !LUA_HAS_WSTRING || FORCE_OWN_WSTRING

#undef LUA_API
#define LUA_API
#undef LUALIB_API
#define LUALIB_API

#include "lwstring.c"

#endif

