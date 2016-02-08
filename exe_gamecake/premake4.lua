
project "gamecake"
language "C++"

files { "hacks.c" }

dofile("version.lua")

dofile("cache.lua")
dofile("preloadlibs.lua")

links(static_lib_names)
links(static_lib_names) -- so good, so good, we linked it twice...

-- link in luajit that was compiled externally
if LUA_LIBDIRS then	libdirs(LUA_LIBDIRS) end
if LUA_LINKS   then links  (LUA_LINKS)   end


if RASPI or GAMECAKE_WIN_TYPE=="raspi" then
	
-- look around the exe for any dynamic code we might want	
	linkoptions { "-Wl,-R\\$$ORIGIN/rpi" } -- so much escape \\$$ -> $

	files { "./lua.c" }

-- now building in qemu vbox, so must fiddle :/

-- luajit and SDL2 dev must be available
-- use the qemu box if you dont want to deal with it
	libdirs { "/opt/vc/lib" }
	libdirs { "/usr/local/lib/" }
	links { "luajit-5.1" }
	links { "SDL2" }
	
	
--	libdirs { "../lib_sdl2/raspi/usr/local/lib/" } -- we have SDL2 binary for raspi
--	linkoptions { " -Wl,-Bstatic,-lSDL2,-Bdynamic " } -- force static SDL2 linking


	links { "GLESv2" , "EGL" , "vcos" , "bcm_host" , "vchiq_arm"}
	links { "crypt" }
	links { "pthread" }
	
	links { "dl" , "m" , "pthread" ,"rt"}

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

	links { "m" }
	
	if TARGET=="PEPPER" then
		
		KIND{kind="WindowedApp",name="gamecake.js"}

	else

--		links { "SDL2" }	-- expected to be pre-built
		
--		links { "stdc++" }
		links { "ppapi"  }
		links { "ppapi_gles2" }
		links { "nacl_io" }
		links { "pthread" }
		links { "nosys" } -- remove newlib link errors
		links { "nosys" } -- remove newlib link errors


		KIND{kind="WindowedApp",name="gamecake.pexe"}
	end


elseif ANDROID then 
	
	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link
	linkoptions { "-u android_main" } -- we really need an android_main as well

--	links { "SDL2" }	-- expected to be pre-built

	links { "GLESv2" }
	
	links { "EGL" , "android" , "jnigraphics" , "OpenSLES" }
	links { "dl", "log", "c", "m", "gcc" }

	KIND{kind="SharedLib",name="liblua"}

elseif WINDOWS then

	linkoptions { "-Wl,--export-all-symbols" }

	files { "./lua.c" }

	libdirs { "../lib_sdl2/win32/i686-w64-mingw32/lib/" } -- we have SDL2 binary for windows
	linkoptions { " -Wl,-Bstatic,-lSDL2,-lm,-ldinput8,-ldxguid,-ldxerr8,-luser32,-lgdi32,-lwinmm,-limm32,-lole32,-loleaut32,-lshell32,-lversion,-luuid,-Bdynamic " }  -- force static SDL2 linking

	links { "opengl32" , "glu32" }
	links {  "ws2_32" , "gdi32"}
	
	if GCC then
		links { "stdc++" , "mingw32" }
	end
		
	links { "winmm" }

--	linkoptions{ "-mwindows" }
	linkoptions{ "-mconsole" }

	local exe=".exe"
	if not GCC then exe="" end -- native builds add .exe automatically	

--	KIND{kind="WindowedApp",name="gamecake"..exe}
	KIND{kind="ConsoleApp",name="gamecake"..exe}

elseif OSX then

-- look around the exe for any dynamic code we might want	
--	linkoptions { "-Wl,-R\\$$ORIGIN/osx" } -- so much escape \\$$ -> $

	files { "./lua.c" }

--	if CPU=="64" then
--		libdirs { "/usr/local/64/lib/" }
--	else
		libdirs { "/usr/local/lib/" }
--	end

	links { "luajit-5.1" }
	links { "SDL2" }	

	links { "ForceFeedback.framework" } -- SDL2 requires these frameworks
	links { "Carbon.framework" }
	links { "IOKit.framework" }
	links { "CoreAudio.framework" }
	links { "AudioToolbox.framework" }
	links { "AudioUnit.framework" }

	links { "OpenGL.framework" }
	links { "OpenAL.framework" }
	links { "Cocoa.framework" }

	links { "pthread" }
	links { "dl" }
	links { "m" }
	links { "objc" }
	links { "iconv" }

	if CPU=="64" then
		linkoptions { "-pagezero_size 10000","-image_base 100000000" }
	
		KIND{kind="WindowedApp",name="gamecake.osx"}
	else
		KIND{kind="WindowedApp",name="gamecake.osx32"}
	end
	
elseif NIX then

-- look around the exe for any dynamic code we might want	
	if CPU=="64" or ( CPU~="32" and BUILD_CPU=="64" ) then
		linkoptions { "-Wl,-R\\$$ORIGIN/x64" } -- so much escape \\$$ -> $
	elseif CPU=="32" or BUILD_CPU=="32" then
		linkoptions { "-Wl,-R\\$$ORIGIN/x32" } -- so much escape \\$$ -> $
	else
		linkoptions { "-Wl,-R\\$$ORIGIN" } -- so much escape \\$$ -> $
	end

	files { "./lua.c" }

	if LSB then
		linkoptions { "--lsb-use-default-linker" }
		linkoptions { "--lsb-besteffort" }
	end

-- luajit and SDL2 dev must be available
-- use the vagrant boxes if you dont want to deal with it
	libdirs { "/usr/local/lib/" }
	links { "luajit-5.1" }
	links { "SDL2" }	

	links { "GL" }
	links { "crypt" }
	links { "pthread" }
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



