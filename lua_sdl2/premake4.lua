
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
}

if NIX or OSX then
-- use system includes
	includedirs { "/usr/local/include/SDL2" }
elseif WINDOWS then
	includedirs {	"../lib_sdl2/win32/i686-w64-mingw32/include/SDL2",	}
elseif RASPI then
	includedirs {	"../lib_sdl2/raspi/usr/local/include/SDL2",	}
elseif EMCC then
	buildlinkoptions{
		"-s USE_SDL=2","-Wno-error=format-security",
	}
end

KIND{kind="lua",name="SDL"}

