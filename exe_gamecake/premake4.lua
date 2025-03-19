
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


if EMCC then

	files { "./lua.c" }

	linkoptions { "-rdynamic" }

	linkoptions { "-lidbfs.js" }
	
	linkoptions { "-v" }
	
	links { "m" }
	
	KIND{kind="WindowedApp",name="gamecake.js"}

elseif ANDROID then 
	
	linkoptions { "-static-libstdc++" }
	linkoptions { "-static-libgcc" }
	linkoptions { "-Wl,-soname,libgamecake.so" }

--	linkoptions { "-Wl,-z,defs" }

--	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link
--	linkoptions { "-u android_main" } -- we really need an android_main as well

-- these must be dynamic
	links { "GLESv2" , "EGL" , "android" , "jnigraphics" , "OpenSLES" , "log"  }

	links { "dl",  "c", "m", }

	KIND{kind="SharedLib",name="libgamecake"}

elseif WINDOWS then

	linkoptions { "-static" }

	linkoptions { "-Wl,--export-all-symbols" }

	files { "./lua.c" }

	links { "SDL2" , "m" , "dinput8" , "dxguid" , "dxerr8" , "user32" , "gdi32" , "winmm" , "imm32" , "ole32" , "oleaut32" , "shell32" , "version" , "uuid" , "setupapi" }

--	links { "opengl32" , "glu32" }
	links {  "ws2_32" , "gdi32"}
	
	if GCC then
		links { "stdc++" , "mingw32" }
	end
		
	links { "winmm" }

	links { "psapi" }

	linkoptions{ "-mconsole" }

	local exe=".exe"
	if not GCC then exe="" end -- native builds add .exe automatically	

	KIND{kind="ConsoleApp",name="gamecake"..exe}

elseif OSX then

	files { "./lua.c" }

--	libdirs { "/usr/local/64/lib/" }
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

	if CPU=="x32" then
		KIND{kind="WindowedApp",name="gamecake.osx32"}
	else
		linkoptions { "-pagezero_size 10000","-image_base 100000000" }
	
		KIND{kind="WindowedApp",name="gamecake.osx"}
	end
	
elseif NIX then

--	linkoptions { "-static" }
	linkoptions { "-static-libstdc++" }
	linkoptions { "-static-libgcc" }

	linkoptions { "-Wl,-R\\$$ORIGIN,-R\\$$ORIGIN/arm,-R\\$$ORIGIN/x32,-R\\$$ORIGIN/x64" } -- so much escape \\$$ -> $

	files { "./lua.c" }

	libdirs { "/usr/local/lib/" }

	if _OPTIONS["openal"]=="sys" then -- link to system openal

		links { "openal" }

	end

if not MIDIJUNKIES then
	links { "SDL2" }
	links { "udev" }
	links { "asound" }
	links { "crypt" }
	links { "pthread" }
end


	links { "dl" }
	links { "m" }
	links { "rt" }
	links { "c" }

	if CPU=="x64" then
		KIND{kind="ConsoleApp",name="gamecake.x64"}
	elseif CPU=="x32" then
		KIND{kind="ConsoleApp",name="gamecake.x32"}
	elseif CPU=="a32" then
		KIND{kind="ConsoleApp",name="gamecake.a32"}
	elseif CPU=="a64" then
		KIND{kind="ConsoleApp",name="gamecake.a64"}
	else
		KIND{kind="ConsoleApp",name="gamecake.nix"}
	end
	
end



