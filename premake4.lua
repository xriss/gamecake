

function buildlinkoptions(t) buildoptions(t) linkoptions(t) end

------------------------------------------------------------------------
-- hacky premake functions
------------------------------------------------------------------------

function newplatform(plf)
    local name = plf.name
    local description = plf.description
 
    -- Register new platform
    premake.platforms[name] = {
        cfgsuffix = "_"..name,
        iscrosscompiler = true
    }
 
    -- Allow use of new platform in --platfroms
    table.insert(premake.option.list["platform"].allowed, { name, description })
    table.insert(premake.fields.platforms.allowed, name)
 
    -- Add compiler support
    -- gcc
    premake.gcc.platforms[name] = plf.gcc
    --other compilers (?)
end
 
function newgcctoolchain(toolchain)
    newplatform {
        name = toolchain.name,
        description = toolchain.description,
        gcc = {
            cc = toolchain.prefix .. "gcc",
            cxx = toolchain.prefix .. "g++",
            ar = toolchain.prefix .. "ar",
            cppflags = "-MMD " .. toolchain.cppflags,
        }
    }
end

--	local raspisdk=path.getabsolute("./sdks/raspi")
--	local raspisdk_gcc=raspisdk.."/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/"

-- use the cross compiler from ./sdks/raspi/tools
newplatform {
    name = "raspi",
    description = "raspi",
	gcc = {
		cc = "arm-linux-gnueabihf-gcc-4.8.3",
		cxx = "arm-linux-gnueabihf-g++",
		ar = "arm-linux-gnueabihf-ar",
		cppflags = "-MMD -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s",
	}
}

newplatform {
    name = "raspi-clang",
    description = "raspi",
--    prefix = "arm-bcm2708-linux-gnueabi-",
--    prefix = "arm-raspi-linux-gnueabi-",
--	prefix = "arm-linux-gnueabihf-",
    gcc = {
        cc = "clang",
        cxx = "clang++",
        ar= "ar",
		cppflags = "-target armv7-eab -marm -mfpu=vfp -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard",
    }
}



newgcctoolchain {
    name = "android",
    description = "android",
    prefix = "arm-linux-androideabi-",
    cppflags = "",
}

newgcctoolchain {
    name = "android-x86",
    description = "android-x86",
    prefix = "i686-android-linux-",
    cppflags = "",
}

newplatform {
    name = "mingwin",
    description = "mingw on windows",
	gcc=
	{
		cc ="gcc",
		cxx="c++",
		ar ="ar",
		cppflags = "-MMD",
	}
}
newplatform {
    name = "mingw32",
    description = "mingw32",
	gcc=
	{
		cc ="i586-mingw32msvc-cc",
		cxx="i586-mingw32msvc-c++",
		ar ="i586-mingw32msvc-ar",
		cppflags = "-MMD",
	}
}

newplatform {
    name = "mingw",
    description = "mingw",
	gcc=
	{
		cc ="i686-w64-mingw32-gcc",
		cxx="i686-w64-mingw32-c++",
		ar ="i686-w64-mingw32-ar",
		cppflags = "-MMD",
	}
}

newplatform {
    name = "clang",
    description = "clang",
    gcc = {
        cc = "clang",
        cxx = "clang++",
        ar= "ar",
        cppflags = "-MMD",
    }
}

newplatform {
    name = "nacl",
    description = "nacl",
    gcc = {
        cc = "pnacl-clang",
        cxx = "pnacl-clang++",
        ar= "pnacl-ar",
        cppflags = "-MMD",
    }
}

newplatform {
    name = "lsb",
    description = "lsb",
    gcc = {
        cc = "lsbcc",
        cxx = "lsbc++",
        ar= "ar",
        cppflags = "-MMD -D_FORTIFY_SOURCE=0",
    }
}

newplatform {
    name = "emcc",
    description = "emcc",
    gcc = {
        cc = "emcc",
        cxx = "em++",
        ar= "ar",
        cppflags = "-MMD",
    }
}

--[[ ios notez, something like this makefile but platforms are in /Applications

/Applications/Xcode.app/Contents/Developer/Platforms/

https://gitorious.org/mac-app-from-scratch/ios-app-from-scratch/source/ccb29c96bce7ee72aaa9bc222415f4393d008b9f:Makefile#L77

]]


------------------------------------------------------------------------
-- work out what we should be building for
------------------------------------------------------------------------

solution("wetgenes")

-- work out build type and set flags
EMCC=false
NACL=false
ANDROID=false
WINDOWS=false
MINGW=false
NIX=false
CPU="32"
TARGET="NIX"
GCC=false
CLANG=false

local t= _ARGS[1] or ""
if t:sub(1,5)=="raspi" then
	TARGET="RASPI"
	CPU=t:sub(6)
	RASPI=true
	GCC=true
elseif t:sub(1,4)=="nacl" then
	TARGET="NACL"
	CPU=t:sub(5)
	NACL=true
	GCC=true
elseif t:sub(1,4)=="emcc" then
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
	if t:sub(1,7)=="mingw32" then MINGW32=true end -- use old compiler
	if t:sub(1,7)=="mingwin" then MINGWIN=true end -- use exe compiler
	TARGET="WINDOWS"
	CPU=t:sub(6)
	WINDOWS=true
	MINGW=true
	GCC=true
	TARGET="WINDOWS"
	CPU=t:sub(6)
	WINDOWS=true
	MINGW=true
	GCC=true
elseif t:sub(1,3)=="nix" then
	TARGET="NIX"
	CPU=t:sub(4)
	NIX=true
	GCC=true
elseif t:sub(1,3)=="lsb" then
	TARGET="NIX"
	CPU=t:sub(4)
	NIX=true
	GCC=true
	LSB=true
elseif t:sub(1,5)=="clang" then
	TARGET="NIX"
	CPU=t:sub(6)
	NIX=true
	GCC=true
	CLANG=true
elseif t:sub(1,3)=="osx" then
	TARGET="OSX"
	CPU=t:sub(4)
	OSX=true
	GCC=true
	CLANG=true
elseif os.get() == "windows" then
	TARGET="WINDOWS"
	WINDOWS=true
else
	TARGET="NIX"
	NIX=true
end

if CPU=="32" then -- done
else
	if CPU=="-64" then
		CPU="64"
	elseif CPU=="-32" then
		CPU="32"
	elseif CPU=="-arm" then
		CPU="arm"
	elseif CPU=="-armv7" then
		CPU="armv7"
	else
		CPU="native"
	end
end

print("TARGET == "..TARGET.." " ..CPU )

if EMCC then

	defines "EMCC"

	buildlinkoptions{
--		"-Wno-warn-absolute-paths",
--		"-Wno-error-shift-negative-value",
		"-Wno-long-long",
		"-Werror",
		"-s NO_EXIT_RUNTIME=1",
		"-s ALLOW_MEMORY_GROWTH=1",
		"-Wno-almost-asm",
--		"-s ASSERTIONS=1",
		"-s \"BINARYEN_TRAP_MODE='clamp'\"",
		"-s WASM=1",
	}

	linkoptions{
		"-as-needed",
--		"-s RESERVED_FUNCTION_POINTERS=256",
--		"-s TOTAL_MEMORY=134217728",			-- 128meg
		"-s EXPORTED_FUNCTIONS=\"['_main_post']\"",
	}
	
	platforms { "emcc" }

-- set debug/release build flags
	configuration {"Debug"}
		buildlinkoptions{
			"-O0",
			"-g3",
			"-s ASSERTIONS=1",
--			"-s SAFE_HEAP=3",
--			"-s ALIASING_FUNCTION_POINTERS=0",
--			"--minify 0",
		}
	configuration {"Release"}
		buildlinkoptions{
			"-O3",
			"-g0",
		}
	configuration {}


elseif NACL then

	naclsdk_path=path.getabsolute( os.getenv("NACLPATH") ) -- set by make script as this keeps changing
	pepperjs_path=path.getabsolute("./lib_pepperjs/pepper.js")

	platforms { "nacl" } --hax
	
	defines "NACL"
	
	includedirs { naclsdk_path.."/include" }
	includedirs { naclsdk_path.."/include/newlib" }

--	includedirs { naclsdk_path.."/ports/include" }

-- libs to link
	configuration {"Debug"}
	libdirs { naclsdk_path.."/ports/lib/newlib_pnacl/Debug" }
	libdirs { naclsdk_path.."/lib/pnacl/Debug" }

	configuration {"Release"}
	libdirs { naclsdk_path.."/ports/lib/newlib_pnacl/Release" }
	libdirs { naclsdk_path.."/lib/pnacl/Release" }

	configuration {}

elseif RASPI then

	local raspisdk=path.getabsolute("./sdks/raspi")

--	includedirs { "/usr/local/include/luajit-2.1" } -- pickup our luajit / SDL2 builds from default install paths
--	includedirs { "/usr/local/include/SDL2" } -- pickup our luajit / SDL2 builds from default install paths

	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include" }
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include/interface/vmcs_host/linux" }
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include/interface/vcos/pthreads"}
	libdirs { raspisdk.."/firmware/hardfp/opt/vc/lib" }

	platforms { "raspi" } --hax

	defines "RASPI"

	defines("LUA_USE_POSIX")

--	buildoptions{ "-mtune=generic" }
	
	
elseif ANDROID then

	local androidsdk=path.getabsolute("./sdks/android-sdk")
	local androidsys=path.getabsolute("./sdks/android-9-arm/sysroot/usr")


	defines "ANDROID"

	defines("LUA_USE_POSIX")
	
-- these are application specific and need proper paths so will get overidden
AND_LIB_DIR=AND_LIB_DIR or path.getabsolute("android")

	if CPU=="32" then

		platforms { "android-x86" } --hax
	
		androidsys=path.getabsolute("./sdks/android-9-x86/sysroot/usr")
		
		AND_OUT_DIR=AND_OUT_DIR or path.getabsolute("android/libs/x86")

		buildoptions{ "-mtune=generic" }
--		linkoptions{ "-m32" }
		
	elseif CPU=="arm" then
	
		platforms { "android" } --hax
		
		buildoptions{ "-mthumb"  }
		
		AND_OUT_DIR=AND_OUT_DIR or path.getabsolute("android/libs/armeabi")
		
	elseif CPU=="armv7" then
	
		platforms { "android" } --hax

		buildoptions{ "-march=armv7-a" , "-mfloat-abi=softfp" , "-mfpu=vfpv3" }
		linkoptions{ "--fix-cortex-a8" }

		AND_OUT_DIR=AND_OUT_DIR or path.getabsolute("android/libs/armeabi-v7a")
	end

	includedirs { androidsys.."/include" }
	libdirs { androidsys.."/lib" }

elseif WINDOWS then

	defines "_WIN32_WINNT=0x0501"
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

	if MINGW then
	
		if MINGW32 then		platforms { "mingw32" } --use old 32 bit compiler
		elseif MINGWIN then	platforms { "mingwin" } --use exe compiler
		else				platforms { "mingw" }	--use new 64 bit compiler
		end
		
		local w32api=path.getabsolute("./sdks/w32api") -- do we still need this?
		
		includedirs { w32api.."/include" }
		libdirs { w32api.."/lib" }
		
		linkoptions { "-static-libgcc" }
		
		buildoptions{"-mtune=generic"}
		linkoptions {"-mtune=generic"}

	end

elseif OSX then
	
--	includedirs { "/usr/local/include/SDL2" } -- pickup our luajit / SDL2 builds from default install paths

	defines("LUA_USE_MKSTEMP") -- remove warning
	defines("LUA_USE_POPEN") -- we want to enable popen

--	defines "X11"
--	defines	"LUA_USE_DLOPEN"
--	linkoptions "-Wl,-rpath=\\$$ORIGIN:."
	
--	if CLANG then
		platforms { "clang" } --hax
--	end

	buildoptions{"-mmacosx-version-min=10.6"}
	linkoptions {"-mmacosx-version-min=10.6"}

	if CPU=="32" then
	
		buildoptions{"-m32 -msse -msse2 -mtune=generic"}
		linkoptions {"-m32 -msse -msse2 -mtune=generic"}
		
	elseif CPU=="64" then
	
		buildoptions{"-m64 -mtune=generic"}
		linkoptions {"-m64 -mtune=generic"}
		
	end
	
elseif NIX then

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

	if LSB then
		platforms { "lsb" } --hax
	elseif CLANG then
		platforms { "clang" } --hax
	end

	if CPU=="32" then
	
		buildoptions{"-m32 -msse -msse2 -mtune=generic"}
		linkoptions{"-m32 -msse -msse2 -mtune=generic"}
		
	elseif CPU=="64" then
	
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

	local t=TARGET
	if CLANG and t=="NIX" then t="clang" end
	if LSB and t=="NIX" then t="lsb" end

	BUILD_DIR_BASE=("build-"..(_ACTION or "gmake").."-"..t.."-"..CPU):lower()
	
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
local function csplit(str,c)
	local ret={}
	local n=1
	for w in str:gmatch("([^"..c.."]*)") do
			ret[n]=ret[n] or w -- only set once (so the blank after a string is ignored)
			if w=="" then n=n+1 end -- step forwards on a blank but not a string
	end
	return ret
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
		
			configuration {"Debug"}
			targetdir(AND_OUT_DIR..d)

			configuration {"Release"}
			targetdir(AND_OUT_DIR..d)
			
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

	
--
-- assume we have a prebuilt luajit.so for the target platform
--
-- use asm.sh in lib_luajit to build them all
--
-- this only needs to be done if you change buildflags or luajit code
-- so it should be a very rare action 
-- it seems to be OK to link the x32/x64 linux libs on LSB builds
-- these built binary files are added into the repository.
--
-- This is currently only still used for MINGW ...
--

if EMCC then -- need to build and use our lua

	LUA_BIT="lua_bit"
	LIB_LUA="lib_lua" -- default 
	includedirs { "lib_lua/src" }
	LUA_LINKS= nil

elseif RASPI then -- hardfloat for raspbian

--	defines{ "LIB_LUAJIT" }
	includedirs { "lib_luajit/src" }
	LUA_LIBDIRS={ "../lib_luajit/libs/armhf/" }
	LUA_LINKS= { "luajit" }

elseif ANDROID then

--	defines{ "LIB_LUAJIT" }
	includedirs { "lib_luajit/src" }
	LUA_LIBDIRS={ "../lib_luajit/libs/arm/" }
	LUA_LINKS= { "luajit" }

elseif MINGW then

--	defines{ "LIB_LUAJIT" }
	includedirs { "lib_luajit/src" }
	LUA_LIBDIRS={ "../lib_luajit/libs/win32/" }
	LUA_LINKS= { "luajit" }

else -- luajit

-- we expect luajit to be provided in the system

	includedirs { "/usr/local/include/luajit-2.1" } -- assume only one of these exists
	includedirs { "/usr/include/luajit-2.1" }
	includedirs { "/usr/include/luajit-2.0" }
	LUA_LINKS="luajit-5.1"

-- or expect lua to be provided in the system by swapping with above

--	LUA_BIT="lua_bit"
--	includedirs { "/usr/include/lua5.2" }
--	LUA_LINKS="lua5.2"

end

-- pick the os interface we will build, you can force one with environment
-- most of them are variants on linux so this can be useful
GAMECAKE_WIN_TYPE=os.getenv("GAMECAKE_WIN_TYPE")

-- or we look at what code we are building
if not GAMECAKE_WIN_TYPE then
	
	if     WINDOWS then GAMECAKE_WIN_TYPE="windows"
	elseif NIX     then GAMECAKE_WIN_TYPE="linux"
	elseif NACL    then GAMECAKE_WIN_TYPE="nacl"
	elseif EMCC    then GAMECAKE_WIN_TYPE="emcc"
	elseif ANDROID then GAMECAKE_WIN_TYPE="android"
	elseif RASPI   then GAMECAKE_WIN_TYPE="raspi"
	elseif OSX     then GAMECAKE_WIN_TYPE="osx"
	end

end
-- allow no win option, maybe usefull?
if GAMECAKE_WIN_TYPE=="none" then GAMECAKE_WIN_TYPE=false end


-- many many versions of GL to suport, these make this work -> #include INCLUDE_GLES_GL
if RASPI or GAMECAKE_WIN_TYPE=="raspi" then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	
elseif ANDROID then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	
elseif NACL or EMCC then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }

elseif WINDOWS then -- need windows GL hacks

	includedirs { "lua_gles/code" }
	defines{ "LUA_GLES_GL" }
	if GCC then
		defines{ "INCLUDE_GLES_GL=\\\"GL3/gl3w.h\\\"" }
	else
		defines{ "INCLUDE_GLES_GL=\"GL3/gl3w.h\"" }
	end

else -- use GL 

	includedirs { "lua_gles/code" }
	defines{ "LUA_GLES_GL" }

	defines{ "INCLUDE_GLES_GL=\\\"GL3/gl3w.h\\\"" }

end

-- Any web type build these are all kinda similar
WEB=NACL or EMCC


includedirs { "./lib_hacks/code" }
includedirs { "./lua_freetype/code" }
includedirs { "./lua_grd/code" }

	
all_includes=all_includes or {

-- lua bindings
	{LUA_BIT,		    WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_kissfft",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_pack",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_zip",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_zlib",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_freetype",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_ogg",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_al",		   (WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		)
																						and		(not PEPPER) 			},
	{"lua_tardis",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_gles",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_grd",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_grdmap",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_sod",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_socket",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_gamecake",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_win",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_lfs",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_profiler",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_posix",		nil			or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_lash",		WINDOWS		or		NIX		or		EMCC	or		nil			or		nil			or	OSX		},
	
	{"lua_win_"..GAMECAKE_WIN_TYPE, GAMECAKE_WIN_TYPE }, -- pick the os interface, see above

	{"lua_sdl2",	   (WINDOWS		or		NIX		or		EMCC	or		nil			or		RASPI		or	OSX		)
																						and		(not LSB) 				},

--new lua bindings and libs (maybe be buggy unfinshed or removed at anytime)
	{"lua_chipmunk",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_raspi_unicornhat",																	RASPI					},
	{"lua_utf8",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_cmsgpack",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_periphery",	nil			or		NIX		or		nil		or		nil			or		RASPI		or	nil		},
	{"lua_v4l2",		nil			or		NIX		or		nil		or		nil			or		RASPI		or	nil		},
	{"lua_rex",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
	{"lua_linenoise",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_brimworkszip",WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_sys",			WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_polarssl",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lib_polarssl",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lib_zip",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_pgsql",		nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
	{"lib_pq",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
	{"lua_opus",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_opus",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_speexdsp",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},

-- this may be the main lua or luajit lib depending on build
-- would really like to just use luajit but nacl mkes this a problem...
	{LIB_LUA,			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},

-- static libs used by the lua bindings
	{"lib_zzip",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_png",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_jpeg",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_gif",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_z",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_freetype",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_vorbis",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_ogg",			WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_openal",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	nil		},
	{"lib_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_pcre",		nil			or		NIX		or		nil		or		nil			or		nil			or	OSX		},

-- dont think building this is a good idea?
--	{"lib_angle",		nil			or		nil		or		nil		or		nil			or		nil			or	nil		},

-- link lanes on nix?
	{"lua_lanes",		NIX		},

-- test glslang?
	{"lua_glslang",		NIX		},

-- old/broken and no longer supported but will probably still build
--	{"lua_speak",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	OSX		},
--	{"lua_lanes",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	OSX		},
--	{"lua_sec",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
--	{"lib_openssl",		nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},

-- need to lazy link to .so before we can add these back in otherwise they force unwanted dependencies
--	{"lua_hid",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
--	{"lib_hidapi",		nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},

	{"lib_hacks",		WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},

-- the output executables
	{"exe_gamecake",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
--	{"exe_pagecake",	nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
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




