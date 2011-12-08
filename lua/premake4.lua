
project "lua"
language "C"

includedirs { "../lib_lua/src" }



--dofile("cache.lua")
--dofile("preloadlibs.lua")

links(lua_lib_names)

if NACL then -- we just link with prebuilt

	linkoptions { "-v"  }

	links { "ppapi" }--, "ppapi_cpp" } -- , "nosys"
	
--	links { "lib_lua" }
	links { "lib_nacl" }

--	links {  "crt_platform", "ppruntime", "nosys", "nacl" , "platform", "m" , "pthread" }
		
--	links {  "crt_common" , "crt_platform" }


--	links { "stdc++" }
	links { "m" }
	
	SET_KIND("WindowedApp")
	SET_TARGET("","lua.32.nexe",true)

elseif ANDROID then 

	linkoptions { "-v" }
	links { "lib_lua" , "dl", "log", "GLESv1_CM", "c", "m", "gcc" }
	linkoptions{ "-Bsymbolic"}

	files { "../lib_lua/src/*.h", --[["src/lua.c"]]  }
	SET_KIND("SharedLib")
	SET_TARGET("","liblua",true)

else

	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }

	if os.get() == "windows" then

		links { "lib_lua" }

		SET_KIND("ConsoleApp")
		SET_TARGET("","lua.exe",true)

	else -- nix

-- we need to include libs again here for linking, cant prelink with statics?
-- it should probably auto handle stuff
-- anyway it gets complicated, so this is all hax

		links { "lib_lua" }
		links { "lib_z" }
		links { "lua_grd_libpng" }
		links { "lua_zip_zziplib"}
		links { "GL" , "GLU" }
		links { "crypt" }
		links { "pthread" }
		
		links { "dl" , "m" , "pthread" }
		
		SET_KIND("ConsoleApp")
		SET_TARGET("","lua",true)
	end

end
