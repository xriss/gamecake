
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>


#include "lua.h"
#include "lauxlib.h"

#include "lua_raspi_unicornhat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ws2811.h"
#include "board_info.h"



#define TARGET_FREQ    WS2811_TARGET_FREQ
#define GPIO_PIN       18
#define DMA            5

#define WIDTH          8
#define HEIGHT         8
#define LED_COUNT      (WIDTH * HEIGHT)

static ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum    = GPIO_PIN,
            .count      = LED_COUNT,
            .invert     = 0,
            .brightness = 128,
        }
    }
};

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
//const char *lua_raspi_unicornhat_ptr_name="raspi_unicornhat*ptr";

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create the interface
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_create(lua_State *l)
{
    if(ws2811_init(&ledstring))
    {
		return 0;
	}
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy the interface
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_destroy(lua_State *l)
{
    ws2811_fini(&ledstring);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set brightness
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_brightness(lua_State *l)
{
	int b=0;

	b=lua_tonumber(l,1);

    ledstring.channel[0].brightness = b;
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set a single pixel
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_pixel(lua_State *l)
{
    int i=0;
	int r=0;
	int g=0;
	int b=0;

	i=lua_tonumber(l,1);

	if( lua_isnumber(l,2) ) { r=lua_tonumber(l,2); }
	if( lua_isnumber(l,3) ) { g=lua_tonumber(l,3); }
	if( lua_isnumber(l,4) ) { b=lua_tonumber(l,4); }

	if( (i>=0) && (i<LED_COUNT) ) // sanity
	{
		ledstring.channel[0].leds[i] = (r << 16) | (g << 8) | b;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set all pixels
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_clear(lua_State *l)
{
    int i=0;
	int r=0;
	int g=0;
	int b=0;
	
	if( lua_isnumber(l,1) ) { r=lua_tonumber(l,1); }
	if( lua_isnumber(l,2) ) { g=lua_tonumber(l,2); }
	if( lua_isnumber(l,3) ) { b=lua_tonumber(l,3); }
	
    for(i=0; i<LED_COUNT;i++){
		ledstring.channel[0].leds[i] = (r << 16) | (g << 8) | b;
    }
    
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// show the frame buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int lua_raspi_unicornhat_show(lua_State *l)
{
    ws2811_render(&ledstring);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_raspi_unicornhat(lua_State *l)
{
	const luaL_reg lib[] =
	{

		{"create",			lua_raspi_unicornhat_create},
		{"destroy",			lua_raspi_unicornhat_destroy},
		{"brightness",		lua_raspi_unicornhat_brightness},
		{"pixel",			lua_raspi_unicornhat_pixel},
		{"clear",			lua_raspi_unicornhat_clear},
		{"show",			lua_raspi_unicornhat_show},

		{0,0}
	};

/*
 	const luaL_reg meta[] =
	{
		{"__gc",			lua_raspi_unicornhat_destroy},
		{0,0}
	};
	
	luaL_newmetatable(l, lua_raspi_unicornhat_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);
*/

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);

	return 1;
}

