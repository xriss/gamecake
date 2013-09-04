
project "gamecake"
language "C"

files { "hacks.c" }


GAMECAKE_VERSION="13.659"

dofile("cache.lua")
dofile("preloadlibs.lua")

links(static_lib_names)
links(static_lib_names) -- so good, so good, we linked it twice...


--print("LIBS TO LINK ",table.concat(static_lib_names,","))


if RASPI then
	
	files { "../lib_lua/src/lua.c" }
	
	links { "GLESv2" , "EGL" , "vcos" , "bcm_host" , "vchiq_arm"}
	links { "crypt" }
	links { "pthread" }

--	links { "X11"  }
	
	links { "dl" , "m" , "pthread" ,"rt"}

	linkoptions { "-v" }
--	linkoptions { "-v -nostdlib" }
--	links {  "gcc" , "c" , "c++" }

	KIND{kind="ConsoleApp",name="gamecake.raspi"}

elseif NACL then

	files { "../nacl/code/lua_force_import.c" }

	linkoptions { "-v -O0" }
	
	links { "ppapi"  }
	links { "ppapi_gles2" }
	links { "m" , "stdc++" }
	links { "pthread" }
	links { "nosys" } -- remove newlib link errors
	links { "nosys" } -- remove newlib link errors
	
	KIND{kind="WindowedApp",name="gamecake."..CPU..".nexe"}

elseif ANDROID then 

--	linkoptions { "-v" }
	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link
	linkoptions { "-u android_main" } -- we really need an android_main as well

    linkoptions { "../lib_luajit/src/libluajit.a" }
    
    	
--	links { "GLESv1_CM" }
	links { "GLESv2" }
	
	links { "EGL" , "android" , "jnigraphics" , "OpenSLES" }
	links { "dl", "log", "c", "m", "gcc" }	
	
--	linkoptions{ "-Bsymbolic"}

	KIND{kind="SharedLib",name="liblua"}


elseif WINDOWS then

	files { "../lib_lua/src/lua.c" }

	links { "opengl32" , "glu32" }
	links {  "ws2_32" , "gdi32"}
	
	if GCC then
		links { "stdc++" , "mingw32" }
	end
		
	links { "winmm" }

--	links { "comdlg32" } -- we need to remove this when we impliment our own file-requester
	
--	linkoptions{ "--enable-stdcall-fixup" }

	local exe=".exe"
	if not GCC then exe="" end -- native builds add .exe automatically	
	KIND{kind="ConsoleApp",name="gamecake"..exe}

elseif NIX then

--	linkoptions { "-static-libgcc" }

	files { "../lib_lua/src/lua.c" }
	
--	linkoptions { "-v" }

	links { "GL" }
--	links { "GLU" }

--	links { "udev" }

	links { "crypt" }
	links { "pthread" }
	links { "X11"   }	
	links { "dl" }
	links { "m" }

--	links { "rt" }

	if CPU=="64" then
		KIND{kind="ConsoleApp",name="gamecake.x64"}
	else
		KIND{kind="ConsoleApp",name="gamecake"}
	end
	
end



