
newoption {
   trigger     = "openal",
   value       = "soft",
   description = "Choose openal",
   allowed = {
      { "soft", "build our own" },
      { "sys",  "System provided" },
   }
}

function buildlinkoptions(t) buildoptions(t) linkoptions(t) end


function newplatform(platform)
 
    platform.cfgsuffix = ""
    platform.iscrosscompiler = true

    premake.gcc.platforms[platform.name] = platform.gcc
    premake.platforms[platform.name] = platform

    table.insert(premake.option.list["platform"].allowed, { platform.name, platform.description })
    table.insert(premake.fields.platforms.allowed, platform.name)
 
end

ANDROID_VERSION="29"

newplatform {
    name = "android-a32",
    description = "android 32 bit arm",
    cpu_id = "a32",
    cpu_name = "armeabi-v7a",
	gcc=
	{
		cc ="armv7a-linux-androideabi"..ANDROID_VERSION.."-clang",
		cxx="armv7a-linux-androideabi"..ANDROID_VERSION.."-clang++",
		ar ="llvm-ar",
		cppflags = "-MMD -fPIC",	-- should we build in thumb mode?  "-mthumb"
	}
}
newplatform {
    name = "android-a64",
    description = "android 64 bit arm",
    cpu_id = "a64",
    cpu_name = "arm64-v8a",
	gcc=
	{
		cc ="aarch64-linux-android"..ANDROID_VERSION.."-clang",
		cxx="aarch64-linux-android"..ANDROID_VERSION.."-clang++",
		ar ="llvm-ar",
		cppflags = "-MMD -fPIC",
	}
}

newplatform {
    name = "android-x32",
    description = "android 32 bit intel",
    cpu_id = "x32",
    cpu_name = "x86",
	gcc=
	{
		cc ="i686-linux-android"..ANDROID_VERSION.."-clang",
		cxx="i686-linux-android"..ANDROID_VERSION.."-clang++",
		ar ="llvm-ar",
		cppflags = "-MMD -fPIC",
	}
}
newplatform {
    name = "android-x64",
    description = "android 64 bit intel",
    cpu_id = "x64",
    cpu_name = "x86_64",
	gcc=
	{
		cc ="x86_64-linux-android"..ANDROID_VERSION.."-clang",
		cxx="x86_64-linux-android"..ANDROID_VERSION.."-clang++",
		ar ="llvm-ar",
		cppflags = "-MMD -fPIC",
	}
}

newplatform {
    name = "mingwin",
    description = "mingw on windows",
	gcc=
	{
		cc ="gcc",
		cxx="c++",
		ar ="ar",
		cppflags = "-MMD -fPIC",
	}
}

newplatform {
    name = "mingw",
    description = "mingw",
	gcc=
	{
		cc ="i686-w64-mingw32-gcc-posix",
		cxx="i686-w64-mingw32-c++-posix",
		ar ="i686-w64-mingw32-ar",
		cppflags = "-MMD -fPIC",
	}
}

newplatform {
    name = "gcc",
    description = "gcc",
    gcc = {
        cc = "gcc",
        cxx = "g++",
        ar= "ar",
        cppflags = "-MMD -fPIC",
    }
}

newplatform {
    name = "osx",
    description = "osx",
    gcc = {
        cc = "clang",
        cxx = "clang++",
        ar= "ar",
        cppflags = "-MMD -fPIC",
    }
}

newplatform {
    name = "clang",
    description = "clang",
    gcc = {
        cc = "clang",
        cxx = "clang++",
        ar= "ar",
        cppflags = "-MMD -fPIC",
    }
}

newplatform {
    name = "emcc",
    description = "emcc",
    gcc = {
        cc = "emcc",
        cxx = "em++",
        ar= "emar",
        cppflags = "-MMD -fPIC",
    }
}

newplatform {
    name = "clang-arm",
    description = "clang",
    gcc = {
        cc = "clang --target=armv7-linux-gnu",
        cxx = "clang++ --target=armv7-linux-gnu",
        ar= "ar",
        cppflags = "-MMD -fPIC",
    }
}


------------------------------------------------------------------------
-- work out what we should be building for
------------------------------------------------------------------------

solution("wetgenes")

-- work out build type and set flags
EMCC=false
ANDROID=false
WINDOWS=false
MINGW=false
NIX=false
CPU="NATIVE"
TARGET="NIX"
NIX=false
GCC=false
CLANG=false

local t= _ARGS[1] or ""

if t:sub(1,4)=="emcc" then
	TARGET="EMCC"
	CPU=t:sub(5)
	EMCC=true
	GCC=true
elseif t:sub(1,7)=="android" then
	TARGET="ANDROID"
	CPU=t:sub(8)
	ANDROID=true
	GCC=true
elseif t:sub(1,5)=="mingw" then
	if t:sub(1,7)=="mingwin" then MINGWIN=true end -- use exe compiler
	TARGET="MINGW"
	CPU=t:sub(6)
	WINDOWS=true
	MINGW=true
	GCC=true
	CPU=t:sub(6)
	WINDOWS=true
	MINGW=true
	GCC=true
elseif t:sub(1,3)=="gcc" then
	TARGET="GCC"
	CPU=t:sub(4)
	NIX=true
	GCC=true
elseif t:sub(1,5)=="clang" then
	TARGET="CLANG"
	CPU=t:sub(6)
	NIX=true
	GCC=true
	CLANG=true
elseif t:sub(1,3)=="osx" then
	TARGET="OSX"
	CPU=t:sub(4)
	OSX=true
	CLANG=true
else
	TARGET="NIX"
	NIX=true
end

if CPU=="native" then -- done
else
	if 			CPU=="-64" 		then		CPU="x64"
	elseif 		CPU=="-32" 		then		CPU="x32"
	elseif 		CPU=="-x64" 	then		CPU="x64"
	elseif 		CPU=="-x32" 	then		CPU="x32"
	elseif 		CPU=="-a32" 	then		CPU="a32"
	elseif 		CPU=="-a64" 	then		CPU="a64"
	else									CPU="native"
	end
end

print("TARGET == "..TARGET.." " ..CPU )

if EMCC then

	defines "EMCC"

	buildlinkoptions {
		"-pthread",
		"-Wno-pthreads-mem-growth",
		"-s USE_PTHREADS",
	}
	linkoptions{
--		"-s PTHREAD_POOL_SIZE=4",
		"-s PROXY_TO_PTHREAD",
		"-s OFFSCREEN_FRAMEBUFFER",
	}


	buildlinkoptions{
		"-Wno-long-long",
--		"-Werror",
		"-Wno-almost-asm",
	}

	linkoptions{
		"-as-needed",
--		"--emrun",
		"-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 ",
		"-s ALLOW_MEMORY_GROWTH=1",
--		"-s TOTAL_MEMORY=1GB",
		"-s \"BINARYEN_METHOD='native-wasm'\"",
		"-s EXPORTED_RUNTIME_METHODS=\"['cwrap']\"",
		"-s WASM=1",
		"-s EXPORTED_FUNCTIONS=\"['_main']\"",
	}
	
	platforms { "emcc" }

-- set debug/release build flags
	configuration {"Debug"}
		linkoptions{
			"-s ASSERTIONS=1",
			"-s SAFE_HEAP=1",
		}
		buildlinkoptions{
			"-O0",
			"-g3",
--			"-s ALIASING_FUNCTION_POINTERS=0",
--			"--minify 0",
		}
	configuration {"Release"}
		buildlinkoptions{
			"-g0",
			"-O3",
--			"-fno-exceptions",
		}
	configuration {}


elseif ANDROID then

--	local androidsdk=path.getabsolute("./sdks/android-sdk")
--	local androidsys=path.getabsolute("./sdks/android-9-arm/sysroot/usr")


	local platform=premake.platforms[ "android-"..CPU ]

	platforms { "android-"..CPU } --hax

	defines "ANDROID"

	defines("LUA_USE_POSIX")
	
	includedirs { "exe/android/include" }
	libdirs { path.getabsolute("exe/android/lib/"..platform.cpu_name) }

	buildoptions{ "-mtune=generic" }


elseif WINDOWS then

	defines "_WIN32_WINNT=0x0501"
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

	if MINGW then
	
		if MINGWIN then	platforms { "mingwin" } --use exe compiler
		else			platforms { "mingw" }	--use new 64 bit compiler
		end
		
		buildoptions{"-mtune=generic"}

	end

elseif OSX then
	
--	includedirs { "/usr/local/include/SDL2" } -- pickup our luajit / SDL2 builds from default install paths

	defines("LUA_USE_MKSTEMP") -- remove warning
	defines("LUA_USE_POPEN") -- we want to enable popen
	
	platforms { "osx" } --hax

	buildoptions{"-mmacosx-version-min=10.12"}
	linkoptions {"-mmacosx-version-min=10.12"}

	if CPU=="x32" then
	
		buildoptions{"-m32 -msse -msse2 -mtune=generic"}
		linkoptions {"-m32 -msse -msse2 -mtune=generic"}
		
	elseif CPU=="x64" then
	
		buildoptions{"-m64 -mtune=generic"}
		linkoptions {"-m64 -mtune=generic"}
		
	end
	
elseif NIX then

	platforms { "gcc" } --default hax

	buildlinkoptions {
		"-pthread",
	}

	buildoptions{
		"-Wno-format-security",
		"-Wno-deprecated-declarations",
	}

	function os.capture(cmd, raw)
	  local f = assert(io.popen(cmd, 'r'))
	  local s = assert(f:read('*a'))
	  f:close()
	  if raw then return s end
	  s = string.gsub(s, '^%s+', '')
	  s = string.gsub(s, '%s+$', '')
	  s = string.gsub(s, '[\n\r]+', ' ')
	  return s
	end

	BUILD_CPU = os.capture("getconf LONG_BIT") -- remember the build cpu "32"/"64"

--	BUILD_ARCH=os.capture("dpkg --print-architecture") -- better guess?
	
--	includedirs { "/usr/local/include/SDL2" } -- pickup our luajit / SDL2 builds from default install paths

	defines("LUA_USE_MKSTEMP") -- remove warning
	defines("LUA_USE_POPEN") -- we want to enable popen

	defines "X11"
--	defines	"LUA_USE_DLOPEN"
	linkoptions "-Wl,-rpath=\\$$ORIGIN:."

	if CLANG then
		platforms { "clang" } --hax
	end

	if CPU=="x32" then
	
		buildoptions{"-m32 -msse -msse2 -mtune=generic"}
		linkoptions{"-m32 -msse -msse2 -mtune=generic"}
		
	elseif CPU=="x64" then
	
		buildoptions{"-m64 -mtune=generic"}
		linkoptions{"-m64 -mtune=generic"}
	
	elseif CPU=="a64" then
	
		buildoptions{"-m64 -mtune=generic"}
		linkoptions{"-m64 -mtune=generic"}
	
	elseif CPU=="native" then -- do not mess with build flags?
	
--		buildoptions{"-mtune=generic"} -- aim for a stable build?
--		linkoptions{"-mtune=generic"}
--		buildoptions{"-march=native"}
--		linkoptions{"-march=native"}
				
	end
	
end






if not BUILD_DIR_BASE then

	BUILD_DIR_BASE=("build-"..(_ACTION or "gmake").."-"..TARGET.."-"..CPU):lower()
	
end

if not BUILD_DIR then

	BUILD_DIR=BUILD_DIR_BASE

end

location( BUILD_DIR )


	

configurations { "Debug", "Release" }


EXE_OUT_DIR=path.getabsolute("exe")
DBG_OUT_DIR=path.getabsolute("dbg")



ALL_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj")
EXE_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Release")
DBG_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Debug")


static_lib_names={}

lua_lib_names={}
lua_lib_loads={}


-- deadsimple single char string spliter, c must be a single char possibly escaped with %
-- this is a simple path or modulename spliting function
local function csplit(p,c)
	local ps={}
	local fi=1
	while true do
		local fa,fb=string.find(p,c,fi)
		if fa then
			local s=string.sub(p,fi,fa-1)
			ps[#ps+1]=s
			fi=fb+1
		else
			break
		end
	end
	ps[#ps+1]=string.sub(p,fi)
	return ps
end

function KIND(opts)

-- apply to all configs
	configuration{}

	opts=opts or {}
	
	if opts.lua then -- shorthand lua options, fill in other opts with it
	
		opts.kind="lua" 

		opts.luaname=opts.lua -- the full lua module name
		
		local aa=csplit(opts.lua,"%.") -- break it on every .
		
		opts.luaopen=table.concat(aa,"_") -- replace . with _ and that is the call function
		
		opts.name=aa[#aa] -- last part is the name
		
		aa[#aa]=nil -- remove name and what everything else is the dir
		
		opts.dir=table.concat(aa,"/")
		
	end

	if opts.kind=="lua" then
	
		opts.kind="StaticLib" -- lua turns to static
		
		lua_lib_names[#lua_lib_names+1]=project().name
		lua_lib_loads[#lua_lib_loads+1]={opts.luaname or opts.name,opts.luaopen or opts.luaname or opts.name}

--print( "LIB" , lua_lib_loads[#lua_lib_loads][1] , lua_lib_loads[#lua_lib_loads][2] )
	end
	
	
	opts.kind=opts.kind or "StaticLib" -- default kind
	
	kind(opts.kind)


	
	if opts.name and opts.kind~="StaticLib" then -- force an output target name
	
		targetprefix ("")
		targetname (opts.name)
		
	end

-- setup configurations

	configuration {"Debug"}
	flags {"Symbols"} -- blue debug needs symbols badly

	configuration {"Release"}
	flags {"Optimize"}
--	flags {"Symbols"} -- keep symbols to help with release only crashes
	
-- set output dirs

	if opts.kind~="StaticLib" then -- force output dir
	
		local d=""
		if opts.dir and opts.dir~="" then d="/"..opts.dir end
		
		if ANDROID then
		
			local platform=premake.platforms[ "android-"..CPU ]

			configuration {"Debug"}
			targetdir(DBG_OUT_DIR.."/android/lib/"..platform.cpu_name..d)

			configuration {"Release"}
			targetdir(EXE_OUT_DIR.."/android/lib/"..platform.cpu_name..d)

print(EXE_OUT_DIR.."/android/lib/"..platform.cpu_name..d)

		else

			configuration {"Debug"}
			targetdir(DBG_OUT_DIR..d)

			configuration {"Release"}
			targetdir(EXE_OUT_DIR..d)

		end
		
	else
	
		static_lib_names[#static_lib_names+1]=project().name
	
		configuration {"Debug"}
		targetdir(DBG_OBJ_DIR)

		configuration {"Release"}
		targetdir(EXE_OBJ_DIR)
	end
	
end



------------------------------------------------------------------------
-- which lua version should we use
------------------------------------------------------------------------

if EMCC then -- need to build and use our lua

	LUA_BIT="lua_bit"
	LIB_LUA="lib_lua" -- default 
	includedirs { "lib_lua/src" }
	LUA_LINKS= nil

elseif ANDROID then

	defines{ "LUA_JIT_USED" }

	LUA_LINKS= { "luajit" }

elseif MINGW then

	defines{ "LUA_JIT_USED" }

	LUA_LINKS= { "luajit" }

-- build these files using build/install --mingw
	includedirs { "/usr/i686-w64-mingw32/include/luajit" }
--	libdirs { "/usr/i686-w64-mingw32/lib/luajit" }

--	includedirs { path.getabsolute("./vbox_mingw/luajit/include") }
--	libdirs { path.getabsolute("./vbox_mingw/luajit/lib") }

else -- luajit

	defines{ "LUA_JIT_USED" }

-- we expect luajit to be provided in the system

	includedirs { "/usr/local/include/luajit-2.1" } -- assume only one of these possible locations is correct
	includedirs { "/usr/local/64/include/luajit-2.1" }
	includedirs { "/usr/include/luajit-2.1" }
	includedirs { "/usr/include/luajit-2.0" }
	includedirs { "/app/include/luajit-2.1" } -- flatpack build

	LUA_LINKS="luajit-5.1"

-- or expect lua to be provided in the system by swapping this with above

--	LUA_BIT="lua_bit"
--	includedirs { "/usr/include/lua5.2" }
--	LUA_LINKS="lua5.2"

end

-- pick the os interface we will build, you can force one with environment
-- most of them are variants on linux so this can be useful
GAMECAKE_WIN_TYPE=os.getenv("GAMECAKE_WIN_TYPE")

-- or we look at what code we are building
if (not GAMECAKE_WIN_TYPE) or (GAMECAKE_WIN_TYPE=="") then
	
	if     WINDOWS then GAMECAKE_WIN_TYPE="windows"
	elseif NIX     then GAMECAKE_WIN_TYPE="linux"
	elseif EMCC    then GAMECAKE_WIN_TYPE="emcc"
	elseif ANDROID then GAMECAKE_WIN_TYPE="android"
	elseif OSX     then GAMECAKE_WIN_TYPE="osx"
	end

end
-- allow no win option, maybe usefull?
if GAMECAKE_WIN_TYPE=="none" then GAMECAKE_WIN_TYPE=false end


-- many many versions of GL to suport, these make this work -> #include INCLUDE_GLES_GL
if EMCC or ANDROID then

	defines{ "LUA_GLES_GLES3" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES3/gl3.h\\\"" }

elseif WINDOWS then -- need windows GL hacks

	includedirs { "lua_gles/include" }
	defines{ "LUA_GLES_GL" }
	if GCC then
		defines{ "INCLUDE_GLES_GL=\\\"GL/gl3w.h\\\"" }
	else
		defines{ "INCLUDE_GLES_GL=\"GL/gl3w.h\"" }
	end

else -- use GL 

	includedirs { "lua_gles/include" }
	defines{ "LUA_GLES_GL" }

	defines{ "INCLUDE_GLES_GL=\\\"GL/gl3w.h\\\"" }

end

-- add get SDL2 include files

if MINGW then

-- build these files using build/install --mingw
	includedirs { "/usr/i686-w64-mingw32/include/SDL2" }
--	libdirs { "/usr/i686-w64-mingw32/lib/SDL2" }

--	includedirs { path.getabsolute("./vbox_mingw/SDL2/include/SDL2") }
--	libdirs { path.getabsolute("./vbox_mingw/SDL2/lib") }

elseif EMCC then

	buildlinkoptions{
		"-Wno-error=format-security",
		"-s USE_SDL=2",
	}
	linkoptions{
		"-s FULL_ES3=1",
		"-s USE_WEBGL2=1",
		"-s MIN_WEBGL_VERSION=2",
		"-s MAX_WEBGL_VERSION=2",
	}

else

-- use system includes
	includedirs { "/usr/local/include/SDL2" }
	includedirs { "/usr/include/SDL2" }

end

-- OpenAL

--print(_OPTIONS)
--for n,v in pairs(_OPTIONS) do print(n,v) end

if _OPTIONS["openal"]=="sys" then
	print("USING SYSTEM PROVIDED OPENAL")
	LIB_OPENAL=nil
else
	LIB_OPENAL="lib_openal"
	includedirs { "./lib_openal/mojoal" }
	defines("AL_LIBTYPE_STATIC")
end


includedirs { "./lib_hacks/code" }
includedirs { "./lua_freetype/code" }
includedirs { "./lua_grd/code" }


all_includes=all_includes or {

-- lua bindings that should always be available no matter the OS host.
	{LUA_BIT,		    WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_djon",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_kissfft",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_pack",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_fats",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_zip",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_zlib",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_freetype",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_ogg",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_al",		    WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_tardis",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_gles",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_grd",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_grdmap",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_sod",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_socket",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_sec",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_gamecake",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_win",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_lfs",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_sqlite",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_lash",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_sdl2",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_bullet",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_chipmunk",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_utf8",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_cmsgpack",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_brimworkszip",WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_opus",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lua_lanes",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},

-- emcc needs a little bit of special sauce
--	{"lua_win_emcc",	nil			or		nil		or		EMCC		or		nil			or	nil		},


-- These are mostly linux only bindings for linux only gamecake projects...
	{"lua_linenoise",	WINDOWS		or		NIX		or		nil			or		nil			or	OSX		},
	{"lua_posix",		nil			or		NIX		or		nil			or		nil			or	OSX		},
	{"lua_periphery",	nil			or		NIX		or		nil			or		nil			or	nil		},
	{"lua_v4l2",		nil			or		NIX		or		nil			or		nil			or	nil		},
	{"lua_rex",			nil			or		NIX		or		nil			or		nil			or	nil		},
	{"lua_sys",			WINDOWS		or		NIX		or		nil			or		nil			or	OSX		},
	{"lua_glslang",		nil			or		NIX		or		nil			or		nil			or	nil		},
	{"lua_midi",		nil			or		NIX		or		nil			or		nil			or	nil		},

	{"lua_pgsql",		nil			or		NIX		or		nil			or		nil			or	nil		},
	{"lib_pq",			nil			or		NIX		or		nil			or		nil			or	nil		},

-- hid is now in SDL
--	{"lua_hid",			nil			or		NIX		or		nil			or		nil			or	nil		},
--	{"lib_hidapi",		nil			or		NIX		or		nil			or		nil			or	nil		},

-- this is probably luajit but may be lua 5.1
	{LIB_LUA,			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},

-- static libs used by the lua bindings so they should be linked afterwards
	{"lib_wolfssl",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_opus",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_speexdsp",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_png",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_jpeg",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_gif",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_z",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_zip",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_zzip",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_freetype",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_vorbis",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_ogg",			WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_sqlite",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_hacks",		WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},
	{"lib_pcre",		nil			or		NIX		or		nil			or		nil			or	OSX		},

-- some OS will provide openal so do not need this.
	{LIB_OPENAL,		WINDOWS		or		NIX		or		nil 		or		ANDROID		or	nil		},

-- the output executables
	{"exe_gamecake",	WINDOWS		or		NIX		or		EMCC		or		ANDROID		or	OSX		},

}

------------------------------------------------------------------------
-- include sub projects depending on above build tests or you could choose
-- to set all_includes before including this file to choose your own config
------------------------------------------------------------------------

for i,v in ipairs(all_includes) do
	if v[1] and v[2] then
		include(v[1])
	end
end




