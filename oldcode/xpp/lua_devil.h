/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define LUA_DEVILLIBNAME	"devil"
int luaopen_devil (lua_State *l);



#define DEVILHANDLE "DevIL*"


struct devilhandle
{
	ILuint image;
	ILuint frame;
	ILuint layer;
	ILuint mipmap;
};

int lua_devil_error_check (lua_State *l);

devilhandle *lua_devil_to_imagep (lua_State *L, int index);
devilhandle *lua_devil_to_image (lua_State *l, int index);
void lua_devil_bind_imagep (devilhandle *p);

