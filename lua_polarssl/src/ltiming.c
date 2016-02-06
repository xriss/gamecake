/* Lua PolarSSL: Portable interface to the CPU cycle counter */

#include "polarssl/timing.h"


/*
 * Returns: number
 */
static int
ltiming_hardclock (lua_State *L)
{
  lua_pushinteger(L, hardclock());
  return 1;
}


#define TIMING_METHODS \
  {"hardclock",		ltiming_hardclock}
