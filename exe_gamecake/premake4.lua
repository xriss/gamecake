
project "gamecake"
language "C++"

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

	libdirs { "/opt/vc/lib" }
	libdirs { "/usr/local/lib/" }
	links { "SDL2" }
		
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

	links { "GLESv2" }
	
	links { "EGL" , "android" , "jnigraphics" , "OpenSLES" }
	links { "dl", "log", "c", "m", "gcc" }

	KIND{kind="SharedLib",name="liblua"}

elseif WINDOWS then

	linkoptions { "-Wl,--export-all-symbols" }

	files { "./lua.c" }

	libdirs { "../lua_sdl2/windows/i686-w64-mingw32/lib/" } -- we have SDL2 binary for windows, and have deleted the dll, so it has no choice but static
	links { "SDL2" , "m" , "dinput8" , "dxguid" , "dxerr8" , "user32" , "gdi32" , "winmm" , "imm32" , "ole32" , "oleaut32" , "shell32" , "version" , "uuid" }

	links { "opengl32" , "glu32" }
	links {  "ws2_32" , "gdi32"}
	
	if GCC then
		links { "stdc++" , "mingw32" }
	end
		
	links { "winmm" }

	linkoptions{ "-mconsole" }

	local exe=".exe"
	if not GCC then exe="" end -- native builds add .exe automatically	

	KIND{kind="ConsoleApp",name="gamecake"..exe}

elseif OSX then

-- look around the exe for any dynamic code we might want	
--	linkoptions { "-Wl,-R\\$$ORIGIN/osx" } -- so much escape \\$$ -> $

	files { "./lua.c" }

	libdirs { "/usr/local/64/lib/" }
	libdirs { "/usr/local/lib/" }

	links { "SDL2" }	

	links { "ForceFeedback.framework" } -- SDL2 requires these frameworks
	links { "Carbon.framework" }
	links { "IOKit.framework" }
	links { "CoreAudio.framework" }
	links { "CoreVideo.framework" }
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

	linkoptions { "-Wl,-R\\$$ORIGIN,-R\\$$ORIGIN/x32,-R\\$$ORIGIN/x64" } -- so much escape \\$$ -> $

	files { "./lua.c" }

	if LSB then
		linkoptions { "--lsb-use-default-linker" }
		linkoptions { "--lsb-besteffort" }
	end

	libdirs { "/usr/local/lib/" }
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



