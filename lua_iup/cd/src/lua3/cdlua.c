/***************************************************************************\
* CDLUA.C, for LUA 3.1                                                      *
* Diego Fernandes Nehab, Antonio Escano Scuri                               *
* 01/99                                                                     *
* Implements all that TOLUA couldn't handle.                                * 
\***************************************************************************/

/***************************************************************************\
* Included definitions.                                                     *
\***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***************************************************************************\
* CD Definitions.                                                           *
\***************************************************************************/
#include "cd.h"
#include "wd.h"

#include "cdirgb.h"  /* cdRedImage, cdGreenImage, cdBlueImage */

/* error checking when there is no active canvas */
#include "cdvoid.h"

/***************************************************************************\
* LUA Definitions.                                                          *
\***************************************************************************/
#include <lua.h>
#include <lauxlib.h>

/***************************************************************************\
* CDLUA Definitions.                                                        *
\***************************************************************************/
#include "cdlua.h"
#include "cdlua3_private.h"

/***************************************************************************\
* Globals.                                                                  *
\***************************************************************************/
static int color_tag;
static int stipple_tag;
static int pattern_tag;
static int image_tag;
static int bitmap_tag;
static int imagergb_tag;
static int imagergba_tag;
static int palette_tag;
static int imagemap_tag;
static int channel_tag;
static int canvas_tag;
static int state_tag;

static channel_t channel_info;
static cdCanvas *void_canvas;
static cdContextLUA* cdlua_drivers[50];
static int cdlua_numdrivers = 0;
static lua_Object cdlua_namespace;

int luaL_cd_open(void); /* from toluacd.c */
int luaL_wd_open(void); /* from toluawd.c */
void cdlua_initdrivers(void); /* to cdluactx.c */


/***************************************************************************\
* Creation and destruction of types LUA can't handle.                       *
\***************************************************************************/

void cdlua_setnamespace(char* name, char* new_name)
{
  lua_Object obj = lua_getglobal(name);
  lua_pushobject(cdlua_namespace);
  lua_pushstring(new_name);
  lua_pushobject(obj);
  lua_settable();
}

void cdlua_register(char* name, lua_CFunction func)
{
  lua_register(name, func);

  if (name[0] == 'w') 
  {
    char new_name[100];
    new_name[0] = 'w';
    strcpy(new_name+1, name+2);
    cdlua_setnamespace(name, new_name); /* wdXXX */
  }
  else                
    cdlua_setnamespace(name, name+2);   /* cdXXX */
}

void cdlua_pushnumber(double num, char* name)
{
  lua_pushnumber(num); lua_setglobal(name);
  cdlua_setnamespace(name, name+3);     /* CD_XXXX */
}

static void cdlua_pushcolor(long color, char* name)
{
  lua_pushusertag((void*)color, color_tag); lua_setglobal(name);
  cdlua_setnamespace(name, name+3);     /* CD_XXXX */
}

void cdlua_addcontext(cdContextLUA* luactx)
{
  int i;
  luactx->id = cdlua_numdrivers;
  cdlua_drivers[cdlua_numdrivers] = luactx;

  cdlua_pushnumber(cdlua_numdrivers, luactx->name);

  /* skip CD_SIZECB, register other callbacks */
  for (i=1; i<luactx->cb_n; i++)
  {
    cdlua_pushnumber(i, luactx->cb_list[i].name);
  }

  cdlua_numdrivers++;
}

/***************************************************************************\
* Creates a CD canvas as a canvas_tag usertag lua_Object.                   *
* If the creation fails, the function returns a nil lua_Object.             *
\***************************************************************************/
static void cdlua_createcanvas(void) 
{
  lua_Object driver;

  long int driver_i;
  canvas_t *canvas_p;
  void *data_p;
  
  /* if there is not enough memory */
  canvas_p = (canvas_t *) malloc(sizeof(canvas_t));
  if (!canvas_p) {
    lua_pushnil();
    return;
  }

  /* get driver parameter */
  driver = lua_getparam(1);
  if (!lua_isnumber(driver))
    lua_error("cdCreateCanvas: invalid driver parameter!");
  driver_i = (long int) lua_getnumber(driver);

  if (driver_i >= cdlua_numdrivers)
    lua_error("cdCreateCanvas: unknown driver!");

  data_p = cdlua_drivers[driver_i]->checkdata(2);
  canvas_p->cd_canvas = cdCreateCanvas(cdlua_drivers[driver_i]->ctx(), data_p);
  
  /* if creation failed, return nil so that the user can compare */
  /* the result with nil and know that it failed */
  if (!canvas_p->cd_canvas) {
    free(canvas_p);
    lua_pushnil();
  }
  /* else, return a canvas_t structure */
  else 
    lua_pushusertag((void *) canvas_p, canvas_tag);
}

static lua_Object wdlua_hardcopy_func_lua = 0;

static void wdlua_hardcopy_func(void) 
{
  lua_callfunction(wdlua_hardcopy_func_lua);
}

static void wdlua_hardcopy(void) 
{
  lua_Object driver;
  lua_Object canvas;

  long int driver_i;
  canvas_t *canvas_p;
  void *data_p;

  /* get driver parameter */
  driver = lua_getparam(1);
  if (!lua_isnumber(driver))
    lua_error("wdHardcopy: invalid driver parameter!");
  driver_i = (long int) lua_getnumber(driver);

  canvas = lua_getparam(3);
  
  if (canvas == LUA_NOOBJECT)
    lua_error("wdHardcopy: canvas parameter missing!");

  /* if the creation failed, canvas can be nil, in which case we */
  /* issue an error */
  if (lua_isnil(canvas))
    lua_error("wdHardcopy: attempt to get a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("wdHardcopy: invalid canvas parameter!");
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) 
    lua_error("wdHardcopy: attempt to get a killed canvas!");

  wdlua_hardcopy_func_lua = lua_getparam(4);

  if (!lua_isfunction(wdlua_hardcopy_func_lua))
    lua_error("wdHardcopy: invalid draw function!");

  if (lua_getparam(5) != LUA_NOOBJECT)
    lua_error("wdHardcopy: too many parameters!");

  if (driver_i >= cdlua_numdrivers)
    lua_error("wdHardcopy: unknown driver!");

  data_p = cdlua_drivers[driver_i]->checkdata(2);
  wdHardcopy(cdlua_drivers[driver_i]->ctx(), data_p, canvas_p->cd_canvas, wdlua_hardcopy_func);
}

static void cdlua_getcontext(void)
{
  lua_Object canvas;
  canvas_t *canvas_p;
  cdContext* ctx;
  int driver_i = -1, i;

  canvas = lua_getparam(1);
  
  if (canvas == LUA_NOOBJECT)
    lua_error("cdGetContext: canvas parameter missing!");

  /* if the creation failed, canvas can be nil, in which case we */
  /* issue an error */
  if (lua_isnil(canvas))
    lua_error("cdGetContext: attempt to get a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("cdGetContext: invalid canvas parameter!");
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) 
    lua_error("cdGetContext: attempt to get a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdGetContext: too many parameters!");
    
  ctx = cdGetContext(canvas_p->cd_canvas);

  for (i=0; i < cdlua_numdrivers; i++)
  {
    if (ctx == cdlua_drivers[i]->ctx())
    {
      driver_i = i;
      break;
    }
  }

  if (i == cdlua_numdrivers)
    lua_error("cdGetContext: unknown driver!");

  lua_pushnumber(driver_i);
}

static void cdlua_contextcaps(void) 
{
  lua_Object driver;
  long int driver_i;
  unsigned long caps;
  
  /* get driver parameter */
  driver = lua_getparam(1);
  if (!lua_isnumber(driver))
    lua_error("cdCreateCanvas: invalid driver parameter!");
  driver_i = (long int) lua_getnumber(driver);

  if (driver_i >= cdlua_numdrivers)
    lua_error("cdContextCaps: unknown driver!");

  caps = cdContextCaps(cdlua_drivers[driver_i]->ctx());
  
  lua_pushnumber(caps);
}

/***************************************************************************\
* Activates a cd canvas.                                                    *
\***************************************************************************/
static void cdlua_activate(void)
{
  lua_Object canvas;
  canvas_t *canvas_p;

  canvas = lua_getparam(1);

  if (canvas == LUA_NOOBJECT)
    lua_error("cdActivate: canvas parameter missing!");

  /* if canvas is nil, activate a void canvas */
  if (lua_isnil(canvas)) {
    lua_pushnumber(cdActivate(void_canvas));
    return;
  }

  if (lua_tag(canvas) != canvas_tag) {
    cdActivate(void_canvas);
    lua_error("cdActivate: invalid canvas parameter!");
  }
  
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) {
    cdActivate(void_canvas);
    lua_error("cdActivate: attempt to activate a killed canvas!");
  }

  if (lua_getparam(2) != LUA_NOOBJECT) {
    cdActivate(void_canvas);
    lua_error("cdActivate: too many parameters!");
  }
  
  lua_pushnumber(cdActivate(canvas_p->cd_canvas));
}

/***************************************************************************\
* Returns the active canvas.                                                *
\***************************************************************************/
static void cdlua_activecanvas(void)
{
  canvas_t *canvas_p;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdActiveCanvas: too many parameters!");

  canvas_p = (canvas_t *) malloc(sizeof(canvas_t));
  if (!canvas_p) {
    lua_pushnil();
    return;
  }

  canvas_p->cd_canvas = cdActiveCanvas();

  /* if the active canvas is NULL, return nil so that the user can compare */
  /* the result with nil */
  if (!canvas_p->cd_canvas) {
    free(canvas_p);
    lua_pushnil();
  }
  else 
    lua_pushusertag((void *) canvas_p, canvas_tag);
}

cdCanvas* cdlua_checkcanvas(int pos)
{
  lua_Object canvas;
  canvas_t *canvas_p;

  canvas = lua_getparam(pos);

  if (canvas == LUA_NOOBJECT)
    lua_error("cdlua_getcanvas: canvas parameter missing!");

  if (lua_tag(canvas) != canvas_tag) 
    lua_error("cdlua_getcanvas: invalid canvas parameter!");
  
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas)
    lua_error("cdlua_getcanvas: attempt to get a killed canvas!");

  return canvas_p->cd_canvas;
}

cdCanvas* cdlua_getcanvas(void)
{
  return cdlua_checkcanvas(1);
}

void cdlua_pushcanvas(cdCanvas* canvas)
{
  canvas_t *canvas_p = (canvas_t *) malloc(sizeof(canvas_t));
  canvas_p->cd_canvas = canvas;
  lua_pushusertag((void *)canvas_p, canvas_tag);
}

static void cdlua_savestate(void)
{
  state_t *state_p;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdSaveState: too many parameters!");

  state_p = (state_t *) malloc(sizeof(state_t));
  if (!state_p) {
    lua_pushnil();
    return;
  }

  state_p->state = cdSaveState();

  /* if the active canvas is NULL, return nil so that the user can compare */
  /* the result with nil */
  if (!state_p->state) {
    free(state_p);
    lua_pushnil();
  }
  else 
    lua_pushusertag((void *) state_p, state_tag);
}

static void cdlua_restorestate(void)
{
  lua_Object state;
  state_t *state_p;

  state = lua_getparam(1);
  if (state == LUA_NOOBJECT)
    lua_error("cdRestoreState: state parameter missing!");
  if (lua_isnil(state))
    lua_error("cdRestoreState: attempt to restore a NIL state!");

  if (lua_tag(state) != state_tag)
    lua_error("cdRestoreState: invalid canvas parameter!");
  
  state_p = (state_t *) lua_getuserdata(state);
  if (!state_p->state)
    lua_error("cdRestoreState: attempt to restore a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdRestoreState: too many parameters!");
  
  cdRestoreState(state_p->state);
}

static void cdlua_releasestate(void)
{
  lua_Object state;
  state_t *state_p;

  state = lua_getparam(1);
  if (state == LUA_NOOBJECT)
    lua_error("cdReleaseState: state parameter missing!");
  if (lua_isnil(state))
    lua_error("cdReleaseState: attempt to release a NIL state!");

  if (lua_tag(state) != state_tag)
    lua_error("cdReleaseState: invalid canvas parameter!");
  
  state_p = (state_t *) lua_getuserdata(state);
  if (!state_p->state)
    lua_error("cdReleaseState: attempt to release a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdReleaseState: too many parameters!");
  
  cdReleaseState(state_p->state);
  state_p->state = NULL;
}

static void cdlua_LineStyleDashes(void)
{
  lua_Object dashes, count, value;
  int *dashes_int, dashes_count, i;

  dashes = lua_getparam(1);
  if (dashes == LUA_NOOBJECT)
    lua_error("cdLineStyleDashes: dashes parameter missing!");
  if (lua_isnil(dashes))
    lua_error("cdLineStyleDashes: dashes parameter is nil!");
  if (!lua_istable(dashes))
    lua_error("cdLineStyleDashes: invalid dashes parameter!");

  count = lua_getparam(2);
  if (count == LUA_NOOBJECT)
    lua_error("cdLineStyleDashes: count parameter missing!");
  if (lua_isnil(count))
    lua_error("cdLineStyleDashes: count parameter is nil!");
  if (!lua_isnumber(dashes))
    lua_error("cdLineStyleDashes: invalid count parameter!");
  
  dashes_count = (int)lua_getnumber(count);
  dashes_int = malloc(dashes_count*sizeof(int));

  for (i=0; i < dashes_count; i++)
  {
    lua_pushobject(dashes);
    lua_pushnumber(i+1);
    value = lua_gettable();

    if (!lua_isnumber(value))
      lua_error("cdLineStyleDashes: invalid dash!");

    dashes_int[i] = (int)lua_getnumber(value);
  }

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdLineStyleDashes: too many parameters!");
  
  cdLineStyleDashes(dashes_int, dashes_count);
  free(dashes_int);
}

/***************************************************************************\
* Frees a previously alocated canvas.                                       *
\***************************************************************************/
static void cdlua_killcanvas(void)
{
  lua_Object canvas;
  canvas_t *canvas_p;
  cdCanvas *current_canvas;

  canvas = lua_getparam(1);
  
  if (canvas == LUA_NOOBJECT)
    lua_error("cdKillCanvas: canvas parameter missing!");

  /* if the creation failed, canvas can be nil, in which case we */
  /* issue an error */
  if (lua_isnil(canvas))
    lua_error("cdKillCanvas: attempt to kill a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("cdKillCanvas: invalid canvas parameter!");
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) 
    lua_error("cdKillCanvas: attempt to kill a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillCanvas: too many parameters!");
    
  /* find out about the currently active canvas */
  current_canvas = cdActiveCanvas();

  /* this should never happen, unless the user did it on purpouse! */
  if (canvas_p->cd_canvas == void_canvas)
    lua_error("cdKillCanvas: trying to kill the void canvas???");
  
  /* if the user killed the currently active canvas, activate void canvas */
  if (canvas_p->cd_canvas == current_canvas) {
    cdActivate(void_canvas);
  }

  cdKillCanvas(canvas_p->cd_canvas);
  canvas_p->cd_canvas = NULL;
}

/***************************************************************************\
* Creates a color as a color_tag usertag lua_Object. The color value is     *
* placed in the (void *) value. Not beautiful, but works best.              * 
\***************************************************************************/
static void cdlua_encodecolor(void)
{
  lua_Object red, green, blue;
  float red_f, green_f, blue_f;
  unsigned char red_i, green_i, blue_i;
  long int color_i;

  red = lua_getparam(1);
  green = lua_getparam(2);
  blue = lua_getparam(3);
  if (!(lua_isnumber(red) && lua_isnumber(green) && lua_isnumber(blue)))
    lua_error("cdEncodeColor: invalid color component parameter!");
  red_f = (float) lua_getnumber(red);
  green_f = (float) lua_getnumber(green);
  blue_f = (float) lua_getnumber(blue);
  if (red_f < 0 || red_f > 255 || green_f < 0 || 
    green_f > 255 || blue_f < 0 || blue_f > 255)
    lua_error("cdEncodeColor: color components values should be in range [0, 255]!");
  red_i = (unsigned char) (red_f);
  green_i = (unsigned char) (green_f);
  blue_i = (unsigned char) (blue_f);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdEncodeColor: too many parameters!");

  color_i = cdEncodeColor(red_i, green_i, blue_i);
  lua_pushusertag((void *) color_i, color_tag);
}

static void cdlua_encodealpha(void)
{
  lua_Object color, alpha;
  float alpha_f;
  unsigned char alpha_i;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdEncodeAlpha: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  alpha = lua_getparam(2);
  if (!lua_isnumber(alpha))
    lua_error("cdEncodeAlpha: invalid alpha parameter!");
  alpha_f = (float) lua_getnumber(alpha);
  if (alpha_f < 0 || alpha_f > 255)
    lua_error("cdEncodeAlpha: alpha components values should be in range [0, 255]!");
  alpha_i = (unsigned char) (alpha_f);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdEncodeAlpha: too many parameters!");

  color_i = cdEncodeAlpha(color_i, alpha_i);
  lua_pushusertag((void *) color_i, color_tag);
}

/***************************************************************************\
* Decodes a color previously created.                                       *
\***************************************************************************/
static void cdlua_decodecolor(void)
{
  lua_Object color;
  long int color_i;
  unsigned char red_i, green_i, blue_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdDecodeColor: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdDecodeColor: too many parameters!");

  cdDecodeColor(color_i, &red_i, &green_i, &blue_i);
  lua_pushnumber(red_i);
  lua_pushnumber(green_i);
  lua_pushnumber(blue_i);
}

static void cdlua_decodealpha(void)
{
  lua_Object color;
  long int color_i;
  unsigned char alpha_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdDecodeAlpha: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdDecodeAlpha: too many parameters!");

  alpha_i = cdDecodeAlpha(color_i);
  lua_pushnumber(alpha_i);
}

static void cdlua_alpha(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdRed: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdAlpha: too many parameters!");

  lua_pushnumber(cdAlpha(color_i));
}

static void cdlua_red(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdRed: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdRed: too many parameters!");

  lua_pushnumber(cdRed(color_i));
}

static void cdlua_blue(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdBlue: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdBlue: too many parameters!");

  lua_pushnumber(cdBlue(color_i));
}

static void cdlua_green(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdGreen: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdGreen: too many parameters!");

  lua_pushnumber(cdGreen(color_i));
}

static void cdlua_reserved(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdReserved: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdReserved: too many parameters!");

  lua_pushnumber(cdReserved(color_i));
}

/***************************************************************************\
* Creates a stipple as a stipple_tag usertag lua_Object.                    *
\***************************************************************************/
static void cdlua_createstipple(void)
{
  lua_Object width, height;
  long int width_i, height_i;
  stipple_t *stipple_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateStipple: invalid dimension parameters!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateStipple: stipple dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreateStipple: too many parameters!");
  
  stipple_p = (stipple_t *) malloc(sizeof(stipple_t));
  if (!stipple_p) {
    lua_pushnil();
    return;
  }

  stipple_p->size = width_i*height_i;
  stipple_p->height = height_i;
  stipple_p->width = width_i;
  stipple_p->value = (unsigned char *) malloc(stipple_p->size);
  if (!stipple_p->value) {
    free(stipple_p);
    lua_pushnil();
    return;
  }

  memset(stipple_p->value, '\0', stipple_p->size);
  lua_pushusertag((void *) stipple_p, stipple_tag);
}

static void cdlua_getstipple(void)
{
  int width, height;
  unsigned char * stipple;
  stipple_t *stipple_p;

  stipple = cdGetStipple(&width, &height);

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdGetStipple: too many parameters!");
  
  stipple_p = (stipple_t *) malloc(sizeof(stipple_t));
  if (!stipple_p) {
    lua_pushnil();
    return;
  }

  stipple_p->size = width*height;
  stipple_p->height = height;
  stipple_p->width = width;
  stipple_p->value = (unsigned char *) malloc(stipple_p->size);
  if (!stipple_p->value) {
    free(stipple_p);
    lua_pushnil();
    return;
  }

  memcpy(stipple_p->value, stipple, stipple_p->size);
  lua_pushusertag((void *) stipple_p, stipple_tag);
}

/***************************************************************************\
* Frees a previously allocated stipple. We don't free stipple_p to prevent  *
* a problem if the user called killstipple twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killstipple(void)
{
  lua_Object stipple;
  stipple_t *stipple_p;

  stipple = lua_getparam(1);
  if (stipple == LUA_NOOBJECT)
    lua_error("cdKillStipple: stipple parameter missing!");
  if (lua_isnil(stipple))
    lua_error("cdKillStipple: attempt to kill a NIL stipple!");
  if (lua_tag(stipple) != stipple_tag)
    lua_error("cdKillStipple: invalid stipple parameter!");
  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p->value) 
    lua_error("cdKillStipple: attempt to kill a killed stipple!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillStipple: too many parameters!");

  free(stipple_p->value);
  stipple_p->value = NULL;
}

/***************************************************************************\
* Creates a pattern as a pattern_tag usertag lua_Object. A pattern can be   *
* considered and treated as a color table.                                  *
\***************************************************************************/
static void cdlua_createpattern(void)
{
  lua_Object width, height;
  long int width_i, height_i;
  pattern_t *pattern_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreatePattern: invalid dimension parameters!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreatePattern: pattern dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreatePattern: too many parameters!");

  pattern_p = (pattern_t *) malloc(sizeof(pattern_t));
  if (!pattern_p) {
    lua_pushnil();
    return;
  }

  pattern_p->size = width_i*height_i;
  pattern_p->width = width_i;
  pattern_p->height = height_i;
  pattern_p->color = (long int *) malloc(pattern_p->size * sizeof(long int));
  if (!pattern_p->color) {
    free(pattern_p);
    lua_pushnil();
    return;
  }

  memset(pattern_p->color, 255, pattern_p->size * sizeof(long int));
  lua_pushusertag((void *) pattern_p, pattern_tag);
}

static void cdlua_getpattern(void)
{
  int width, height;
  long int * pattern;
  pattern_t *pattern_p;

  pattern = cdGetPattern(&width, &height);

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdGetPattern: too many parameters!");
  
  pattern_p = (pattern_t *) malloc(sizeof(pattern_t));
  if (!pattern_p) {
    lua_pushnil();
    return;
  }

  pattern_p->size = width*height;
  pattern_p->height = height;
  pattern_p->width = width;
  pattern_p->color = (long int *) malloc(pattern_p->size * sizeof(long int));
  if (!pattern_p->color) {
    free(pattern_p);
    lua_pushnil();
    return;
  }

  memcpy(pattern_p->color, pattern, pattern_p->size * sizeof(long int));
  lua_pushusertag((void *) pattern_p, pattern_tag);
}

/***************************************************************************\
* Frees a previously allocated pattern. We don't free pattern_p to prevent  *
* a problem if the user called killpattern twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killpattern(void)
{
  lua_Object pattern;
  pattern_t *pattern_p;

  pattern = lua_getparam(1);

  if (pattern == LUA_NOOBJECT)
    lua_error("cdKillPattern: pattern parameter missing!");
  if (lua_isnil(pattern))
    lua_error("cdKillPattern: attempt to kill a NIL pattern!");
  if (lua_tag(pattern) != pattern_tag)
    lua_error("cdKillPattern: invalid pattern parameter!");
  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p->color) 
    lua_error("cdKillPattern: attempt to kill a killed pattern!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillPattern: too many parameters!");

  free(pattern_p->color);
  pattern_p->color = NULL;
}

/***************************************************************************\
* Creates a palette as a palette_tag usertag lua_Object. A palette can be   *
* considered and treated as a color table.                                  *
\***************************************************************************/
static void cdlua_createpalette(void)
{
  lua_Object size;
  long int size_i;
  palette_t *palette_p;

  size = lua_getparam(1);
  if (!(lua_isnumber(size)))
    lua_error("cdCreatePalette: invalid size parameter!");
  size_i = (long int) lua_getnumber(size);
  if (size_i < 1)
    lua_error("cdCreatePalette: palette size should be a positive integer!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdCreatePalette: too many parameters!");

  palette_p = (palette_t *) malloc(sizeof(palette_t));
  if (!palette_p) {
    lua_pushnil();
    return;
  }

  palette_p->size = size_i;
  palette_p->color = (long int *) malloc(palette_p->size * sizeof(long int));
  if (!palette_p->color) {
    free(palette_p);
    lua_pushnil();
    return;
  }

  memset(palette_p->color, 255, palette_p->size * sizeof(long int));
  lua_pushusertag((void *) palette_p, palette_tag);
}

/***************************************************************************\
* Frees a previously allocated palette. We don't free palette_p to prevent  *
* a problem if the user called killpalette twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killpalette(void)
{
  lua_Object palette;
  palette_t *palette_p;

  palette = lua_getparam(1);
  if (palette == LUA_NOOBJECT)
    lua_error("cdKillPalette: palette parameter missing!");
  if (lua_isnil(palette))
    lua_error("cdKillPalette: attempt to kill a NIL palette!");
  if (lua_tag(palette) != palette_tag)
    lua_error("cdKillPalette: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color) 
    lua_error("cdKillPalette: attempt to kill a killed palette!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillPalette: too many parameters!");

  free(palette_p->color);
  palette_p->color = NULL;
}

/***************************************************************************\
* Image Extended Functions.                                                 *
\***************************************************************************/

static void cdlua_createbitmap(void)
{
  lua_Object width;
  lua_Object height;
  lua_Object type;

  long int width_i;
  long int height_i;
  int type_i;
  bitmap_t *image_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  type = lua_getparam(3);
  if (!(lua_isnumber(type) && lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateBitmap: invalid parameters!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  type_i = (long int) lua_getnumber(type);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateBitmap: imagemap dimensions should be positive integers!");

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdCreateBitmap: too many parameters!");
  
  image_p = (bitmap_t *) malloc(sizeof(bitmap_t));
  if (!image_p) {
    lua_pushnil();
    return;
  }
  
  image_p->image = cdCreateBitmap(width_i, height_i, type_i);
  if (!image_p->image) {
    free(image_p);
    lua_pushnil();
  }
  else 
    lua_pushusertag((void *) image_p, bitmap_tag);
}

static void cdlua_killbitmap(void)
{
  lua_Object image;
  bitmap_t *image_p;

  image = lua_getparam(1);
  if (image == LUA_NOOBJECT)
    lua_error("cdKillBitmap: image parameter missing!");
  if (lua_isnil(image))
    lua_error("cdKillBitmap: attempt to kill a NIL image!");
  if (lua_tag(image) != bitmap_tag)
    lua_error("cdKillBitmap: invalid image parameter!");
  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p->image)
    lua_error("cdKillBitmap: attempt to kill a killed image");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillBitmap: too many parameters!");

  cdKillBitmap(image_p->image);
  image_p->image = NULL;
}

static void cdlua_getbitmap(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;

  bitmap_t *image_p;
  int x_i;
  int y_i;

  image = lua_getparam(1);
  if (image == LUA_NOOBJECT)
    lua_error("cdGetBitmap: image parameter missing!");
  if (lua_isnil(image))
    lua_error("cdGetBitmap: attempt to get NIL image");
  if (lua_tag(image) != bitmap_tag)
    lua_error("cdGetBitmap: invalid image parameter!");
  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p->image)
    lua_error("cdGetBitmap: attempt to get a killed image");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("cdGetBitmap: invalid (x, y) parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdGetBitmap: too many parameters!");

  cdGetBitmap(image_p->image, x_i, y_i);
}

static void cdlua_putbitmap(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;

  bitmap_t *image_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;

  image = lua_getparam(1);
  if (image == LUA_NOOBJECT)
    lua_error("cdPutBitmap: image parameter missing!");
  if (lua_isnil(image))
    lua_error("cdPutBitmap: attempt to put a NIL image!");
  if (lua_tag(image) != bitmap_tag)
    lua_error("cdPutBitmap: invalid image parameter!");
  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p->image)
    lua_error("cdPutBitmap: attempt to put a killed image!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h)))
    lua_error("cdPutBitmap: invalid (x, y) parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutBitmap: target region dimensions should be positive integers!");

  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("cdPutBitmap: too many parameters!");

  cdPutBitmap(image_p->image, x_i, y_i, w_i, h_i);
}

static void wdlua_putbitmap(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;

  bitmap_t *image_p;
  double x_i;
  double y_i;
  double w_i;
  double h_i;

  image = lua_getparam(1);
  if (image == LUA_NOOBJECT)
    lua_error("wdPutBitmap: image parameter missing!");
  if (lua_isnil(image))
    lua_error("wdPutBitmap: attempt to put a NIL image!");
  if (lua_tag(image) != bitmap_tag)
    lua_error("wdPutBitmap: invalid image parameter!");
  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p->image)
    lua_error("wdPutBitmap: attempt to put a killed image!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h)))
    lua_error("wdPutBitmap: invalid (x, y) parameter!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);
  w_i = (double) lua_getnumber(w);
  h_i = (double) lua_getnumber(h);
  if (w_i < 0 || h_i < 0)
    lua_error("wdPutBitmap: target region dimensions should be positive integers!");

  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("wdPutBitmap: too many parameters!");

  wdPutBitmap(image_p->image, x_i, y_i, w_i, h_i);
}

static void cdlua_bitmapsetrect(void)
{
  lua_Object image;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  bitmap_t *image_p;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  image = lua_getparam(1);
  if (image == LUA_NOOBJECT)
    lua_error("cdBitmapSetRect: image parameter missing!");
  if (lua_isnil(image))
    lua_error("cdBitmapSetRect: attempt to get a NIL image!");
  if (lua_tag(image) != bitmap_tag)
    lua_error("cdBitmapSetRect: invalid image parameter!");
  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p->image)
    lua_error("cdBitmapSetRect: attempt to get a killed image!");

  xmin = lua_getparam(2);
  xmax = lua_getparam(3);
  ymin = lua_getparam(4);
  ymax = lua_getparam(5);
  if (!(lua_isnumber(xmin) && lua_isnumber(xmax) && 
    lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("cdBitmapSetRect: invalid parameter!");
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);

  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("cdBitmapSetRect: too many parameters!");

  cdBitmapSetRect(image_p->image, xmin_i, xmax_i, ymin_i, ymax_i);
}

static void cdlua_rgb2mapex(void)
{
  lua_Object imagemap;
  lua_Object imagergb;

  bitmap_t *imagemap_p;
  bitmap_t *imagergb_p;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdBitmapRGB2Map: attempt to put a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdBitmapRGB2Map: invalid imagergb parameter!");
  imagergb_p = (bitmap_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->image))
    lua_error("cdBitmapRGB2Map: attempt to put a killed imagergb!");
  
  imagemap = lua_getparam(2);
  if (lua_isnil(imagemap))
    lua_error("cdBitmapRGB2Map: attempt to put a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("cdBitmapRGB2Map: imagemap invalid parameter!");
  imagemap_p = (bitmap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->image)
    lua_error("cdBitmapRGB2Map: attempt to put a killed imagemap!");

  if (imagergb_p->image->type != CD_RGB || imagemap_p->image->type <= 0)
    lua_error("cdBitmapRGB2Map: invalid image type!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdBitmapRGB2Map: too many parameters!");

  cdBitmapRGB2Map(imagergb_p->image, imagemap_p->image);
}

/***************************************************************************\
* Creates a buffer for a RGB image.                                         *
\***************************************************************************/
static void cdlua_createimagergb(void)
{
  lua_Object width, height;
  long int width_i, height_i;
  imagergb_t *imagergb_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateImageRGB: invalid imagergb parameter!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateImageRGB: image dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreateImageRGB: too many parameters!");
  
  imagergb_p = (imagergb_t *) malloc(sizeof(imagergb_t));
  if (!imagergb_p) {
    lua_pushnil();
    return;
  }

  imagergb_p->width = width_i;
  imagergb_p->height = height_i;
  imagergb_p->size = width_i*height_i;
  imagergb_p->red = (unsigned char *) malloc(imagergb_p->size);
  imagergb_p->green = (unsigned char *) malloc(imagergb_p->size);
  imagergb_p->blue = (unsigned char *) malloc(imagergb_p->size);
  
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue)) { 
    if (imagergb_p->red) free(imagergb_p->red);
    if (imagergb_p->green) free(imagergb_p->green);
    if (imagergb_p->blue) free(imagergb_p->blue);
    free(imagergb_p);
    lua_pushnil();
    return;
  }

  memset(imagergb_p->red, 255, imagergb_p->size);
  memset(imagergb_p->green, 255, imagergb_p->size);
  memset(imagergb_p->blue, 255, imagergb_p->size);
  
  lua_pushusertag((void *) imagergb_p, imagergb_tag);
}

static void cdlua_imagergb(void)
{
  lua_Object canvas;
  canvas_t *canvas_p;
  cdCanvas *current_canvas;
  int w, h, type = CD_RGB;

  canvas = lua_getparam(1);
  
  /* if the creation failed, canvas can be nil, in which case we */
  /* issue an error */
  if (lua_isnil(canvas))
    lua_error("cdImageRGB: attempt to get a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("cdImageRGB: invalid canvas parameter!");
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) 
    lua_error("cdImageRGB: attempt to get a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdImageRGB: too many parameters!");

  if (cdAlphaImage(canvas_p->cd_canvas))
    type = CD_RGBA;

  current_canvas = cdActiveCanvas();
  cdActivate(canvas_p->cd_canvas);
  cdGetCanvasSize(&w, &h, NULL, NULL);
  cdActivate(current_canvas);

  if (type == CD_RGBA)
  {
    imagergba_t *imagergba_p = (imagergba_t *) malloc(sizeof(imagergba_t));
    if (!imagergba_p) {
      lua_pushnil();
      return;
    }

    imagergba_p->width = w;
    imagergba_p->height = h;
    imagergba_p->size = w*h;
    imagergba_p->red = cdRedImage(canvas_p->cd_canvas);
    imagergba_p->green = cdGreenImage(canvas_p->cd_canvas);
    imagergba_p->blue = cdBlueImage(canvas_p->cd_canvas);
    imagergba_p->blue = cdAlphaImage(canvas_p->cd_canvas);
    
    lua_pushusertag((void *) imagergba_p, imagergba_tag);
  }
  else
  {
    imagergb_t * imagergb_p = (imagergb_t *) malloc(sizeof(imagergb_t));
    if (!imagergb_p) {
      lua_pushnil();
      return;
    }

    imagergb_p->width = w;
    imagergb_p->height = h;
    imagergb_p->size = w*h;
    imagergb_p->red = cdRedImage(canvas_p->cd_canvas);
    imagergb_p->green = cdGreenImage(canvas_p->cd_canvas);
    imagergb_p->blue = cdBlueImage(canvas_p->cd_canvas);
    
    lua_pushusertag((void *) imagergb_p, imagergb_tag);
  }
}

static void cdlua_imagergbbitmap(void)
{
  lua_Object canvas;
  canvas_t *canvas_p;
  cdCanvas *current_canvas;
  bitmap_t *image_p;
  int w, h, type = CD_RGB;

  canvas = lua_getparam(1);
  
  /* if the creation failed, canvas can be nil, in which case we */
  /* issue an error */
  if (lua_isnil(canvas))
    lua_error("cdImageRGBBitmap: attempt to get a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("cdImageRGBBitmap: invalid canvas parameter!");
  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas) 
    lua_error("cdImageRGBBitmap: attempt to get a killed canvas!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdImageRGBBitmap: too many parameters!");

  if (cdAlphaImage(canvas_p->cd_canvas))
    type = CD_RGBA;

  current_canvas = cdActiveCanvas();
  cdActivate(canvas_p->cd_canvas);
  cdGetCanvasSize(&w, &h, NULL, NULL);
  cdActivate(current_canvas);

  image_p = (bitmap_t *) malloc(sizeof(bitmap_t));
  if (!image_p) {
    lua_pushnil();
    return;
  }
  
  image_p->image = cdInitBitmap(w, h, type, 
                                cdRedImage(canvas_p->cd_canvas),
                                cdGreenImage(canvas_p->cd_canvas),
                                cdBlueImage(canvas_p->cd_canvas),
                                cdAlphaImage(canvas_p->cd_canvas));

  lua_pushusertag((void *)image_p, bitmap_tag);
}

/***************************************************************************\
* Frees a previously allocated imagergb. We don't free imagergb_p to avoid  *
* problems if the user called killimagergb twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killimagergb(void)
{
  lua_Object imagergb;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdKillImageRGB: attempt to kill a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdKillImageRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue)) 
    lua_error("cdKillImageRGB: attempt to kill a killed imagergb!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillImageRGB: too many parameters!");
  
  free(imagergb_p->red);
  free(imagergb_p->green);
  free(imagergb_p->blue);
  imagergb_p->red = NULL;
  imagergb_p->green = NULL;
  imagergb_p->blue = NULL;
}

/***************************************************************************\
* Creates a buffer for a RGBA image.                                        *
\***************************************************************************/
static void cdlua_createimagergba(void)
{
  lua_Object width, height;
  long int width_i, height_i;
  imagergba_t *imagergba_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateImageRGBA: invalid imagergba parameter!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateImageRGBA: image dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreateImageRGBA: too many parameters!");
  
  imagergba_p = (imagergba_t *) malloc(sizeof(imagergba_t));
  if (!imagergba_p) {
    lua_pushnil();
    return;
  }

  imagergba_p->width = width_i;
  imagergba_p->height = height_i;
  imagergba_p->size = width_i*height_i;
  imagergba_p->red = (unsigned char *) malloc(imagergba_p->size);
  imagergba_p->green = (unsigned char *) malloc(imagergba_p->size);
  imagergba_p->blue = (unsigned char *) malloc(imagergba_p->size);
  imagergba_p->alpha = (unsigned char *) malloc(imagergba_p->size);
  
  if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue && imagergba_p->alpha)) { 
    if (imagergba_p->red) free(imagergba_p->red);
    if (imagergba_p->green) free(imagergba_p->green);
    if (imagergba_p->blue) free(imagergba_p->blue);
    if (imagergba_p->alpha) free(imagergba_p->alpha);
    free(imagergba_p);
    lua_pushnil();
    return;
  }

  memset(imagergba_p->red, 255, imagergba_p->size);
  memset(imagergba_p->green, 255, imagergba_p->size);
  memset(imagergba_p->blue, 255, imagergba_p->size);
  memset(imagergba_p->alpha, 255, imagergba_p->size);
  
  lua_pushusertag((void *) imagergba_p, imagergba_tag);
}

/***************************************************************************\
* Frees a previously allocated imagergba. Don't free imagergba_p to avoid   *
* problems if the user called killimagergba twice with the same object. The *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killimagergba(void)
{
  lua_Object imagergba;
  imagergba_t *imagergba_p;

  imagergba = lua_getparam(1);
  if (lua_isnil(imagergba))
    lua_error("cdKillImageRGBA: attempt to kill a NIL imagergba!");
  if (lua_tag(imagergba) != imagergba_tag)
    lua_error("cdKillImageRGBA: invalid imagergba parameter!");
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue && imagergba_p->alpha))
    lua_error("cdKillImageRGBA: attempt to kill a killed imagergba!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillImageRGBA: too many parameters!");
  
  free(imagergba_p->red);
  free(imagergba_p->green);
  free(imagergba_p->blue);
  free(imagergba_p->alpha);
  imagergba_p->red = NULL;
  imagergba_p->green = NULL;
  imagergba_p->blue = NULL;
  imagergba_p->alpha = NULL;
}

/***************************************************************************\
* Creates a imagemap as a imagemap_tag usertag lua_Object.                  *
\***************************************************************************/
static void cdlua_createimagemap(void)
{
  lua_Object width;
  lua_Object height;

  long int width_i;
  long int height_i;
  imagemap_t *imagemap_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateImageMap: invalid imagemap parameter!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateImageMap: imagemap dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreateImageMap: too many parameters!");
  
  imagemap_p = (imagemap_t *) malloc(sizeof(imagemap_t));
  if (!imagemap_p) {
    lua_pushnil();
    return;
  }

  imagemap_p->size = width_i*height_i;
  imagemap_p->width = width_i;
  imagemap_p->height = height_i;
  imagemap_p->index = (unsigned char *) malloc(imagemap_p->size);
  if (!imagemap_p->index) {
    free(imagemap_p);
    lua_pushnil();
    return;
  }

  memset(imagemap_p->index, 0, imagemap_p->size);
  lua_pushusertag((void *) imagemap_p, imagemap_tag);
}

/***************************************************************************\
* Frees a previously allocated imagemap. We don't free imagemap_p to avoid  *
* problems if the user called killimagemap twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void cdlua_killimagemap(void)
{
  lua_Object imagemap;
  imagemap_t *imagemap_p;

  imagemap = lua_getparam(1);
  if (lua_isnil(imagemap))
    lua_error("cdKillImageMap: attempt to kill a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("cdKillImageMap: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index) 
    lua_error("cdKillImageMap: attempt to kill a killed imagemap!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillImageMap: too many parameters!");
    
  free(imagemap_p->index);
  imagemap_p->index = NULL;
}

/***************************************************************************\
* Creates an image as a image_tag usertag lua_Object.                       *
\***************************************************************************/
static void cdlua_createimage(void)
{
  lua_Object width;
  lua_Object height;

  long int width_i;
  long int height_i;
  image_t *image_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("cdCreateImage: invalid dimension parameters!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("cdCreateImage: imagemap dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdCreateImage: too many parameters!");
  
  image_p = (image_t *) malloc(sizeof(image_t));
  if (!image_p) {
    lua_pushnil();
    return;
  }
  
  image_p->cd_image = cdCreateImage(width_i, height_i);
  if (!image_p->cd_image) {
    free(image_p);
    lua_pushnil();
  }
  else 
    lua_pushusertag((void *) image_p, image_tag);
}

/***************************************************************************\
* Frees a previously allocated image.                                       *
\***************************************************************************/
static void cdlua_killimage(void)
{
  lua_Object image;
  image_t *image_p;

  image = lua_getparam(1);
  if (lua_isnil(image))
    lua_error("cdKillImage: attempt to kill a NIL image!");
  if (lua_tag(image) != image_tag)
    lua_error("cdKillImage: invalid image parameter!");
  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("cdKillImage: attempt to kill a killed image");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdKillImage: too many parameters!");

  cdKillImage(image_p->cd_image);
  image_p->cd_image = NULL;
}

/***************************************************************************\
* Fallback definitions.                                                     *
\***************************************************************************/
/***************************************************************************\
* stipple "settable" fallback.                                              *
\***************************************************************************/
static void stipplesettable_fb(void)
{
  lua_Object stipple, index, value; 
  
  stipple_t *stipple_p;
  long int index_i;
  unsigned char value_i;

  stipple = lua_getparam(1);
  index = lua_getparam(2);
  value = lua_getparam(3);

  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p) {
    lua_error("stipple_tag \"settable\": invalid stipple_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("stipple_tag \"settable\": index should be a number!");
  }

  if (!lua_isnumber(value)) {
    lua_error("stipple_tag \"settable\": value should be a number!");
  }
  
  value_i = (unsigned char) lua_getnumber(value);
  if ((value_i != 0 && value_i != 1)) 
    lua_error("stipple_tag \"settable\": value should belong to {0, 1}!");

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= stipple_p->size)
    lua_error("stipple_tag \"settable\": index is out of bounds!");

  stipple_p->value[index_i] = value_i;
}

/***************************************************************************\
* imagemap "settable" fallback.                                             *
\***************************************************************************/
static void imagemapsettable_fb(void)
{
  lua_Object imagemap, index, value;

  imagemap_t *imagemap_p;
  long int index_i;
  long int value_i;

  imagemap = lua_getparam(1);
  index = lua_getparam(2);
  value = lua_getparam(3);

  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p) {
    lua_error("imagemap_tag \"settable\": invalid imagemap_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("imagemap_tag \"settable\": index should be a number!");
  }

  if (!lua_isnumber(value)) {
    lua_error("imagemap_tag \"settable\": value should be a number!");
  }
  
  value_i = (long int) lua_getnumber(value);
  if ((value_i < 0 || value_i > 255)) 
    lua_error("imagemap_tag \"settable\": value should be in range [0, 255]!");

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= imagemap_p->size)
    lua_error("imagemap_tag \"settable\": index is out of bounds!");

  imagemap_p->index[index_i] = (unsigned char) value_i;
}

/***************************************************************************\
* pattern "settable" fallback.                                              *
\***************************************************************************/
static void patternsettable_fb(void)
{
  lua_Object pattern, index, color;

  pattern_t *pattern_p;
  long int index_i;
  long int color_i;

  pattern = lua_getparam(1);
  index = lua_getparam(2);
  color = lua_getparam(3);

  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p) {
    lua_error("pattern_tag \"settable\": invalid pattern_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("pattern_tag \"settable\": index should be a number!");
  }

  if (lua_tag(color) != color_tag) 
    lua_error("pattern_tag \"settable\": value should be of type color_tag!");
  
  color_i = (long int) lua_getuserdata(color);
  
  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= pattern_p->size)
    lua_error("pattern_tag \"settable\": index is out of bounds!");

  pattern_p->color[index_i] = color_i;
}

/***************************************************************************\
* palette "settable" fallback.                                              *
\***************************************************************************/
static void palettesettable_fb(void)
{
  lua_Object palette, index, color;
  
  palette_t *palette_p;
  long int index_i;
  long int color_i;

  palette = lua_getparam(1);
  index = lua_getparam(2);
  color = lua_getparam(3);

  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p) {
    lua_error("palette_tag \"settable\": invalid palette_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("palette_tag \"settable\": index should be a number!");
  }

  if (lua_tag(color) != color_tag) 
    lua_error("palette_tag \"settable\": value should be of type color_tag!");
  
  color_i = (long int) lua_getuserdata(color);
  
  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= palette_p->size)
    lua_error("palette_tag \"settable\": index is out of bounds!");

  palette_p->color[index_i] = color_i;
}

/***************************************************************************\
* channel "settable" fallback. This fallback is called when a LUA line like *
* "imagergb.r[y*w + x] = c" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the value is assigned where it should.         *
\***************************************************************************/
static void channelsettable_fb(void)
{
  lua_Object channel, index, value;
  
  channel_t *channel_p;
  long int index_i;
  long int value_i;

  channel = lua_getparam(1);
  index = lua_getparam(2);
  value = lua_getparam(3);

  channel_p = (channel_t *) lua_getuserdata(channel);
  if (!channel_p) {
    lua_error("channel_tag \"settable\": invalid channel_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("channel_tag \"settable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || (channel_p->size > 0 && index_i >= channel_p->size) || 
                     (channel_p->size == -1 && index_i >= 256)) {
    lua_error("channel_tag \"settable\": index is out of bounds!");
  }
  
  if (channel_p->size > 0)
  {
    if (!lua_isnumber(value)) {
      lua_error("channel_tag \"settable\": value should be a number!");
    }

    value_i = (long int) lua_getnumber(value);
    if ((value_i < 0 || value_i > 255)) {
      lua_error("channel_tag \"settable\": value should be in range [0, 255]!");
    }

    channel_p->value[index_i] = (unsigned char) value_i;
  }
  else
  {
    if (lua_tag(value) != color_tag) 
      lua_error("channel_tag \"settable\": value should be of type color_tag!");

    value_i = (long int) lua_getuserdata(value);

    ((long int*)channel_p->value)[index_i] = value_i;
  }
}

/***************************************************************************\
* stipple "gettable" fallback.                                              *
\***************************************************************************/
static void stipplegettable_fb(void)
{
  lua_Object stipple, index;
  
  stipple_t *stipple_p;
  long int index_i;

  stipple = lua_getparam(1);
  index = lua_getparam(2);

  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p)
    lua_error("stipple_tag \"gettable\": invalid stipple_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("stipple_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= stipple_p->size)
    lua_error("stipple_tag \"gettable\": index is out of bounds!");

  lua_pushnumber(stipple_p->value[index_i]);
}

/***************************************************************************\
* imagemap "gettable" fallback.                                             *
\***************************************************************************/
static void imagemapgettable_fb(void)
{
  lua_Object imagemap, index;

  imagemap_t *imagemap_p;
  long int index_i;

  imagemap = lua_getparam(1);
  index = lua_getparam(2);

  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p)
    lua_error("imagemap_tag \"gettable\": invalid imagemap_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("imagemap_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= imagemap_p->size)
    lua_error("imagemap_tag \"gettable\": index is out of bounds!");

  lua_pushnumber(imagemap_p->index[index_i]);
}

/***************************************************************************\
* pattern "gettable" fallback.                                              *
\***************************************************************************/
static void patterngettable_fb(void)
{
  lua_Object pattern, index;

  pattern_t *pattern_p;
  long int index_i;

  pattern = lua_getparam(1);
  index = lua_getparam(2);

  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p)
    lua_error("pattern_tag \"gettable\": invalid pattern_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("pattern_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= pattern_p->size)
    lua_error("pattern_tag \"gettable\": index is out of bounds!");

  lua_pushusertag((void *) pattern_p->color[index_i], color_tag);
}

/***************************************************************************\
* palette "gettable" fallback.                                              *
\***************************************************************************/
static void palettegettable_fb(void)
{
  lua_Object palette, index;
  
  palette_t *palette_p;
  long int index_i;

  palette = lua_getparam(1);
  index = lua_getparam(2);

  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p)
    lua_error("palette_tag \"gettable\": invalid palette_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("palette_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= palette_p->size)
    lua_error("palette_tag \"gettable\": index is out of bounds!");

  lua_pushusertag((void *) palette_p->color[index_i], color_tag);
}

/***************************************************************************\
* channel "gettable" fallback. This fallback is called when a LUA line like *
* "c = imagergb.r[y*w + x]" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the appropriate value is returned.             *
\***************************************************************************/
static void channelgettable_fb(void)
{
  lua_Object channel, index;
  
  channel_t *channel_p;
  long int index_i;

  channel = lua_getparam(1);
  index = lua_getparam(2);

  channel_p = (channel_t *) lua_getuserdata(channel);
  if (!channel_p) {
    lua_error("channel_tag \"gettable\": invalid channel_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("channel_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);

  if (index_i < 0 || (channel_p->size > 0 && index_i >= channel_p->size) || 
                     (channel_p->size == -1 && index_i >= 256)) {
    lua_error("channel_tag \"gettable\": index is out of bounds!");
  }
  
  if (channel_p->size == -1)
    lua_pushusertag((void *) ((long int*)channel_p->value)[index_i], color_tag);
  else
    lua_pushnumber(channel_p->value[index_i]);
}

/***************************************************************************\
* imagergb "gettable" fallback. This fallback is called when a LUA line     *
* like "c = imagergb.r[y*w + x]" or "imagergb.r[y*w + x] = c" is executed.  *
* The channel_info global is filled and its address is returned with a      *
* channel_tag usertag lua_Object. The following "gettable" or "settable"    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static void imagergbgettable_fb(void)
{
  lua_Object imagergb, index;
  
  char *index_s;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  index = lua_getparam(2);

  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!imagergb_p)
    lua_error("imagergb_tag \"gettable\": invalid imagergb_tag object!");

  if (!lua_isstring(index)) {
    lua_error("imagergb_tag \"gettable\": index should be a channel name!");
  }
  index_s = (char *) lua_getstring(index);

  channel_info.size = imagergb_p->size;
  
  if (*index_s == 'r' || *index_s == 'R') {
    channel_info.value = imagergb_p->red;
  }
  else if (*index_s == 'g' || *index_s == 'G') {
    channel_info.value = imagergb_p->green;
  }
  else if (*index_s == 'b' || *index_s == 'B') {
    channel_info.value = imagergb_p->blue;
  }
  else {
    lua_error("imagergb_tag \"gettable\": index is an invalid channel name!");
  }

  lua_pushusertag((void *) &channel_info, channel_tag);
}

/***************************************************************************\
* imagergba "gettable" fallback. This fallback is called when a LUA line    *
* like "c = imagergba.r[y*w + x]" or "imagergba.r[y*w + x] = c" is executed.*
* The channel_info global is filled and its address is returned with a      *
* channel_tag usertag lua_Object. The following "gettable" or "settable"    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static void imagergbagettable_fb(void)
{
  lua_Object imagergba, index;
  
  char *index_s;
  imagergba_t *imagergba_p;

  imagergba = lua_getparam(1);
  index = lua_getparam(2);

  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!imagergba_p)
    lua_error("imagergba_tag \"gettable\": invalid imagergba_tag object!");

  if (!lua_isstring(index)) {
    lua_error("imagergba_tag \"gettable\": index should be a channel name!");
  }
  index_s = (char *) lua_getstring(index);

  channel_info.size = imagergba_p->size;
  
  if (*index_s == 'r' || *index_s == 'R') {
    channel_info.value = imagergba_p->red;
  }
  else if (*index_s == 'g' || *index_s == 'G') {
    channel_info.value = imagergba_p->green;
  }
  else if (*index_s == 'b' || *index_s == 'B') {
    channel_info.value = imagergba_p->blue;
  }
  else if (*index_s == 'a' || *index_s == 'A') {
    channel_info.value = imagergba_p->alpha;
  }
  else {
    lua_error("imagergba_tag \"gettable\": index is an invalid channel name!");
  }

  lua_pushusertag((void *) &channel_info, channel_tag);
}

/***************************************************************************\
* bitmap "gettable" fallback. This fallback is called when a LUA line    *
* like "c = bitmap.r[y*w + x]" or "bitmap.r[y*w + x] = c" is executed.*
* The channel_info global is filled and its address is returned with a      *
* channel_tag usertag lua_Object. The following "gettable" or "settable"    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static void bitmapgettable_fb(void)
{
  lua_Object image, index;
  
  char *index_s;
  bitmap_t *image_p;

  image = lua_getparam(1);
  index = lua_getparam(2);

  image_p = (bitmap_t *) lua_getuserdata(image);
  if (!image_p)
    lua_error("bitmap_tag \"gettable\": invalid bitmap_tag object!");

  if (!lua_isstring(index)) {
    lua_error("bitmap_tag \"gettable\": index should be a channel name!");
  }
  index_s = (char *) lua_getstring(index);

  channel_info.size = image_p->image->w * image_p->image->h;
  
  if (*index_s == 'r' || *index_s == 'R') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_IRED);
  }
  else if (*index_s == 'g' || *index_s == 'G') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_IGREEN);
  }
  else if (*index_s == 'b' || *index_s == 'B') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_IBLUE);
  }
  else if (*index_s == 'a' || *index_s == 'A') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_IALPHA);
  }
  else if (*index_s == 'i' || *index_s == 'I') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_INDEX);
  }
  else if (*index_s == 'c' || *index_s == 'C') {
    channel_info.value = cdBitmapGetData(image_p->image, CD_COLORS);
    channel_info.size = -1;
  }
  else {
    lua_error("imagergba_tag \"gettable\": index is an invalid channel name!");
  }

  lua_pushusertag((void *) &channel_info, channel_tag);
}

static void stategc_fb(void)
{
  lua_Object state;

  state_t *state_p;

  state = lua_getparam(1);
  state_p = (state_t *) lua_getuserdata(state);
  if (!state_p)
    lua_error("state_tag \"gc\": invalid state_tag object!");

  if (state_p->state) cdReleaseState(state_p->state);

  /* free the state_t structure */
  free(state_p);
}

/***************************************************************************\
* stipple "gc" fallback.                                                    *
\***************************************************************************/
static void stipplegc_fb(void)
{
  lua_Object stipple;

  stipple_t *stipple_p;

  stipple = lua_getparam(1);
  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p)
    lua_error("stipple_tag \"gc\": invalid stipple_tag object!");

  /* if the stipple has not been killed, kill it */
  if (stipple_p->value) free(stipple_p->value);

  /* free the stipple_t structure */
  free(stipple_p);
}

/***************************************************************************\
* pattern "gc" fallback.                                                    *
\***************************************************************************/
static void patterngc_fb(void)
{
  lua_Object pattern;

  pattern_t *pattern_p;

  pattern = lua_getparam(1);
  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p)
    lua_error("pattern_tag \"gc\": invalid pattern_tag object!");

  /* if the pattern has not been killed, kill it */
  if (pattern_p->color) free(pattern_p->color);

  /* free the pattern_t structure */
  free(pattern_p);
}

/***************************************************************************\
* palette "gc" fallback.                                                    *
\***************************************************************************/
static void palettegc_fb(void)
{
  lua_Object palette;

  palette_t *palette_p;

  palette = lua_getparam(1);
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p)
    lua_error("palette_tag \"gc\": invalid palette_tag object!");

  /* if the palette has not been killed, kill it */
  if (palette_p->color) free(palette_p->color);

  /* free the palette_t structure */
  free(palette_p);
}

/***************************************************************************\
* imagergb "gc" fallback.                                                   *
\***************************************************************************/
static void imagergbgc_fb(void)
{
  lua_Object imagergb;

  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!imagergb_p)
    lua_error("imagergb_tag \"gc\": invalid imagergb_tag object!");

  /* if the imagergb has not been killed, kill it */
  if (imagergb_p->red) free(imagergb_p->red);
  if (imagergb_p->green) free(imagergb_p->green);
  if (imagergb_p->blue) free(imagergb_p->blue);

  /* free the imagergb_t structure */
  free(imagergb_p);
}

/***************************************************************************\
* imagergba "gc" fallback.                                                   *
\***************************************************************************/
static void imagergbagc_fb(void)
{
  lua_Object imagergba;

  imagergba_t *imagergba_p;

  imagergba = lua_getparam(1);
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!imagergba_p)
    lua_error("imagergba_tag \"gc\": invalid imagergba_tag object!");

  /* if the imagergba has not been killed, kill it */
  if (imagergba_p->red) free(imagergba_p->red);
  if (imagergba_p->green) free(imagergba_p->green);
  if (imagergba_p->blue) free(imagergba_p->blue);
  if (imagergba_p->alpha) free(imagergba_p->alpha);

  /* free the imagergba_t structure */
  free(imagergba_p);
}

/***************************************************************************\
* imagemap "gc" fallback.                                                   *
\***************************************************************************/
static void imagemapgc_fb(void)
{
  lua_Object imagemap;

  imagemap_t *imagemap_p;

  imagemap = lua_getparam(1);
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p)
    lua_error("imagemap_tag \"gc\": invalid imagemap_tag object!");

  /* if the imagemap has not been killed, kill it */
  if (imagemap_p->index) free(imagemap_p->index);

  /* free the imagemap_t structure */
  free(imagemap_p);
}

/***************************************************************************\
* bitmap "gc" fallback.                                                   *
\***************************************************************************/
static void bitmapgc_fb(void)
{
  lua_Object bitmap;

  bitmap_t *bitmap_p;

  bitmap = lua_getparam(1);
  bitmap_p = (bitmap_t *) lua_getuserdata(bitmap);
  if (!bitmap_p)
    lua_error("bitmap_tag \"gc\": invalid bitmap_tag object!");

  /* if the bitmap has not been killed, kill it */
  if (bitmap_p->image) cdKillBitmap(bitmap_p->image);

  /* free the bitmap_t structure */
  free(bitmap_p);
}

/***************************************************************************\
* cdPixel.                                                                  *
\***************************************************************************/
static void cdlua_pixel (void)
{
  lua_Object x;
  lua_Object y;
  lua_Object color;

  int x_i;
  int y_i;
  long int color_i;
  
  x = lua_getparam(1);
  y = lua_getparam(2);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("cdPixel: pixel coordinates should be integers!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);

  color = lua_getparam(3);
  if (lua_tag(color) != color_tag)
    lua_error("cdPixel: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdPixel: too many parameters!");

  cdPixel(x_i, y_i, color_i);
}

static void wdlua_pixel (void)
{
  lua_Object x;
  lua_Object y;
  lua_Object color;

  double x_i;
  double y_i;
  long int color_i;
  
  x = lua_getparam(1);
  y = lua_getparam(2);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("wdPixel: pixel coordinates should be numbers!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);

  color = lua_getparam(3);
  if (lua_tag(color) != color_tag)
    lua_error("cdPixel: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdPixel: too many parameters!");

  wdPixel(x_i, y_i, color_i);
}

/***************************************************************************\
* cdGetCanvasSize.                                                          *
\***************************************************************************/
static void cdlua_getcanvassize(void)
{
  int width;
  int height;
  double mm_width;
  double mm_height;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdGetCanvasSize: too many parameters!");

  cdGetCanvasSize(&width, &height, &mm_width, &mm_height);
  lua_pushnumber(width);
  lua_pushnumber(height);
  lua_pushnumber(mm_width);
  lua_pushnumber(mm_height);
}

/***************************************************************************\
* Register callback functions.                                              *
\***************************************************************************/
static void cdlua_registercallback(void)
{
  lua_Object driver, cb, func;
  int driver_i, cb_i, func_lock;
  cdContextLUA* luactx;
  cdCallbackLUA* cdCB;
  
  driver = lua_getparam(1);
  if (!lua_isnumber(driver))
    lua_error("cdRegisterCallback: invalid driver parameter!");
  driver_i = (int) lua_getnumber(driver);

  cb = lua_getparam(2);
  if (!lua_isnumber(cb))
    lua_error("cdRegisterCallback: invalid cb parameter!");
  cb_i = (int) lua_getnumber(cb);

  func = lua_getparam(3);
  if (lua_isnil(func))
    func_lock = -1;
  else if (!lua_isfunction(func))
  {
    lua_error("cdRegisterCallback: invalid func parameter!");
    return;
  }
  else {
    lua_pushobject(func);
    func_lock = lua_ref(1);
  }

  if (driver_i >= cdlua_numdrivers)
    lua_error("cdRegisterCallback: invalid driver parameter!");

  luactx = cdlua_drivers[driver_i];

  if (cb_i >= luactx->cb_n)
    lua_error("cdRegisterCallback: invalid cb parameter!");

  cdCB = &luactx->cb_list[cb_i];

  if (cdCB->lock != -1) {
    lua_unref(cdCB->lock);
    cdCB->lock = func_lock;
    if (func_lock == -1) {
      cdRegisterCallback(luactx->ctx(), cb_i, NULL);
    }
  }
  else {
    if (func_lock != -1) {
      cdRegisterCallback(luactx->ctx(), cb_i, (cdCallback)cdCB->func);
      cdCB->lock = func_lock;
    }
  }
}

/***************************************************************************\
* cdPlay.                                                                   *
\***************************************************************************/
static void cdlua_play(void)
{
  lua_Object driver;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;
  lua_Object data;

  int driver_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;
  char *data_s;

  driver = lua_getparam(1);
  if (!lua_isnumber(driver))
    lua_error("cdPlay: invalid driver parameter!");
  driver_i = (long int) lua_getnumber(driver);

  xmin = lua_getparam(2);
  xmax = lua_getparam(3);
  ymin = lua_getparam(4);
  ymax = lua_getparam(5);
  if (!(lua_isnumber(xmin) &&  lua_isnumber(xmax) &&  
    lua_isnumber(ymin) &&  lua_isnumber(ymin)))
    lua_error("cdPlay: invalid viewport!");
  xmin_i = (long int) lua_getnumber(xmin);
  xmax_i = (long int) lua_getnumber(xmax);
  ymin_i = (long int) lua_getnumber(ymin);
  ymax_i = (long int) lua_getnumber(ymax);

  data = lua_getparam(6);
  if (!lua_isstring(data))
    lua_error("cdPlay: data should be of type string!");
  data_s = lua_getstring(data);

  if (driver_i >= cdlua_numdrivers)
    lua_error("cdPlay: unknown driver!");

  if (lua_getparam(7) != LUA_NOOBJECT)
    lua_error("cdPlay: too many parameters!");

  cdPlay(cdlua_drivers[driver_i]->ctx(), xmin_i, xmax_i, ymin_i, ymax_i, data_s);
}

/***************************************************************************\
* cdUpdateYAxis.                                                          *
\***************************************************************************/

static void cdlua_updateyaxis(void)
{
  lua_Object y;

  int y_i;

  y = lua_getparam(1);
  if (!lua_isnumber(y))
    lua_error("cdUpdateYAxis: invalid (y) parameter!");
  y_i = (int) lua_getnumber(y);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdUpdateYAxis: too many parameters!");

  cdUpdateYAxis(&y_i);
  lua_pushnumber(y_i);
}

/***************************************************************************\
* cdGetClipArea.                                                            *
\***************************************************************************/
static void cdlua_getcliparea(void)
{
  int xmin;
  int xmax;
  int ymin;
  int ymax;
  int status;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdGetClipArea: too many parameters!");
 
  status = cdGetClipArea(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
  lua_pushnumber(status);
}

static void cdlua_RegionBox(void)
{
  int xmin;
  int xmax;
  int ymin;
  int ymax;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdRegionBox: too many parameters!");
 
  cdRegionBox(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

static void cdlua_getclippoly(void)
{
  int n, i;
  int *pts;
  lua_Object points;

  pts = cdGetClipPoly(&n);
  lua_pushnumber(n);

  points = lua_createtable();

  for (i=0; i < 2*n; i++)
  {
    lua_pushobject(points);
    lua_pushnumber(i+1);
    lua_pushnumber(pts[i]);
    lua_settable();
  }
}

static void wdlua_getclippoly(void)
{
  int n, i;
  double *pts;
  lua_Object points;

  pts = wdGetClipPoly(&n);
  lua_pushnumber(n);

  points = lua_createtable();

  for (i=0; i < 2*n; i++)
  {
    lua_pushobject(points);
    lua_pushnumber(i+1);
    lua_pushnumber(pts[i]);
    lua_settable();
  }
}

/***************************************************************************\
* cdMM2Pixel.                                                               *
\***************************************************************************/
static void cdlua_mm2pixel(void)
{
  lua_Object mm_dx;
  lua_Object mm_dy;

  double mm_dx_d;
  double mm_dy_d;
  int dx;
  int dy;

  mm_dx = lua_getparam(1);
  mm_dy = lua_getparam(2);
  if (!(lua_isnumber(mm_dx) && lua_isnumber(mm_dy)))
    lua_error("cdMM2Pixel: invalid (mm_dx, mm_dy) parameter!");
  mm_dx_d = (double) lua_getnumber(mm_dx);
  mm_dy_d = (double) lua_getnumber(mm_dy);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdMM2Pixel: too many parameters!");

  cdMM2Pixel(mm_dx, mm_dy, &dx, &dy);
  lua_pushnumber(dx);
  lua_pushnumber(dy);
}

/***************************************************************************\
* cdPixel2MM.                                                               *
\***************************************************************************/
static void cdlua_pixel2mm(void)
{
  lua_Object dx;
  lua_Object dy;
  int dx_i;
  int dy_i;

  double mm_dx;
  double mm_dy;

  dx = lua_getparam(1);
  dy = lua_getparam(2);
  if (!(lua_isnumber(dx) && lua_isnumber(dy)))
    lua_error("cdPixel2MM: invalid (dx, dy) parameter!");
  dx_i = (int) lua_getnumber(dx);
  dy_i = (int) lua_getnumber(dy);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdPixel2MM: too many parameters!");

  cdPixel2MM(dx_i, dy_i, &mm_dx, &mm_dy);
  lua_pushnumber(mm_dx);
  lua_pushnumber(mm_dy);
}

/***************************************************************************\
* cdStipple.                                                                *
\***************************************************************************/
static void cdlua_stipple(void)
{
  lua_Object stipple;
  stipple_t *stipple_p;

  stipple = lua_getparam(1);
  if (lua_isnil(stipple))
    lua_error("cdStipple: attempt to set a NIL stipple!");
  if (lua_tag(stipple) != stipple_tag)
    lua_error("cdStipple: invalid stipple parameter!");
  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p->value)
    lua_error("cdStipple: attempt to set a killed stipple!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdStipple: too many parameters!");

  cdStipple(stipple_p->width, stipple_p->height, stipple_p->value);
}

static void wdlua_stipple(void)
{
  lua_Object stipple;
  stipple_t *stipple_p;
  double w_mm;
  double h_mm;

  stipple = lua_getparam(1);
  if (lua_isnil(stipple))
    lua_error("wdStipple: attempt to set a NIL stipple!");
  if (lua_tag(stipple) != stipple_tag)
    lua_error("wdStipple: invalid stipple parameter!");
  stipple_p = (stipple_t *) lua_getuserdata(stipple);
  if (!stipple_p->value)
    lua_error("wdStipple: attempt to set a killed stipple!");

  w_mm = (double)luaL_check_number(2);
  h_mm = (double)luaL_check_number(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdStipple: too many parameters!");

  wdStipple(stipple_p->width, stipple_p->height, stipple_p->value, w_mm, h_mm);
}

/***************************************************************************\
* cdPattern.                                                                *
\***************************************************************************/
static void cdlua_pattern(void)
{
  lua_Object pattern;
  pattern_t *pattern_p;

  pattern = lua_getparam(1);
  if (lua_isnil(pattern))
    lua_error("cdPattern: attempt to set a NIL pattern!");
  if (lua_tag(pattern) != pattern_tag)
    lua_error("cdPattern: invalid pattern parameter!");
  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p->color)
    lua_error("cdPattern: attempt to set a killed pattern!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdPattern: too many parameters!");

  cdPattern(pattern_p->width, pattern_p->height, pattern_p->color);
}

static void wdlua_pattern(void)
{
  lua_Object pattern;
  pattern_t *pattern_p;
  double w_mm;
  double h_mm;

  pattern = lua_getparam(1);
  if (lua_isnil(pattern))
    lua_error("wdPattern: attempt to set a NIL pattern!");
  if (lua_tag(pattern) != pattern_tag)
    lua_error("wdPattern: invalid pattern parameter!");
  pattern_p = (pattern_t *) lua_getuserdata(pattern);
  if (!pattern_p->color)
    lua_error("wdPattern: attempt to set a killed pattern!");

  w_mm = (double)luaL_check_number(2);
  h_mm = (double)luaL_check_number(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdPattern: too many parameters!");

  wdPattern(pattern_p->width, pattern_p->height, pattern_p->color, w_mm, h_mm);
}

/***************************************************************************\
* cdFontDim.                                                                *
\***************************************************************************/
static void cdlua_fontdim(void)
{
  int max_width;
  int height;
  int ascent;
  int descent;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdFontDim: too many parameters!");

  cdFontDim(&max_width, &height, &ascent, &descent);
  lua_pushnumber(max_width);
  lua_pushnumber(height);
  lua_pushnumber(ascent);
  lua_pushnumber(descent);
}

/***************************************************************************\
* cdTextSize.                                                               *
\***************************************************************************/
static void cdlua_textsize(void)
{
  lua_Object text;
  char* text_s;

  int width;
  int height;

  text = lua_getparam(1);
  if (!lua_isstring(text))
    lua_error("cdTextSize: text should be of type string!");
  text_s = (char*) lua_getstring(text);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdTextSize: too many parameters!");

  cdTextSize(text_s, &width, &height);
  lua_pushnumber(width);
  lua_pushnumber(height);
}

static void cdlua_textbox(void)
{
  int xmin;
  int xmax;
  int ymin;
  int ymax;
  int x = (int)luaL_check_number(1);
  int y = (int)luaL_check_number(2);
  char* s = (char*)luaL_check_string(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdTextBox: too many parameters!");
 
  cdTextBox(x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

static void wdlua_textbox(void)
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double x = (double)luaL_check_number(1);
  double y = (double)luaL_check_number(2);
  char* s = (char*)luaL_check_string(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdTextBox: too many parameters!");
 
  wdTextBox(x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

static void cdlua_textbounds(void)
{
  int rect[8];
  int x = (int)luaL_check_number(1);
  int y = (int)luaL_check_number(2);
  char* s = (char*)luaL_check_string(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdTextBox: too many parameters!");
 
  cdTextBounds(x, y, s, rect);
  lua_pushnumber(rect[0]);
  lua_pushnumber(rect[1]);
  lua_pushnumber(rect[2]);
  lua_pushnumber(rect[3]);
  lua_pushnumber(rect[4]);
  lua_pushnumber(rect[5]);
  lua_pushnumber(rect[6]);
  lua_pushnumber(rect[7]);
}

static void wdlua_textbounds(void)
{
  double rect[8];
  double x = (double)luaL_check_number(1);
  double y = (double)luaL_check_number(2);
  char* s = (char*)luaL_check_string(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdTextBox: too many parameters!");
 
  wdTextBounds(x, y, s, rect);
  lua_pushnumber(rect[0]);
  lua_pushnumber(rect[1]);
  lua_pushnumber(rect[2]);
  lua_pushnumber(rect[3]);
  lua_pushnumber(rect[4]);
  lua_pushnumber(rect[5]);
  lua_pushnumber(rect[6]);
  lua_pushnumber(rect[7]);
}

static void cdlua_getfont(void)
{
  int type_face, style, size;

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdGetFont: too many parameters!");

  cdGetFont(&type_face, &style, &size);
  lua_pushnumber(type_face);
  lua_pushnumber(style);
  lua_pushnumber(size);
}

static void wdlua_getfont(void)
{
  int type_face, style;
  double size;

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("wdGetFont: too many parameters!");

  wdGetFont(&type_face, &style, &size);
  lua_pushnumber(type_face);
  lua_pushnumber(style);
  lua_pushnumber(size);
}

/***************************************************************************\
* cdPalette.                                                                *
\***************************************************************************/
static void cdlua_palette(void)
{
  lua_Object palette;
  lua_Object mode;
  palette_t *palette_p;
  int mode_i;
  
  palette = lua_getparam(1);
  if (lua_isnil(palette))
    lua_error("cdPalette: attempt to set a NIL palette!");
  if (lua_tag(palette) != palette_tag)
    lua_error("cdPalette: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color)
    lua_error("cdPalette: attempt to set a killed palette!");

  mode = lua_getparam(2);
  if (!lua_isnumber(mode))
    lua_error("cdPalette: invalid mode parameter!");
  mode_i = (int) lua_getnumber(mode);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("cdPalette: too many parameters!");

  cdPalette(palette_p->size, palette_p->color, mode_i);
}

/***************************************************************************\
* cdBackground.                                                             *
\***************************************************************************/
static void cdlua_background(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdBackground: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);
  
  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdBackground: too many parameters!");

  color_i = cdBackground(color_i);
  lua_pushusertag((void*) color_i, color_tag);
}

/***************************************************************************\
* cdForeground.                                                             *
\***************************************************************************/
static void cdlua_foreground(void)
{
  lua_Object color;
  long int color_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("cdForeground: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);
  
  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdForeground: too many parameters!");

  color_i = cdForeground(color_i);
  lua_pushusertag((void*) color_i, color_tag);
}

/***************************************************************************\
* cdGetImageRGB.                                                            *
\***************************************************************************/
static void cdlua_getimagergb(void)
{
  lua_Object imagergb;
  lua_Object x;
  lua_Object y;

  imagergb_t *imagergb_p;
  int x_i;
  int y_i;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdGetImageRGB: attempt to get a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdGetImageRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
    lua_error("cdGetImageRGB: attempt to get a killed imagergb!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("cdGetImageRGB: invalid (x, y) parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdGetImageRGB: too many parameters!");

  cdGetImageRGB(imagergb_p->red, imagergb_p->green, imagergb_p->blue,
    x_i, y_i, imagergb_p->width, imagergb_p->height);
}
/***************************************************************************\
* cdPutImageRGB.                                                            *
\***************************************************************************/

static void cdlua_rgb2map(void)
{
  lua_Object imagemap;
  lua_Object palette;
  lua_Object imagergb;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdRGB2Map: attempt to put a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdRGB2Map: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
    lua_error("cdRGB2Map: attempt to put a killed imagergb!");
  
  imagemap = lua_getparam(2);
  if (lua_isnil(imagemap))
    lua_error("cdRGB2Map: attempt to put a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("cdRGB2Map: imagemap invalid parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index)
    lua_error("cdRGB2Map: attempt to put a killed imagemap!");

  palette = lua_getparam(3);
  if (lua_isnil(palette))
    lua_error("cdRGB2Map: NIL pallete!");
  if (lua_tag(palette) != palette_tag)
    lua_error("cdRGB2Map: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color)
    lua_error("cdRGB2Map: killed pallete!");
  
  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdRGB2Map: too many parameters!");

  cdRGB2Map(imagergb_p->width, imagergb_p->height, 
            imagergb_p->red, imagergb_p->green, imagergb_p->blue, 
            imagemap_p->index, palette_p->size, palette_p->color);
}

static void cdlua_putimagergb(void)
{
  lua_Object imagergb;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;

  imagergb_t *imagergb_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdPutImageRGB: attempt to put a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdPutImageRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
    lua_error("cdPutImageRGB: attempt to put a killed imagergb!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h)))
    lua_error("cdPutImageRGB: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageRGB: target region dimensions should be positive integers!");
  
  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("cdPutImageRGB: too many parameters!");

  cdPutImageRGB(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
    imagergb_p->green, imagergb_p->blue, x_i, y_i, w_i, h_i);
}

static void cdlua_putimagerectrgb(void)
{
  lua_Object imagergb;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagergb_t *imagergb_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("cdPutImageRectRGB: attempt to put a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("cdPutImageRectRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
    lua_error("cdPutImageRectRGB: attempt to put a killed imagergb!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  xmin = lua_getparam(6);
  xmax = lua_getparam(7);
  ymin = lua_getparam(8);
  ymax = lua_getparam(9);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("cdPutImageRectRGB: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageRectRGB: target region dimensions should be positive integers!");
  
  if (lua_getparam(10) != LUA_NOOBJECT)
    lua_error("cdPutImageRectRGB: too many parameters!");

  cdPutImageRectRGB(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
    imagergb_p->green, imagergb_p->blue, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

static void wdlua_putimagerectrgb(void)
{
  lua_Object imagergb;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagergb_t *imagergb_p;
  double x_i;
  double y_i;
  double w_i;
  double h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagergb = lua_getparam(1);
  if (lua_isnil(imagergb))
    lua_error("wdPutImageRectRGB: attempt to put a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("wdPutImageRectRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
    lua_error("wdPutImageRectRGB: attempt to put a killed imagergb!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  xmin = lua_getparam(6);
  xmax = lua_getparam(7);
  ymin = lua_getparam(8);
  ymax = lua_getparam(9);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("wdPutImageRectRGB: invalid parameter!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);
  w_i = (double) lua_getnumber(w);
  h_i = (double) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("wdPutImageRectRGB: target region dimensions should be positive integers!");
  
  if (lua_getparam(10) != LUA_NOOBJECT)
    lua_error("wdPutImageRectRGB: too many parameters!");

  wdPutImageRectRGB(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
    imagergb_p->green, imagergb_p->blue, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

/***************************************************************************\
* cdPutImageRGBA.                                                           *
\***************************************************************************/
static void cdlua_putimagergba(void)
{
  lua_Object imagergba;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;

  imagergba_t *imagergba_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;

  imagergba = lua_getparam(1);
  if (lua_isnil(imagergba))
    lua_error("cdPutImageRGBA: attempt to put a NIL imagergba!");
  if (lua_tag(imagergba) != imagergba_tag)
    lua_error("cdPutImageRGBA: invalid imagergba parameter!");
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue && imagergba_p->alpha))
    lua_error("cdPutImageRGBA: attempt to put a killed imagergba!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h)))
    lua_error("cdPutImageRGBA: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageRGBA: target region dimensions should be positive integers!");

  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("cdPutImageRGBA: too many parameters!");

  cdPutImageRGBA(imagergba_p->width, imagergba_p->height, imagergba_p->red, 
    imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x_i, y_i, w_i, h_i);
}

static void cdlua_putimagerectrgba(void)
{
  lua_Object imagergba;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagergba_t *imagergba_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagergba = lua_getparam(1);
  if (lua_isnil(imagergba))
    lua_error("cdPutImageRectRGBA: attempt to put a NIL imagergba!");
  if (lua_tag(imagergba) != imagergba_tag)
    lua_error("cdPutImageRectRGBA: invalid imagergba parameter!");
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue && imagergba_p->alpha))
    lua_error("cdPutImageRectRGBA: attempt to put a killed imagergba!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  xmin = lua_getparam(6);
  xmax = lua_getparam(7);
  ymin = lua_getparam(8);
  ymax = lua_getparam(9);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("cdPutImageRectRGBA: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageRectRGBA: target region dimensions should be positive integers!");

  if (lua_getparam(10) != LUA_NOOBJECT)
    lua_error("cdPutImageRectRGBA: too many parameters!");

  cdPutImageRectRGBA(imagergba_p->width, imagergba_p->height, imagergba_p->red, 
    imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

static void wdlua_putimagerectrgba(void)
{
  lua_Object imagergba;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagergba_t *imagergba_p;
  double x_i;
  double y_i;
  double w_i;
  double h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagergba = lua_getparam(1);
  if (lua_isnil(imagergba))
    lua_error("wdPutImageRectRGBA: attempt to put a NIL imagergba!");
  if (lua_tag(imagergba) != imagergba_tag)
    lua_error("wdPutImageRectRGBA: invalid imagergba parameter!");
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue && imagergba_p->alpha))
    lua_error("wdPutImageRectRGBA: attempt to put a killed imagergba!");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  w = lua_getparam(4);
  h = lua_getparam(5);
  xmin = lua_getparam(6);
  xmax = lua_getparam(7);
  ymin = lua_getparam(8);
  ymax = lua_getparam(9);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("wdPutImageRectRGBA: invalid parameter!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);
  w_i = (double) lua_getnumber(w);
  h_i = (double) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("wdPutImageRectRGBA: target region dimensions should be positive integers!");

  if (lua_getparam(10) != LUA_NOOBJECT)
    lua_error("wdPutImageRectRGBA: too many parameters!");

  wdPutImageRectRGBA(imagergba_p->width, imagergba_p->height, imagergba_p->red, 
    imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

/***************************************************************************\
* cdPutImageMap.                                                            *
\***************************************************************************/
static void cdlua_putimagemap(void)
{
  lua_Object imagemap;
  lua_Object palette;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;

  imagemap = lua_getparam(1);
  if (lua_isnil(imagemap))
    lua_error("cdPutImageMap: attempt to put a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("cdPutImageMap: imagemap invalid parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index)
    lua_error("cdPutImageMap: attempt to put a killed imagemap!");

  palette = lua_getparam(2);
  if (lua_isnil(palette))
    lua_error("cdPutImageMap: NIL pallete!");
  if (lua_tag(palette) != palette_tag)
    lua_error("cdPutImageMap: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color)
    lua_error("cdPutImageMap: killed pallete!");

  x = lua_getparam(3);
  y = lua_getparam(4);
  w = lua_getparam(5);
  h = lua_getparam(6);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h)))
    lua_error("cdPutImageMap: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageMap: target region dimensions should be positive integers!");
  
  if (lua_getparam(7) != LUA_NOOBJECT)
    lua_error("cdPutImageMap: too many parameters!");

  cdPutImageMap(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
    palette_p->color, x_i, y_i, w_i, h_i);
}

static void cdlua_putimagerectmap(void)
{
  lua_Object imagemap;
  lua_Object palette;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  int x_i;
  int y_i;
  int w_i;
  int h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagemap = lua_getparam(1);
  if (lua_isnil(imagemap))
    lua_error("cdPutImageMap: attempt to put a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("cdPutImageMap: imagemap invalid parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index)
    lua_error("cdPutImageMap: attempt to put a killed imagemap!");

  palette = lua_getparam(2);
  if (lua_isnil(palette))
    lua_error("cdPutImageRectMap: NIL pallete!");
  if (lua_tag(palette) != palette_tag)
    lua_error("cdPutImageRectMap: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color)
    lua_error("cdPutImageRectMap: killed pallete!");

  x = lua_getparam(3);
  y = lua_getparam(4);
  w = lua_getparam(5);
  h = lua_getparam(6);
  xmin = lua_getparam(7);
  xmax = lua_getparam(8);
  ymin = lua_getparam(9);
  ymax = lua_getparam(10);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("cdPutImageRectMap: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  w_i = (int) lua_getnumber(w);
  h_i = (int) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("cdPutImageRectMap: target region dimensions should be positive integers!");
  
  if (lua_getparam(11) != LUA_NOOBJECT)
    lua_error("cdPutImageRectMap: too many parameters!");

  cdPutImageRectMap(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
    palette_p->color, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

static void wdlua_putimagerectmap(void)
{
  lua_Object imagemap;
  lua_Object palette;
  lua_Object x;
  lua_Object y;
  lua_Object w;
  lua_Object h;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  double x_i;
  double y_i;
  double w_i;
  double h_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  imagemap = lua_getparam(1);
  if (lua_isnil(imagemap))
    lua_error("wdPutImageMap: attempt to put a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("wdPutImageMap: imagemap invalid parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index)
    lua_error("wdPutImageMap: attempt to put a killed imagemap!");

  palette = lua_getparam(2);
  if (lua_isnil(palette))
    lua_error("wdPutImageRectMap: NIL pallete!");
  if (lua_tag(palette) != palette_tag)
    lua_error("wdPutImageRectMap: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color)
    lua_error("wdPutImageRectMap: killed pallete!");

  x = lua_getparam(3);
  y = lua_getparam(4);
  w = lua_getparam(5);
  h = lua_getparam(6);
  xmin = lua_getparam(7);
  xmax = lua_getparam(8);
  ymin = lua_getparam(9);
  ymax = lua_getparam(10);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(w) && lua_isnumber(h) &&
        lua_isnumber(xmin) && lua_isnumber(xmax) && lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("wdPutImageRectMap: invalid parameter!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);
  w_i = (double) lua_getnumber(w);
  h_i = (double) lua_getnumber(h);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);
  if (w_i < 0 || h_i < 0)
    lua_error("wdPutImageRectMap: target region dimensions should be positive integers!");
  
  if (lua_getparam(11) != LUA_NOOBJECT)
    lua_error("wdPutImageRectMap: too many parameters!");

  wdPutImageRectMap(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
    palette_p->color, x_i, y_i, w_i, h_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

/***************************************************************************\
* cdGetImage.                                                               *
\***************************************************************************/
static void cdlua_getimage(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;

  image_t *image_p;
  int x_i;
  int y_i;

  image = lua_getparam(1);
  if (lua_isnil(image))
    lua_error("cdGetImage: attempt to get NIL image");
  if (lua_tag(image) != image_tag)
    lua_error("cdGetImage: invalid image parameter!");
  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("cdGetImage: attempt to get a killed image");
  
  x = lua_getparam(2);
  y = lua_getparam(3);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("cdGetImage: invalid (x, y) parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdGetImage: too many parameters!");
  cdGetImage(image_p->cd_image, x_i, y_i);
}

/***************************************************************************\
* cdPutImage.                                                               *
\***************************************************************************/
static void cdlua_putimage(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;

  image_t *image_p;
  int x_i;
  int y_i;

  image = lua_getparam(1);
  if (lua_isnil(image))
    lua_error("cdPutImage: attempt to put a NIL image!");
  if (lua_tag(image) != image_tag)
    lua_error("cdPutImage: invalid image parameter!");
  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("cdPutImage: attempt to put a killed image!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  if (!(lua_isnumber(x) && lua_isnumber(y)))
    lua_error("cdPutImage: invalid (x, y) parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdPutImage: too many parameters!");

  cdPutImage(image_p->cd_image, x_i, y_i);
}

/***************************************************************************\
* cdPutImageRect.                                                           *
\***************************************************************************/
static void cdlua_putimagerect(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  image_t *image_p;
  int x_i;
  int y_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  image = lua_getparam(1);
  if (lua_isnil(image))
    lua_error("cdPutImageRect: attempt to put a NIL image!");
  if (lua_tag(image) != image_tag)
    lua_error("cdPutImageRect: invalid image parameter!");
  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("cdPutImageRect: attempt to put a killed image!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  xmin = lua_getparam(4);
  xmax = lua_getparam(5);
  ymin = lua_getparam(6);
  ymax = lua_getparam(7);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(xmin) && lua_isnumber(xmax) && 
    lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("cdPutImageRect: invalid parameter!");
  x_i = (int) lua_getnumber(x);
  y_i = (int) lua_getnumber(y);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);

  if (lua_getparam(8) != LUA_NOOBJECT)
    lua_error("cdPutImageRect: too many parameters!");

  cdPutImageRect(image_p->cd_image, x_i, y_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

static void wdlua_putimagerect(void)
{
  lua_Object image;
  lua_Object x;
  lua_Object y;
  lua_Object xmin;
  lua_Object xmax;
  lua_Object ymin;
  lua_Object ymax;

  image_t *image_p;
  double x_i;
  double y_i;
  int xmin_i;
  int xmax_i;
  int ymin_i;
  int ymax_i;

  image = lua_getparam(1);
  if (lua_isnil(image))
    lua_error("wdPutImageRect: attempt to put a NIL image!");
  if (lua_tag(image) != image_tag)
    lua_error("wdPutImageRect: invalid image parameter!");
  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("wdPutImageRect: attempt to put a killed image!");

  x = lua_getparam(2);
  y = lua_getparam(3);
  xmin = lua_getparam(4);
  xmax = lua_getparam(5);
  ymin = lua_getparam(6);
  ymax = lua_getparam(7);
  if (!(lua_isnumber(x) && lua_isnumber(y) && lua_isnumber(xmin) && lua_isnumber(xmax) && 
    lua_isnumber(ymin) && lua_isnumber(ymax)))
    lua_error("wdPutImageRect: invalid parameter!");
  x_i = (double) lua_getnumber(x);
  y_i = (double) lua_getnumber(y);
  xmin_i = (int) lua_getnumber(xmin);
  xmax_i = (int) lua_getnumber(xmax);
  ymin_i = (int) lua_getnumber(ymin);
  ymax_i = (int) lua_getnumber(ymax);

  if (lua_getparam(8) != LUA_NOOBJECT)
    lua_error("wdPutImageRect: too many parameters!");

  wdPutImageRect(image_p->cd_image, x_i, y_i, xmin_i, xmax_i, ymin_i, ymax_i);
}

/***************************************************************************\
* cdVersion.                                                                *
\***************************************************************************/

static void cdlua_version(void)
{
  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdVersion: too many parameters!");

  lua_pushstring(cdVersion());
}

/***************************************************************************\
* wdGetViewport.                                                            *
\***************************************************************************/
static void wdlua_getviewport(void)
{
  int xmin;
  int xmax;
  int ymin;
  int ymax;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("wdGetViewport: too many parameters!");
 
  wdGetViewport(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

/***************************************************************************\
* wdGetWindow.                                                              *
\***************************************************************************/
static void wdlua_getwindow(void)
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("wdGetWindow: too many parameters!");
 
  wdGetWindow(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

/***************************************************************************\
* wdGetClipArea.                                                            *
\***************************************************************************/
static void wdlua_getcliparea(void)
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  int status;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("wdGetClipArea: too many parameters!");
 
  status = wdGetClipArea(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
  lua_pushnumber(status);
}

static void wdlua_RegionBox(void)
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("wdRegionBox: too many parameters!");
 
  wdRegionBox(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(xmin);
  lua_pushnumber(xmax);
  lua_pushnumber(ymin);
  lua_pushnumber(ymax);
}

/***************************************************************************\
* wdFontDim.                                                                *
\***************************************************************************/
static void wdlua_fontdim(void)
{
  double max_width;
  double height;
  double ascent;
  double descent;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("wdFontDim: too many parameters!");

  wdFontDim(&max_width, &height, &ascent, &descent);
  lua_pushnumber(max_width);
  lua_pushnumber(height);
  lua_pushnumber(ascent);
  lua_pushnumber(descent);
}

/***************************************************************************\
* wdTextSize.                                                               *
\***************************************************************************/
static void wdlua_textsize(void)
{
  lua_Object text;
  char* text_s;

  double width;
  double height;

  text = lua_getparam(1);
  if (!lua_isstring(text))
    lua_error("wdTextSize: invalid text parameter!");
  text_s = (char*) lua_getstring(text);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("wdTextSize: too many parameters!");

  wdTextSize(text_s, &width, &height);
  lua_pushnumber(width);
  lua_pushnumber(height);
}

/***************************************************************************\
* wdGetVectorTextSize.                                                      *
\***************************************************************************/
static void wdlua_getvectortextsize(void)
{
  lua_Object text;
  char* text_s;

  double width;
  double height;

  text = lua_getparam(1);
  if (!lua_isstring(text))
    lua_error("wdGetVectorTextSize: invalid text parameter!");
  text_s = (char*) lua_getstring(text);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("wdGetVectorTextSize: too many parameters!");

  wdGetVectorTextSize(text_s, &width, &height);
  lua_pushnumber(width);
  lua_pushnumber(height);
}

static void cdlua_vectortexttransform(void)
{
  lua_Object old_table, table, value;
  double matrix[6], *old_matrix;
  int i;

  table = lua_getparam(1);
  if (!lua_istable(table))
    lua_error("cdVectorTextTransform: invalid table parameter!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdVectorTextTransform: too many parameters!");

  for (i=0; i < 6; i++)
  {
    lua_pushobject(table);
    lua_pushnumber(i+1);
    value = lua_gettable();

    if (!lua_isnumber(value))
      lua_error("cdVectorTextTransform: invalid value!");

    matrix[i] = lua_getnumber(value);
  }

  old_matrix = cdVectorTextTransform(matrix);

  old_table = lua_createtable();

  for (i=0; i < 6; i++)
  {
    lua_pushobject(old_table);
    lua_pushnumber(i+1);
    lua_pushnumber(old_matrix[i]);
    lua_settable();
  }
}

static void cdlua_vectortextbounds(void)
{
  char* s = (char*)luaL_check_string(1);
  int x = (int)luaL_check_number(2);
  int y = (int)luaL_check_number(3);
  int rect[8], i;
  lua_Object lua_rect;

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("cdGetVectorTextBounds: too many parameters!");

  cdGetVectorTextBounds(s, x, y, rect);

  lua_rect = lua_createtable();

  for (i=0; i < 8; i++)
  {
    lua_pushobject(lua_rect);
    lua_pushnumber(i+1);
    lua_pushnumber(rect[i]);
    lua_settable();
  }
}

static void wdlua_vectortextbounds(void)
{
  char* s = (char*)luaL_check_string(1);
  double x = luaL_check_number(2);
  double y = luaL_check_number(3);
  double rect[8];
  int i;
  lua_Object lua_rect;

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("wdGetVectorTextBounds: too many parameters!");

  wdGetVectorTextBounds(s, x, y, rect);

  lua_rect = lua_createtable();

  for (i=0; i < 8; i++)
  {
    lua_pushobject(lua_rect);
    lua_pushnumber(i+1);
    lua_pushnumber(rect[i]);
    lua_settable();
  }
}

static void cdlua_getvectortextsize(void)
{
  lua_Object text;
  char* text_s;

  int width;
  int height;

  text = lua_getparam(1);
  if (!lua_isstring(text))
    lua_error("cdGetVectorTextSize: invalid text parameter!");
  text_s = (char*) lua_getstring(text);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("cdGetTextSize: too many parameters!");

  cdGetVectorTextSize(text_s, &width, &height);
  lua_pushnumber(width);
  lua_pushnumber(height);
}
/***************************************************************************\
* wdWorld2Canvas.                                                           *
\***************************************************************************/
static void wdlua_world2canvas(void)
{
  lua_Object xw;
  lua_Object yw;
  double xw_d;
  double yw_d;

  int xv_i;
  int yv_i;

  xw = lua_getparam(1);
  yw = lua_getparam(2);
  if (!(lua_isnumber(xw) && lua_isnumber(yw)))
    lua_error("wdWorld2Canvas: invalid (xw, yw) parameter!");
  xw_d = (double) lua_getnumber(xw);
  yw_d = (double) lua_getnumber(yw);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("wdWorld2Canvas: too many parameters!");

  wdWorld2Canvas(xw_d, yw_d, &xv_i, &yv_i);
  lua_pushnumber(xv_i);
  lua_pushnumber(yv_i);
}

/***************************************************************************\
* wdCanvas2World.                                                           *
\***************************************************************************/
static void wdlua_canvas2world(void)
{
  lua_Object xv;
  lua_Object yv;
  int xv_i;
  int yv_i;

  double xw_d;
  double yw_d;

  xv = lua_getparam(1);
  yv = lua_getparam(2);
  if (!(lua_isnumber(xv) && lua_isnumber(yv)))
    lua_error("wdCanvas2World: invalid (xc, yc) parameter!");
  xv_i = (int) lua_getnumber(xv);
  yv_i = (int) lua_getnumber(yv);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("wdCanvas2World: too many parameters!");

  wdCanvas2World(xv_i, yv_i, &xw_d, &yw_d);
  lua_pushnumber(xw_d);
  lua_pushnumber(yw_d);
}

/***************************************************************************\
* Initializes CDLua.                                                        *
\***************************************************************************/
static void cdlua_pushstring(char* str, char* name)
{
  lua_pushstring(str); lua_setglobal(name);
  cdlua_setnamespace(name, name+2);     /* CD_XXXX = _XXXX */
}

static void setinfo(void) 
{
  cdlua_pushstring(CD_COPYRIGHT, "CD_COPYRIGHT");
  cdlua_pushstring("A 2D Graphics Library", "CD_DESCRIPTION");
  cdlua_pushstring("CD - Canvas Draw", "CD_NAME");
  cdlua_pushstring("CD "CD_VERSION, "CD_VERSION");
}

void cdlua_open(void)
{
  char version[50];
  lua_Object imlua_tag;

  cdlua_namespace = lua_createtable();
  lua_pushobject(cdlua_namespace); lua_setglobal ("cd");
  
  sprintf(version, "CDLua %s", cdVersion());
  lua_pushstring(version); lua_setglobal ("CDLUA_VERSION");
  setinfo();

  /* check if IM has been initialized */
  imlua_tag = lua_getglobal("IMLUA_INSTALLED");

  /* get IM defined tags, let IM handle with the user tag objects */
  if ((imlua_tag != LUA_NOOBJECT) && (!lua_isnil(imlua_tag))) 
  {
    imlua_tag = lua_getglobal("IMLUA_COLOR_TAG");
    color_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_IMAGERGB_TAG");
    imagergb_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_IMAGERGBA_TAG");
    imagergba_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_PALETTE_TAG");
    palette_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_IMAGEMAP_TAG");
    imagemap_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_CHANNEL_TAG");
    channel_tag = (int) lua_getnumber(imlua_tag);
    imlua_tag = lua_getglobal("IMLUA_BITMAP_TAG");
    bitmap_tag = (int) lua_getnumber(imlua_tag);
  }
  else /* define CD own tags and fallbacks  */
  {
    /* create user tags */
    color_tag     = lua_newtag();
    bitmap_tag    = lua_newtag();
    imagergb_tag  = lua_newtag();
    imagergba_tag = lua_newtag();
    imagemap_tag  = lua_newtag();
    palette_tag   = lua_newtag();
    channel_tag   = lua_newtag();

    /* hook "settable" and "gettable" tag methods */
    lua_pushcfunction(palettesettable_fb); lua_settagmethod(palette_tag, "settable");
    lua_pushcfunction(imagemapsettable_fb); lua_settagmethod(imagemap_tag, "settable");
    lua_pushcfunction(channelsettable_fb); lua_settagmethod(channel_tag, "settable");
  
    lua_pushcfunction(palettegettable_fb); lua_settagmethod(palette_tag, "gettable");
    lua_pushcfunction(imagemapgettable_fb); lua_settagmethod(imagemap_tag, "gettable");
    lua_pushcfunction(channelgettable_fb); lua_settagmethod(channel_tag, "gettable");
    lua_pushcfunction(imagergbgettable_fb); lua_settagmethod(imagergb_tag, "gettable");
    lua_pushcfunction(imagergbagettable_fb); lua_settagmethod(imagergba_tag, "gettable");
    lua_pushcfunction(bitmapgettable_fb); lua_settagmethod(bitmap_tag, "gettable");

    lua_pushcfunction(palettegc_fb); lua_settagmethod(palette_tag, "gc");
    lua_pushcfunction(imagemapgc_fb); lua_settagmethod(imagemap_tag, "gc");
    lua_pushcfunction(imagergbgc_fb); lua_settagmethod(imagergb_tag, "gc");
    lua_pushcfunction(imagergbagc_fb); lua_settagmethod(imagergba_tag, "gc");
    lua_pushcfunction(bitmapgc_fb); lua_settagmethod(bitmap_tag, "gc");
  }

  /* these are not handled by IM */
  stipple_tag = lua_newtag();
  pattern_tag = lua_newtag();
  image_tag   = lua_newtag();
  canvas_tag  = lua_newtag();
  state_tag   = lua_newtag();

  /* hook "settable" and "gettable" tag methods */
  lua_pushcfunction(stipplesettable_fb); lua_settagmethod(stipple_tag, "settable");
  lua_pushcfunction(patternsettable_fb); lua_settagmethod(pattern_tag, "settable");
  lua_pushcfunction(stipplegettable_fb); lua_settagmethod(stipple_tag, "gettable");
  lua_pushcfunction(patterngettable_fb); lua_settagmethod(pattern_tag, "gettable");

  lua_pushcfunction(stipplegc_fb); lua_settagmethod(stipple_tag, "gc");
  lua_pushcfunction(patterngc_fb); lua_settagmethod(pattern_tag, "gc");
  lua_pushcfunction(stategc_fb);   lua_settagmethod(state_tag, "gc");

  /* register used tags in global context for other libraries use */
  lua_pushnumber(1.0f); lua_setglobal("CDLUA_INSTALLED");
  
  lua_pushnumber(color_tag);     lua_setglobal(COLOR_TAG);
  lua_pushnumber(stipple_tag);   lua_setglobal(STIPPLE_TAG);
  lua_pushnumber(pattern_tag);   lua_setglobal(PATTERN_TAG);
  lua_pushnumber(image_tag);     lua_setglobal(IMAGE_TAG);
  lua_pushnumber(bitmap_tag);    lua_setglobal(BITMAP_TAG);
  lua_pushnumber(imagergb_tag);  lua_setglobal(IMAGERGB_TAG);
  lua_pushnumber(imagergba_tag); lua_setglobal(IMAGERGBA_TAG);
  lua_pushnumber(imagemap_tag);  lua_setglobal(IMAGEMAP_TAG);
  lua_pushnumber(palette_tag);   lua_setglobal(PALETTE_TAG);
  lua_pushnumber(channel_tag);   lua_setglobal(CHANNEL_TAG);
  lua_pushnumber(canvas_tag);    lua_setglobal(CANVAS_TAG);
  lua_pushnumber(state_tag);     lua_setglobal(STATE_TAG);

  /* registered cd functions */         
  cdlua_register("cdSaveState",           cdlua_savestate);
  cdlua_register("cdRestoreState",        cdlua_restorestate);
  cdlua_register("cdReleaseState",        cdlua_releasestate);
  cdlua_register("cdCreateCanvas",        cdlua_createcanvas);
  cdlua_register("cdContextCaps",         cdlua_contextcaps);
  cdlua_register("cdGetContext",          cdlua_getcontext);
  cdlua_register("cdActivate",            cdlua_activate);
  cdlua_register("cdActiveCanvas",        cdlua_activecanvas);
  cdlua_register("cdKillCanvas",          cdlua_killcanvas);
  cdlua_register("cdCreateStipple",       cdlua_createstipple);
  cdlua_register("cdGetStipple",          cdlua_getstipple);
  cdlua_register("cdKillStipple",         cdlua_killstipple);
  cdlua_register("cdCreatePattern",       cdlua_createpattern);
  cdlua_register("cdGetPattern",          cdlua_getpattern);
  cdlua_register("cdKillPattern",         cdlua_killpattern);
  cdlua_register("cdCreatePalette",       cdlua_createpalette);
  cdlua_register("cdKillPalette",         cdlua_killpalette);
  cdlua_register("cdCreateImage",         cdlua_createimage);
  cdlua_register("cdKillImage",           cdlua_killimage);
  cdlua_register("cdImageRGB",            cdlua_imagergb);
  cdlua_register("cdImageRGBBitmap",      cdlua_imagergbbitmap);
  cdlua_register("cdCreateImageRGB",      cdlua_createimagergb);
  cdlua_register("cdKillImageRGB",        cdlua_killimagergb);
  cdlua_register("cdCreateImageRGBA",     cdlua_createimagergba);
  cdlua_register("cdKillImageRGBA",       cdlua_killimagergba);
  cdlua_register("cdCreateImageMap",      cdlua_createimagemap);
  cdlua_register("cdKillImageMap",        cdlua_killimagemap);
                                        
  cdlua_register("cdRegisterCallback",    cdlua_registercallback);
  cdlua_register("cdEncodeColor",         cdlua_encodecolor);
  cdlua_register("cdDecodeColor",         cdlua_decodecolor);
  cdlua_register("cdEncodeAlpha",         cdlua_encodealpha);
  cdlua_register("cdDecodeAlpha",         cdlua_decodealpha);
  cdlua_register("cdReserved",            cdlua_reserved);
  cdlua_register("cdBlue",                cdlua_blue);
  cdlua_register("cdGreen",               cdlua_green);
  cdlua_register("cdRed",                 cdlua_red);
  cdlua_register("cdAlpha",               cdlua_alpha);
  cdlua_register("cdForeground",          cdlua_foreground);
  cdlua_register("cdBackground",          cdlua_background);
  cdlua_register("cdUpdateYAxis",         cdlua_updateyaxis);
  cdlua_register("cdFontDim",             cdlua_fontdim);
  cdlua_register("cdGetCanvasSize",       cdlua_getcanvassize);
  cdlua_register("cdGetClipPoly",         cdlua_getclippoly);
  cdlua_register("cdGetClipArea",         cdlua_getcliparea);
  cdlua_register("cdRegionBox",           cdlua_RegionBox);
  cdlua_register("cdGetImage",            cdlua_getimage);
  cdlua_register("cdGetImageRGB",         cdlua_getimagergb);
  cdlua_register("cdMM2Pixel",            cdlua_mm2pixel);
  cdlua_register("cdPalette",             cdlua_palette);
  cdlua_register("cdPattern",             cdlua_pattern);
  cdlua_register("cdPixel",               cdlua_pixel);
  cdlua_register("cdPixel2MM",            cdlua_pixel2mm);
  cdlua_register("cdPlay",                cdlua_play);
  cdlua_register("cdPutImage",            cdlua_putimage);
  cdlua_register("cdRGB2Map",             cdlua_rgb2map);
  cdlua_register("cdPutImageRGB",         cdlua_putimagergb);
  cdlua_register("cdPutImageRectRGB",     cdlua_putimagerectrgb);
  cdlua_register("cdPutImageRGBA",        cdlua_putimagergba);
  cdlua_register("cdPutImageRectRGBA",    cdlua_putimagerectrgba);
  cdlua_register("cdPutImageMap",         cdlua_putimagemap);
  cdlua_register("cdPutImageRectMap",     cdlua_putimagerectmap);
  cdlua_register("cdPutImageRect",        cdlua_putimagerect);
  cdlua_register("cdStipple",             cdlua_stipple);
  cdlua_register("cdTextSize",            cdlua_textsize);
  cdlua_register("cdTextBox",             cdlua_textbox);
  cdlua_register("cdTextBounds",          cdlua_textbounds);
  cdlua_register("cdGetFont",             cdlua_getfont);
  cdlua_register("cdVersion",             cdlua_version);
  cdlua_register("cdGetVectorTextSize",   cdlua_getvectortextsize);
  cdlua_register("cdVectorTextTransform", cdlua_vectortexttransform);
  cdlua_register("cdVectorTextBounds",    cdlua_vectortextbounds);
  cdlua_register("cdCreateBitmap",        cdlua_createbitmap);
  cdlua_register("cdKillBitmap",          cdlua_killbitmap);
  cdlua_register("cdBitmapSetRect",       cdlua_bitmapsetrect);
  cdlua_register("cdPutBitmap",           cdlua_putbitmap);
  cdlua_register("cdGetBitmap",           cdlua_getbitmap);
  cdlua_register("cdBitmapRGB2Map",       cdlua_rgb2mapex);
  cdlua_register("cdLineStyleDashes",     cdlua_LineStyleDashes);

  /* registered wd functions */         
  cdlua_register("wdHardcopy",            wdlua_hardcopy);
  cdlua_register("wdPutBitmap",           wdlua_putbitmap);
  cdlua_register("wdGetWindow",           wdlua_getwindow);
  cdlua_register("wdGetViewport",         wdlua_getviewport);
  cdlua_register("wdWorld2Canvas",        wdlua_world2canvas);
  cdlua_register("wdCanvas2World",        wdlua_canvas2world);
  cdlua_register("wdGetClipArea",         wdlua_getcliparea);
  cdlua_register("wdRegionBox",           wdlua_RegionBox);
  cdlua_register("wdMM2Pixel",            cdlua_mm2pixel);
  cdlua_register("wdPixel2MM",            cdlua_pixel2mm);
  cdlua_register("wdFontDim",             wdlua_fontdim);
  cdlua_register("wdGetFont",             wdlua_getfont);
  cdlua_register("wdTextSize",            wdlua_textsize);
  cdlua_register("wdGetVectorTextSize",   wdlua_getvectortextsize);
  cdlua_register("wdVectorTextBounds",    wdlua_vectortextbounds);
  cdlua_register("wdPixel",               wdlua_pixel);
  cdlua_register("wdTextBox",             wdlua_textbox);
  cdlua_register("wdTextBounds",          wdlua_textbounds);
  cdlua_register("wdPutImageRectRGB",     wdlua_putimagerectrgb);
  cdlua_register("wdPutImageRectRGBA",    wdlua_putimagerectrgba);
  cdlua_register("wdPutImageRectMap",     wdlua_putimagerectmap);
  cdlua_register("wdPutImageRect",        wdlua_putimagerect);
  cdlua_register("wdStipple",             wdlua_stipple);
  cdlua_register("wdPattern",             wdlua_pattern);
  cdlua_register("wdGetClipPoly",         wdlua_getclippoly);

  cdlua_initdrivers();

  /* color constants */
  cdlua_pushcolor(CD_RED, "CD_RED");
  cdlua_pushcolor(CD_DARK_RED, "CD_DARK_RED");
  cdlua_pushcolor(CD_GREEN, "CD_GREEN");
  cdlua_pushcolor(CD_DARK_GREEN, "CD_DARK_GREEN");
  cdlua_pushcolor(CD_BLUE, "CD_BLUE");
  cdlua_pushcolor(CD_DARK_BLUE, "CD_DARK_BLUE");
  cdlua_pushcolor(CD_YELLOW, "CD_YELLOW");
  cdlua_pushcolor(CD_DARK_YELLOW, "CD_DARK_YELLOW");
  cdlua_pushcolor(CD_MAGENTA, "CD_MAGENTA");
  cdlua_pushcolor(CD_DARK_MAGENTA, "CD_DARK_MAGENTA");
  cdlua_pushcolor(CD_CYAN, "CD_CYAN");
  cdlua_pushcolor(CD_DARK_CYAN, "CD_DARK_CYAN");
  cdlua_pushcolor(CD_WHITE, "CD_WHITE");
  cdlua_pushcolor(CD_BLACK, "CD_BLACK");
  cdlua_pushcolor(CD_DARK_GRAY, "CD_DARK_GRAY");
  cdlua_pushcolor(CD_GRAY, "CD_GRAY");

  /* cdplay constants */
  cdlua_pushnumber(CD_SIZECB, "CD_SIZECB");
  
  /* create void canvas used when there is no active canvas to avoid protection faults */
  void_canvas = cdCreateCanvas(CD_VOID, NULL);
  cdActivate(void_canvas);

  /* initialize toLua implementation */
  luaL_cd_open();
  luaL_wd_open();
}

void cdlua_close(void)
{
  cdKillCanvas(void_canvas);
}
