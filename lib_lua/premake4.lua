
--if not NACL then -- we just link with prebuilt?

project "lib_lua"
language "C"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
includedirs { "src" }

SET_KIND("luamain")

defines("LUA_PRELOADLIBS=lua_preloadlibs")


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

--end
