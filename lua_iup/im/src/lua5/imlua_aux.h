/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_aux.h,v 1.2 2010/06/11 17:43:52 scuri Exp $
 */

#ifndef __IMLUA_AUX_H
#define __IMLUA_AUX_H

#if	defined(__cplusplus)
extern "C" {
#endif


/********************************/
/* exported from "imlua_aux.c". */
/********************************/

/* get table size */

int imlua_getn(lua_State *L, int index);

/* array */

int imlua_newarrayint(lua_State *L, int *value, int count, int start);
int imlua_newarrayulong(lua_State *L, unsigned long *value, int count, int start);
int imlua_newarrayfloat(lua_State *L, float *value, int count, int start);

int *imlua_toarrayint(lua_State *L, int index, int *count, int start);
unsigned long *imlua_toarrayulong (lua_State *L, int index, int *count, int start);
float *imlua_toarrayfloat(lua_State *L, int index, int *count, int start);

/* other parameter checking */

unsigned char imlua_checkmask(lua_State *L, int index);

void imlua_checktype(lua_State *L, int index, imImage *image, int color_space, int data_type);
void imlua_checkdatatype(lua_State *L, int index, imImage *image, int data_type);
void imlua_checkcolorspace(lua_State *L, int index, imImage *image, int color_space);

void imlua_matchsize(lua_State *L, imImage *image1, imImage *image2);
void imlua_matchcolor(lua_State *L, imImage *image1, imImage *image2);
void imlua_matchdatatype(lua_State *L, imImage *image1, imImage *image2);
void imlua_matchcolorspace(lua_State *L, imImage *image1, imImage *image2);
void imlua_match(lua_State *L, imImage *image1, imImage *image2);

/* used only when comparing two images */
#define imlua_matchcheck(L, cond, extramsg) if (!(cond)) \
                                               luaL_error(L, extramsg)

#define imlua_pusherror(L, _e) ((_e == IM_ERR_NONE)? lua_pushnil(L): lua_pushnumber(L, _e))


/********************************/
/* exported from "imlua.c".     */
/********************************/

/* constant registration. */

typedef struct _imlua_constant {
  const char *name;
  lua_Number value;
  const char *str_value;
} imlua_constant;

void imlua_regconstants(lua_State *L, const imlua_constant *imconst);


/********************************/
/* private module open          */
/********************************/

void imlua_open_convert(lua_State *L);  /* imlua_convert.c */
void imlua_open_util(lua_State *L);     /* imlua_util.c    */
void imlua_open_file(lua_State *L);     /* imlua_file.c    */

#if LUA_VERSION_NUM < 502
#define luaL_typeerror luaL_typerror
#endif


#ifdef __cplusplus
}
#endif

#endif
