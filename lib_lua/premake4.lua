

project "lib_lua"
language "C"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
excludes { "src/luac.c" }
includedirs { "src" }

defines("LUA_PRELOADLIBS=lua_preloadlibs")

if NACL then -- nacl needs its own flags

else

end

KIND{}

