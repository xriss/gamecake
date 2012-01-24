
project "lua"
language "C"

includedirs { "../lib_lua/src" }



dofile("cache.lua")
dofile("preloadlibs.lua")

links(lua_lib_names)

if NACL then

	linkoptions { "-v" }

	links { "ppapi" , "ppapi_gles2" }
	
	links { LIB_LUA }
	links { "lib_z" }
--	links { "lib_sqlite" }
	
	links { "lib_nacl" }

	links { "m" , "stdc++" }
	links { "ppapi" , "ppapi_gles2" }
	
	SET_KIND("WindowedApp")
	SET_TARGET("","lua.32.nexe",true)

elseif ANDROID then 

	linkoptions { "-v" }
	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link

	links { "lib_android" }

	links { LIB_LUA }
	links { "lib_z" }
--	links { "lib_sqlite" }
	
	
	links { "dl", "log", "GLESv1_CM", "c", "m", "gcc" }
	
	linkoptions{ "-Bsymbolic"}

	files { "../lib_lua/src/*.h", --[["src/lua.c"]]  }
	SET_KIND("SharedLib")
	SET_TARGET("","liblua",true)

elseif WINDOWS then

	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }

-- we need to include libs again here for linking, cant prelink with statics?
-- it should probably auto handle stuff
-- anyway it gets complicated, so this is all hax

	links { LIB_LUA }
	links { "lib_z" }
	links { "lib_sqlite" }
	links { "lib_png" }
	links { "lib_jpeg" }
	links { "lib_zzip"}
	links { "opengl32" , "glu32" }
	links { "stdc++" , "ws2_32" , "gdi32"}

--LLIBS = ['-lshell32', '-lshfolder', '-ldxguid', '-lgdi32', '-lmsvcrt', '-lwinmm', '-lmingw32', '-lm', '-lws2_32', '-lz', '-lstdc++'] 



	SET_KIND("ConsoleApp")
	SET_TARGET("","lua.exe",true)

elseif NIX then

	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }

-- we need to include libs again here for linking, cant prelink with statics?
-- it should probably auto handle stuff
-- anyway it gets complicated, so this is all hax

	links { LIB_LUA }
	links { "lib_z" }
	links { "lib_sqlite" }
	links { "lib_png" }
	links { "lib_jpeg" }
	links { "lib_zzip"}
	links { "GL" , "GLU" }
	links { "crypt" }
	links { "pthread" }
	
	links { "dl" , "m" , "pthread" }
	
	SET_KIND("ConsoleApp")
	SET_TARGET("","lua",true)

end
