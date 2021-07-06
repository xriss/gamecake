
project "lua_win"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." , "code" }

includedirs {
	"../lua_sdl2/luasdl2",
	"../lua_sdl2/luasdl2/src",
	"../lua_sdl2/luasdl2/extern/queue",
}


KIND{lua="wetgenes.win.core"}

