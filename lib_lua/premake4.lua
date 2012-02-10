
--if not NACL then -- we just link with prebuilt?

project "lib_lua"
language "C"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
includedirs { "src" }

defines("LUA_PRELOADLIBS=lua_preloadlibs")

KIND{}

--end
