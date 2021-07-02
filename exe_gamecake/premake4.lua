
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

	buildlinkoptions { "-pthread" , "-Wno-pthreads-mem-growth" }

	linkoptions { "-s PROXY_TO_PTHREAD" }

	linkoptions { "-rdynamic" }
	
	linkoptions { "-v" }
	
	links { "m" }
	
	KIND{kind="WindowedApp",name="gamecake.html"}

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

	links { "SDL2" , "m" , "dinput8" , "dxguid" , "dxerr8" , "user32" , "gdi32" , "winmm" , "imm32" , "ole32" , "oleaut32" , "shell32" , "version" , "uuid" }

--	links { "opengl32" , "glu32" }
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

	if CPU=="32" then
		KIND{kind="WindowedApp",name="gamecake.osx32"}
	else
		linkoptions { "-pagezero_size 10000","-image_base 100000000" }
	
		KIND{kind="WindowedApp",name="gamecake.osx"}
	end
	
elseif NIX then

	linkoptions { "-Wl,-R\\$$ORIGIN,-R\\$$ORIGIN/arm,-R\\$$ORIGIN/x32,-R\\$$ORIGIN/x64" } -- so much escape \\$$ -> $

	files { "./lua.c" }

	libdirs { "/usr/local/lib/" }
	links { "SDL2" }

	if _OPTIONS["openal"]=="sys" then -- link to system openal

		links { "openal" }

	end

	links { "udev" }
	
	links { "asound" }

--	links { "GL" }
	links { "crypt" }
	links { "pthread" }
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



