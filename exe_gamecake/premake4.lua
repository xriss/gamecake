
project "gamecake"
language "C++"

files { "hacks.c" }


--GAMECAKE_VERSION="14.001"
dofile("version.lua")

dofile("cache.lua")
dofile("preloadlibs.lua")

links(static_lib_names)
links(static_lib_names) -- so good, so good, we linked it twice...


--print("LIBS TO LINK ",table.concat(static_lib_names,","))

-- link in luajit that was compiled externally
if LUA_LIBDIRS then	libdirs(LUA_LIBDIRS) end
if LUA_LINKS   then links  (LUA_LINKS)   end


if RASPI then
	
--	linkoptions { "-Wl,-static" }

	files { "./lua.c" }

-- use prebuilt SDL2 lib	
	libdirs { "../lib_sdl2/raspi/usr/local/lib/" }
	links { "SDL2" }
	linkoptions { "-Wl,-R\\$$ORIGIN" } -- so much escape \\$$ -> $


	links { "GLESv2" , "EGL" , "vcos" , "bcm_host" , "vchiq_arm"}
	links { "crypt" }
	links { "pthread" }

--	links { "X11"  }
	
	links { "dl" , "m" , "pthread" ,"rt"}

	linkoptions { "-v" }
--	linkoptions { "-v -nostdlib" }
--	links {  "gcc" , "c" , "c++" }

	KIND{kind="ConsoleApp",name="gamecake.raspi"}

elseif EMCC then

	linkoptions { "-rdynamic" }
	
	buildlinkoptions{
		"-s USE_SDL=2",
	}
	
	linkoptions { "-v" }
	
	links { "m" }
	
	KIND{kind="WindowedApp",name="gamecake.js"}

elseif NACL then


--	files { "../nacl/code/lua_force_import.c" }

--	linkoptions { "-v -O0" }
	linkoptions { "-v" }
	
--use ports version...
--	links { "openal"  }

	links { "m" }
	
	if TARGET=="PEPPER" then
		
		KIND{kind="WindowedApp",name="gamecake.js"}

	else

		links { "stdc++" }
		links { "ppapi"  }
		links { "ppapi_gles2" }
		links { "nacl_io" }
		links { "pthread" }
		links { "nosys" } -- remove newlib link errors
		links { "nosys" } -- remove newlib link errors


		KIND{kind="WindowedApp",name="gamecake.pexe"}
	end


elseif ANDROID then 

--	linkoptions { "-Wl,-static" }
	linkoptions { "-rdynamic" }
	
--	linkoptions { "-v" }
	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link
	linkoptions { "-u android_main" } -- we really need an android_main as well

--    linkoptions { "../lib_luajit/src/libluajit.a" }
    
    	
--	links { "GLESv1_CM" }
	links { "GLESv2" }
	
	links { "EGL" , "android" , "jnigraphics" , "OpenSLES" }
	links { "dl", "log", "c", "m", "gcc" }	
	
--	linkoptions{ "-Bsymbolic"}

	KIND{kind="SharedLib",name="liblua"}


elseif WINDOWS then

	linkoptions { "-Wl,--export-all-symbols" }
--	linkoptions { "-Wl,-static" }

	linkoptions { "-v" }

	files { "./lua.c" }

	links { "opengl32" , "glu32" }
	links {  "ws2_32" , "gdi32"}
	
	if GCC then
		links { "stdc++" , "mingw32" }
	end
		
	links { "winmm" }

	linkoptions{ "-mwindows" }

	local exe=".exe"
	if not GCC then exe="" end -- native builds add .exe automatically	
	KIND{kind="WindowedApp",name="gamecake"..exe}

elseif OSX then

	files { "./lua.c" }
	
--	linkoptions { "-v" }

	links { "OpenGL.framework" }
	links { "OpenAL.framework" }
--	links { "CoreAudio.framework" }
--	links { "CoreFoundation.framework" }
	links { "Cocoa.framework" }


-- use static SDL2 from sdks
	libdirs { "../../sdks/sdl2/sdl2_osx/build/.libs/" }
	links { "SDL2" }
	
	
--	links { "Carbon.framework" }



--	links { "udev" }

--	links { "crypt" }
	links { "pthread" }
--	links { "X11"   }	
	links { "dl" }
	links { "m" }

--	links { "rt" }

	if CPU=="64" then
--		KIND{kind="ConsoleApp",name="gamecake.osx64"}
		KIND{kind="WindowedApp",name="gamecake.osx64"}
	else
--		KIND{kind="ConsoleApp",name="gamecake.osx"}
		KIND{kind="WindowedApp",name="gamecake.osx"}
	end
	
elseif NIX then

--	linkoptions { "-Wl,-static" }
	linkoptions { "-rdynamic" }

--	linkoptions { "-static-libgcc" }

	files { "./lua.c" }

if LSB then
	linkoptions { "--lsb-use-default-linker" }
	linkoptions { "--lsb-besteffort" }
else
--	links { "SDL2" }
end


-- use prebuilt SDL2 lib
	if CPU=="64" then
		libdirs { "../../sdks/sdl2/sdl2_x64/build/.libs/" }
	elseif CPU=="32" then
		libdirs { "../../sdks/sdl2/sdl2_x32/build/.libs/" }
	end
	links { "SDL2" }



--	linkoptions { "-v" }

	links { "GL" }
--	links { "GLU" }

--	links { "udev" }

	links { "crypt" }
	links { "pthread" }
--	links { "Xrandr" }
	links { "X11"   }
	links { "dl" }
	links { "m" }

	links { "rt" }
	links { "c" }

	if CPU=="64" then
		KIND{kind="ConsoleApp",name="gamecake.x64"}
	elseif CPU=="32" then
		KIND{kind="ConsoleApp",name="gamecake.x32"}
	else
		KIND{kind="ConsoleApp",name="gamecake.nix"}
	end
	
end



