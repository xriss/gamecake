
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

if NIX then
	if CPU=="64" then
		includedirs {	"../../sdks/sdl2/sdl2_x64/include",	}
	elseif CPU=="32" then
		includedirs {	"../../sdks/sdl2/sdl2_x32/include",	}
	end
end

if OSX then
	includedirs {	"../../sdks/sdl2/sdl2_osx/include",	}
end

if WINDOWS then
	includedirs {	"../lib_sdl2/win32/i686-w64-mingw32/include/SDL2",	}
end

if RASPI then
	includedirs {	"../lib_sdl2/raspi/usr/local/include/SDL2",	}
end


KIND{kind="lua",name="SDL"}

