
--if not NACL then -- we just link with prebuilt?

project "lib_lua"
language "C"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
includedirs { "src" }

SET_KIND("luamain")

if NIX then
printf("MKSTEMP")
	defines("LUA_USE_MKSTEMP") -- remove warning
	defines("LUA_USE_POPEN") -- we want to enable popen
end

--if #lua_lib_names>0 then
	defines("LUA_PRELOADLIBS=lua_preloadlibs")
--end


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

--end
