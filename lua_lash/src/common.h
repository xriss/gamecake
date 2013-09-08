#include <errno.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if !defined(LUA_VERSION_NUM) || (LUA_VERSION_NUM < 501)
#include <compat-5.1.h>
#endif

/*
 * read this many bytes from a file at a time
 */
#define READ_SIZE       100*1024

/*
 *
 */
#define ERR_STRING_LEN  512


