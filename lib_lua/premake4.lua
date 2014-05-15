

project "lib_lua"
language "C"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
excludes { "src/luac.c" }
includedirs { "src" }

defines("LUA_PRELOADLIBS=lua_preloadlibs")

if NIX then -- nacl needs its own flags

if LSB then -- readline is bad for lsb, actually readline is just bad but simple to use
else
--defines("LUA_USE_READLINE")
end

end

KIND{}

