
project "lua_sdl2"
language "C"

files {
	"luasdl2/src/*.c",
	"luasdl2/src/*.h",
	"luasdl2/common/*.c",
	"luasdl2/common/*.h",
}
includedirs {
	"luasdl2",
	"luasdl2/src",
	"../../sdks/sdl2/sdl2_x32/include",
}


KIND{kind="lua",name="SDL"}

