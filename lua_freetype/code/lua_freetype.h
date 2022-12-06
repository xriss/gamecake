/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/



#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_freetype_core (lua_State *l);

#ifdef __cplusplus
};
#endif


//
// We own the data stored here
//
struct lua_freetype_font
{
	int error;
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */
};




#define LUA_freetype_LIB_NAME "freetype"

extern const char *lua_freetype_ptr_name;

