/* Lua PolarSSL: Multi-Precision Integer library */

#include "polarssl/bignum.h"


#define MPI_TYPENAME	"polarssl.mpi"


/*
 * Returns: mpi_udata
 */
static int
lmpi_new (lua_State *L)
{
  mpi *X = lua_newuserdata(L, sizeof(mpi));

  luaL_getmetatable(L, MPI_TYPENAME);
  lua_setmetatable(L, -2);

  mpi_init(X);
  return 1;
}

/*
 * Arguments: mpi_udata
 */
static int
lmpi_close (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);

  mpi_free(X);
  return 0;
}

/*
 * Arguments: mpi_udata, nblimbs (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_grow (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t nblimbs = luaL_checkinteger(L, 2);

  return lssl_seterror(L, mpi_grow(X, nblimbs));
}

/*
 * Arguments: mpi_udata, mpi_udata
 */
static int
lmpi_swap (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *Y = checkudata(L, 2, MPI_TYPENAME);

  mpi_swap(X, Y);
  return 0;
}

/*
 * Arguments: mpi_udata
 * Returns: number of least significant bits
 */
static int
lmpi_lsb (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);

  lua_pushinteger(L, mpi_lsb(X));
  return 1;
}

/*
 * Arguments: mpi_udata
 * Returns: number of most significant bits
 */
static int
lmpi_msb (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);

  lua_pushinteger(L, mpi_msb(X));
  return 1;
}

/*
 * Arguments: mpi_udata
 * Returns: total size in bytes (number)
 */
static int
lmpi_size (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);

  lua_pushinteger(L, mpi_size(X));
  return 1;
}

/*
 * Arguments: mpi_udata, mpi_udata (source) | number | string, [radix (number)]
 * Returns: [mpi_udata]
 */
static int
lmpi_set (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 2)) {
  case LUA_TUSERDATA: {
      const mpi *Y = checkudata(L, 2, MPI_TYPENAME);
      res = mpi_copy(X, Y);
    }
    break;
  case LUA_TNUMBER: {
      const int z = lua_tointeger(L, 2);
      res = mpi_lset(X, z);
    }
    break;
  case LUA_TSTRING: {
      const char *s = lua_tostring(L, 2);
      const int radix = luaL_optinteger(L, 3, 16);
      res = (radix != 0) ? mpi_read_string(X, radix, s)
       : mpi_read_binary(X, (const void *) s, lua_rawlen(L, 2));
    }
    break;
  default:
    luaL_argerror(L, 2, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata, [radix (number)]
 * Returns: [string]
 */
static int
lmpi_get (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const int radix = luaL_optinteger(L, 2, 16);
  char buf[POLARSSL_MPI_RW_BUFFER_SIZE];
  size_t len = sizeof(buf);
  int res;

  memset(buf, 0, len);
  if (radix != 0) {
    res = mpi_write_string(X, radix, buf, &len);
    --len;
  }
  else {
    const size_t n = mpi_size(X);
    res = (len < n) ? POLARSSL_ERR_MPI_BUFFER_TOO_SMALL
     : mpi_write_binary(X, (void *) buf, n);
    len = n;
  }

  if (!res) {
    lua_pushlstring(L, buf, len);
    return 1;
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata, positon (number), value (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_set_bit (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t pos = luaL_checkinteger(L, 2);
  const unsigned char val = lua_tointeger(L, 3);

  return lssl_seterror(L, mpi_set_bit(X, pos, val));
}

/*
 * Arguments: mpi_udata, positon (number)
 * Returns: [number]
 */
static int
lmpi_get_bit (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t pos = luaL_checkinteger(L, 2);

  lua_pushinteger(L, mpi_get_bit(X, pos));
  return 1;
}

/*
 * Arguments: mpi_udata, file_udata, [radix (number)]
 * Returns: [mpi_udata]
 */
static int
lmpi_read_file (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  FILE **fp = checkudata(L, 2, LUA_FILEHANDLE);
  const int radix = luaL_optinteger(L, 3, 16);

  return lssl_seterror(L, mpi_read_file(X, radix, *fp));
}

/*
 * Arguments: mpi_udata, file_udata, [prefix (string), radix (number)]
 * Returns: [mpi_udata]
 */
static int
lmpi_write_file (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  FILE **fp = checkudata(L, 2, LUA_FILEHANDLE);
  const char *prefix = lua_tostring(L, 3);
  const int radix = luaL_optinteger(L, 4, 16);

  return lssl_seterror(L, mpi_write_file(prefix, X, radix, *fp));
}

/*
 * Arguments: mpi_udata, buffer_ludata, length (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_read_binary (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const void *p = lua_touserdata(L, 2);
  const size_t len = luaL_checkinteger(L, 3);

  return lssl_seterror(L, mpi_read_binary(X, p, len));
}

/*
 * Arguments: mpi_udata, buffer_ludata, length (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_write_binary (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  void *p = lua_touserdata(L, 2);
  const size_t len = luaL_checkinteger(L, 3);

  return lssl_seterror(L, mpi_write_binary(X, p, len));
}

/*
 * Arguments: mpi_udata, count (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_shift_l (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t n = luaL_checkinteger(L, 2);

  return lssl_seterror(L, mpi_shift_l(X, n));
}

/*
 * Arguments: mpi_udata, count (number)
 * Returns: [mpi_udata]
 */
static int
lmpi_shift_r (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t n = luaL_checkinteger(L, 2);

  return lssl_seterror(L, mpi_shift_r(X, n));
}

/*
 * Arguments: mpi_udata, mpi_udata
 * Returns: number
 */
static int
lmpi_cmp_abs (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *Y = checkudata(L, 2, MPI_TYPENAME);

  lua_pushinteger(L, mpi_cmp_abs(X, Y));
  return 1;
}

/*
 * Arguments: mpi_udata, mpi_udata | number
 * Returns: number
 */
static int
lmpi_cmp (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 2)) {
  case LUA_TUSERDATA: {
      mpi *Y = checkudata(L, 2, MPI_TYPENAME);
      res = mpi_cmp_mpi(X, Y);
    }
    break;
  case LUA_TNUMBER: {
      const int z = lua_tointeger(L, 2);
      res = mpi_cmp_int(X, z);
    }
    break;
  default:
    luaL_argerror(L, 2, "invalid value");
  }

  lua_pushinteger(L, res);
  return 1;
}

static int
lmpi_eq (lua_State *L)
{
  lmpi_cmp(L);
  lua_pushboolean(L, lua_tointeger(L, -1) == 0);
  return 1;
}

static int
lmpi_lt (lua_State *L)
{
  lmpi_cmp(L);
  lua_pushboolean(L, lua_tointeger(L, -1) < 0);
  return 1;
}

static int
lmpi_le (lua_State *L)
{
  lmpi_cmp(L);
  lua_pushboolean(L, lua_tointeger(L, -1) <= 0);
  return 1;
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_add_abs (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *B = checkudata(L, 3, MPI_TYPENAME);

  return lssl_seterror(L, mpi_add_abs(X, A, B));
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_add (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 3)) {
  case LUA_TUSERDATA: {
      mpi *B = checkudata(L, 3, MPI_TYPENAME);
      res = mpi_add_mpi(X, A, B);
    }
    break;
  case LUA_TNUMBER: {
      const int b = lua_tointeger(L, 3);
      res = mpi_add_int(X, A, b);
    }
    break;
  default:
    luaL_argerror(L, 3, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_new_add (lua_State *L)
{
  lmpi_new(L);
  lua_insert(L, 1);
  return lmpi_add(L);
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_sub_abs (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *B = checkudata(L, 3, MPI_TYPENAME);

  return lssl_seterror(L, mpi_sub_abs(X, A, B));
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_sub (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 3)) {
  case LUA_TUSERDATA: {
      mpi *B = checkudata(L, 3, MPI_TYPENAME);
      res = mpi_sub_mpi(X, A, B);
    }
    break;
  case LUA_TNUMBER: {
      const int b = lua_tointeger(L, 3);
      res = mpi_sub_int(X, A, b);
    }
    break;
  default:
    luaL_argerror(L, 3, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_new_sub (lua_State *L)
{
  lmpi_new(L);
  lua_insert(L, 1);
  return lmpi_sub(L);
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_mul (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 3)) {
  case LUA_TUSERDATA: {
      mpi *B = checkudata(L, 3, MPI_TYPENAME);
      res = mpi_mul_mpi(X, A, B);
    }
    break;
  case LUA_TNUMBER: {
      const int b = lua_tointeger(L, 3);
      res = mpi_mul_int(X, A, b);
    }
    break;
  default:
    luaL_argerror(L, 3, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_new_mul (lua_State *L)
{
  lmpi_new(L);
  lua_insert(L, 1);
  return lmpi_mul(L);
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata | number (Right-hand),
 *	[mpi_udata (Rest)]
 * Returns: [mpi_udata]
 */
static int
lmpi_div (lua_State *L)
{
  mpi *Q = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *R = lua_isuserdata(L, 4) ? checkudata(L, 4, MPI_TYPENAME) : NULL;
  int res = 0;

  switch (lua_type(L, 3)) {
  case LUA_TUSERDATA: {
      mpi *B = checkudata(L, 3, MPI_TYPENAME);
      res = mpi_div_mpi(Q, R, A, B);
    }
    break;
  case LUA_TNUMBER: {
      const int b = lua_tointeger(L, 3);
      res = mpi_div_int(Q, R, A, b);
    }
    break;
  default:
    luaL_argerror(L, 3, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata (Left-hand), mpi_udata | number (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_new_div (lua_State *L)
{
  lmpi_new(L);
  lua_insert(L, 1);
  return lmpi_div(L);
}

/*
 * Arguments: mpi_udata (Left-hand), number (Right-hand)
 * Returns: [number]
 * |
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_mod (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  int res = 0;

  switch (lua_type(L, 2)) {
  case LUA_TUSERDATA: {
      mpi *A = checkudata(L, 2, MPI_TYPENAME);
      mpi *B = checkudata(L, 3, MPI_TYPENAME);
      res = mpi_mod_mpi(X, A, B);
    }
    break;
  case LUA_TNUMBER: {
      const int b = lua_tointeger(L, 2);
      t_uint r;

      res = mpi_mod_int(&r, X, b);
      if (!res) {
        lua_pushinteger(L, r);
        return 1;
      }
    }
    break;
  default:
    luaL_argerror(L, 2, "invalid value");
  }

  return lssl_seterror(L, res);
}

/*
 * Arguments: mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_new_mod (lua_State *L)
{
  lmpi_new(L);
  lua_insert(L, 1);
  return lmpi_mod(L);
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Exponent),
 *	mpi_udata (Modular), [mpi_udata (Speed-up recalculations)]
 * Returns: [mpi_udata]
 */
static int
lmpi_exp_mod (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *E = checkudata(L, 3, MPI_TYPENAME);
  mpi *N = checkudata(L, 4, MPI_TYPENAME);
  mpi *_RR = lua_isuserdata(L, 5) ? checkudata(L, 5, MPI_TYPENAME) : NULL;

  return lssl_seterror(L, mpi_exp_mod(X, A, E, N, _RR));
}

/*
 * Arguments: mpi_udata, size (number), [ssl_udata (RNG)]
 * Returns: [mpi_udata]
 */
static int
lmpi_fill_random (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t size = luaL_checkinteger(L, 2);
  ssl_context *ssl = lua_isuserdata(L, 3) ? checkudata(L, 3, SSL_TYPENAME) : NULL;
  havege_state hs;
  f_rng_t f_rng = ssl ? ssl->f_rng : havege_random;
  void *p_rng = ssl ? ssl->p_rng : &hs;

  if (!ssl) havege_init(&hs);
  return lssl_seterror(L, mpi_fill_random(X, size, f_rng, p_rng));
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_gcd (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *B = checkudata(L, 3, MPI_TYPENAME);

  return lssl_seterror(L, mpi_gcd(X, A, B));
}

/*
 * Arguments: mpi_udata, mpi_udata (Left-hand), mpi_udata (Right-hand)
 * Returns: [mpi_udata]
 */
static int
lmpi_inv_mod (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  mpi *A = checkudata(L, 2, MPI_TYPENAME);
  mpi *N = checkudata(L, 3, MPI_TYPENAME);

  return lssl_seterror(L, mpi_inv_mod(X, A, N));
}

/*
 * Arguments: mpi_udata, [ssl_udata (RNG)]
 * Returns: [mpi_udata]
 */
static int
lmpi_is_prime (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  ssl_context *ssl = lua_isuserdata(L, 2) ? checkudata(L, 2, SSL_TYPENAME) : NULL;
  havege_state hs;
  f_rng_t f_rng = ssl ? ssl->f_rng : havege_random;
  void *p_rng = ssl ? ssl->p_rng : &hs;

  if (!ssl) havege_init(&hs);
  return lssl_seterror(L, mpi_is_prime(X, f_rng, p_rng));
}

/*
 * Arguments: mpi_udata, nbits (number), dh_flag (boolean), [ssl_udata (RNG)]
 * Returns: [mpi_udata]
 */
static int
lmpi_gen_prime (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);
  const size_t nbits = luaL_checkinteger(L, 2);
  const int dh_flag = lua_toboolean(L, 3);
  ssl_context *ssl = lua_isuserdata(L, 4) ? checkudata(L, 4, SSL_TYPENAME) : NULL;
  havege_state hs;
  f_rng_t f_rng = ssl ? ssl->f_rng : havege_random;
  void *p_rng = ssl ? ssl->p_rng : &hs;

  if (!ssl) havege_init(&hs);
  return lssl_seterror(L, mpi_gen_prime(X, nbits, dh_flag, f_rng, p_rng));
}

/*
 * Arguments: mpi_udata
 * Returns: string
 */
static int
lmpi_tostring (lua_State *L)
{
  mpi *X = checkudata(L, 1, MPI_TYPENAME);

  lua_pushfstring(L, MPI_TYPENAME " (%p)", X->p);
  return 1;
}


#define MPI_METHODS \
  {"mpi",		lmpi_new}

static luaL_Reg lmpi_meth[] = {
  {"close",		lmpi_close},
  {"grow",		lmpi_grow},
  {"swap",		lmpi_swap},
  {"lsb",		lmpi_lsb},
  {"msb",		lmpi_msb},
  {"__len",		lmpi_size},
  {"set",		lmpi_set},
  {"get",		lmpi_get},
  {"set_bit",		lmpi_set_bit},
  {"get_bit",		lmpi_get_bit},
  {"read_file",		lmpi_read_file},
  {"write_file",	lmpi_write_file},
  {"read_binary",	lmpi_read_binary},
  {"write_binary",	lmpi_write_binary},
  {"shift_l",		lmpi_shift_l},
  {"shift_r",		lmpi_shift_r},
  {"cmp_abs",		lmpi_cmp_abs},
  {"cmp",		lmpi_cmp},
  {"__eq",		lmpi_eq},
  {"__lt",		lmpi_lt},
  {"__le",		lmpi_le},
  {"add_abs",		lmpi_add_abs},
  {"add",		lmpi_add},
  {"__add",		lmpi_new_add},
  {"sub_abs",		lmpi_sub_abs},
  {"sub",		lmpi_sub},
  {"__sub",		lmpi_new_sub},
  {"mul",		lmpi_mul},
  {"__mul",		lmpi_new_mul},
  {"div",		lmpi_div},
  {"__div",		lmpi_new_div},
  {"mod",		lmpi_mod},
  {"__mod",		lmpi_new_mod},
  {"exp_mod",		lmpi_exp_mod},
  {"fill_random",	lmpi_fill_random},
  {"gcd",		lmpi_gcd},
  {"inv_mod",		lmpi_inv_mod},
  {"is_prime",		lmpi_is_prime},
  {"gen_prime",		lmpi_gen_prime},
  {"__tostring",	lmpi_tostring},
  {"__gc",		lmpi_close},
  {NULL, NULL}
};
