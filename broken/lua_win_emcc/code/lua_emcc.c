
#include <emscripten.h>

#include "lua_emcc.h"


static lua_State *L=0;




static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message("", msg);
    lua_pop(L, 1);
  }
  return status;
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
//  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
//  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

static int dostringr (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 0);
  return report(L, status);
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// post sends a msg to the javascript side (need javascript side code to handle it)
// I'm using a cmd=%s\n header, directly followed by custom data depending on the command
// the idea is this is url encoded args on first line and cmd=blah is just the normal use
// also remember js strings are 16bit not 8bit, so, you know, that is a bit shit.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_emcc_js_post(lua_State *l)
{
const char *s1=0;
const char *s2=0;
	
	s1=lua_tostring(l,1);

	if( lua_isstring(l,2) )
	{
		s2=lua_tostring(l,2);
	}

	if(s2)
	{
		EM_ASM_ARGS({
			var s0=UTF8ToString($0);
			var s1=UTF8ToString($1);
			setTimeout(function(){
				Module.msg(s0,s1);
			},0);
		},s1,s2);
	}
	else
	{
		EM_ASM_ARGS({
			var s0=UTF8ToString($0);
			setTimeout(function(){
				Module.msg(s0);
			},0);
		},s1);
	}
	
	return 0;
}

extern void lua_preloadlibs(lua_State *L);
void main_setup()
{
// L is static see top of file
	L = lua_open();  /* create state */
	luaL_openlibs(L);  /* open libraries */
	lua_preloadlibs(L); /* preload gamecake builtin libs */
}

int main_post(const char *message,const char *data)
{
	int ret=0;

	if(!L) { main_setup(); }
	
	int slen=strlen("cmd=lua");
	if (strncmp(message, "cmd=lua", slen) == 0)
	{
		int top=lua_gettop(L);
		data=data ? data : (message+slen+1);
		ret=dostringr(L,data,data);
		lua_settop(L,top);
	}
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------

Eval a javascript string and return a possible string value from that function

+-----------------------------------------------------------------------------------------------------------------+*/
int lua_emcc_js_eval(lua_State *l)
{
const char *eval=0;
const char *result=0;

	eval=lua_tostring(l,1);

	result=emscripten_run_script_string(eval);
	
	if(result)
	{
		lua_pushstring(l,result);
//		free((void*)result);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------

handle clipboard through javascript hacks

+-----------------------------------------------------------------------------------------------------------------+*/

int lua_emcc_has_clipboard(lua_State *l)
{
	lua_pushboolean(l,1); // just say yes always
	return 1;
}

EM_JS(void, lua_emcc_set_clipboard_js, (const char* data), {
	var s0=UTF8ToString(data);
	Module.gamecake.set_clipboard(s0);
});

int lua_emcc_set_clipboard(lua_State *l)
{
	lua_emcc_set_clipboard_js( lua_tostring(l,1) );
	return 0;
}

EM_JS(char*, lua_emcc_get_clipboard_js, (), {
	var jsString = Module.gamecake.get_clipboard();
	var lengthBytes = lengthBytesUTF8(jsString)+1;
	var stringOnWasmHeap = _malloc(lengthBytes);
	stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
	return stringOnWasmHeap;
});

int lua_emcc_get_clipboard(lua_State *l)
{
	char *s=lua_emcc_get_clipboard_js();
	lua_pushstring(l,s);
	free(s);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_emcc_core(lua_State *l)
{
	const luaL_Reg lib[] =
	{
		
		{	"js_post",						lua_emcc_js_post					},
		{	"js_eval",						lua_emcc_js_eval					},

		{	"has_clipboard",				lua_emcc_has_clipboard				},
		{	"set_clipboard",				lua_emcc_set_clipboard				},
		{	"get_clipboard",				lua_emcc_get_clipboard				},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}


