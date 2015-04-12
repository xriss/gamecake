

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

newgcctoolchain {
    name = "raspi",
    description = "raspi",
--    prefix = "arm-bcm2708-linux-gnueabi-",
--    prefix = "arm-raspi-linux-gnueabi-",
	prefix = "arm-linux-gnueabihf-",
    cppflags = "",
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


newgcctoolchain {
    name = "mingw",
    description = "mingw",
    prefix = "i586-mingw32msvc-",
    cppflags = "",
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
    name = "pepper",
    description = "pepper",
    gcc = {
        cc = "emcc",
        cxx = "em++",
        ar= "ar",
        cppflags = "-MMD",
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
PEPPER=false
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
elseif t:sub(1,6)=="pepper" then
	TARGET="PEPPER"
	CPU=t:sub(7)
	NACL=true
	PEPPER=true
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
		"-Wno-warn-absolute-paths",
		"-Wno-long-long",
		"-Werror",
		"-s NO_EXIT_RUNTIME=1",
		"-s ALLOW_MEMORY_GROWTH=1",
	}

	linkoptions{
		"-as-needed",
--		"-s RESERVED_FUNCTION_POINTERS=256",
--		"-s TOTAL_MEMORY=134217728",			-- 128meg
		"-s EXPORTED_FUNCTIONS=\"['_main','_main_post']\"",
--		"--pre-js "..pepperjs_path.."/ppapi_preamble.js",
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

	naclsdk_path=path.getabsolute("../sdks/nacl-sdk/pepper_33")
	pepperjs_path=path.getabsolute("./lib_pepperjs/pepper.js")

	if TARGET=="PEPPER" then

		defines "PEPPER"	
		defines "NACL_ARCH=x86_32"

			buildlinkoptions{
				"-Wno-warn-absolute-paths",
				"-Wno-long-long",
				"-Werror",
			}

			linkoptions{
			"-as-needed",
			"-s RESERVED_FUNCTION_POINTERS=400",
			"-s TOTAL_MEMORY=134217728",			-- 128meg
			"-s EXPORTED_FUNCTIONS=\"['_DoPostMessage', '_DoChangeView', '_DoChangeFocus', '_NativeCreateInstance', '_HandleInputEvent']\"",
			"--pre-js "..pepperjs_path.."/ppapi_preamble.js",
--			"--pre-js "..pepperjs_path.."/wrappers/audio.js",
			"--pre-js "..pepperjs_path.."/wrappers/base.js",
			"--pre-js "..pepperjs_path.."/wrappers/file.js",
			"--pre-js "..pepperjs_path.."/wrappers/gles.js",
			"--pre-js "..pepperjs_path.."/wrappers/gles_ext.js",
			"--pre-js "..pepperjs_path.."/wrappers/graphics_2d.js",
			"--pre-js "..pepperjs_path.."/wrappers/graphics_3d.js",
			"--pre-js "..pepperjs_path.."/wrappers/input_events.js",
			"--pre-js "..pepperjs_path.."/wrappers/mouse_lock.js",
			"--pre-js "..pepperjs_path.."/wrappers/url_loader.js",
			"--pre-js "..pepperjs_path.."/wrappers/view.js",
			"--pre-js "..pepperjs_path.."/wrappers/web_socket.js",
--			"--pre-js "..pepperjs_path.."/third_party/w3c_audio.js",
--			"--pre-js "..pepperjs_path.."/third_party/idb.filesystem.js",
		}
		
		platforms { "pepper" } --hax
	else
		platforms { "nacl" } --hax
	end
	
	defines "NACL"
	
	includedirs { naclsdk_path.."/include" }
--	includedirs { naclsdk_path.."/ports/include" }

-- libs to link
	configuration {"Debug"}
	libdirs { naclsdk_path.."/ports/lib/newlib_pnacl/Debug" }
	libdirs { naclsdk_path.."/lib/pnacl/Debug" }

	if TARGET=="PEPPER" then
		buildlinkoptions{
			"-O0",
			"-g4",
			"-s ASSERTIONS=2",
			"-s SAFE_HEAP=3",
			"-s ALIASING_FUNCTION_POINTERS=0",
			"--minify 0",
		}
	end

	configuration {"Release"}
	libdirs { naclsdk_path.."/ports/lib/newlib_pnacl/Release" }
	libdirs { naclsdk_path.."/lib/pnacl/Release" }

	if TARGET=="PEPPER" then
		buildlinkoptions{
			"-O1",
			"-g0",
		}
	end

	configuration {}

elseif RASPI then

	local raspisdk=path.getabsolute("../sdks/raspi")
	local raspxsdk=path.getabsolute("../sdks/raspx")

	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include" }
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include/interface/vmcs_host/linux" }
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include/interface/vcos/pthreads"}
	libdirs { raspisdk.."/firmware/hardfp/opt/vc/lib" }

-- im not sure its even worth trying to run X11 gl code?
-- best case is we use a large background window to catch clicks/keys but do
-- everything  else the same and take over the screen.

--	includedirs { raspisdk.."/raspx/usr/include" } -- extra includes?
--	libdirs { raspisdk.."/raspx/usr/lib/arm-linux-gnueabihf/" } -- extra libs?

--	includedirs { raspxsdk.."/include" } -- extra includes?
--	libdirs { raspxsdk.."/lib/arm-linux-gnueabihf" } -- extra libs?
--	libdirs { raspxsdk.."/lib" } -- extra libs?

	platforms { "raspi" } --hax

--	defines "X11"
	defines "RASPI"

	defines("LUA_USE_POSIX")
	
--	buildoptions{"-march=armv6zk -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s" }
	buildoptions{"-mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s" }



elseif ANDROID then

	local androidsdk=path.getabsolute("../sdks/android-sdk")
	local androidsys=path.getabsolute("../sdks/android-9-arm/sysroot/usr")


	defines "ANDROID"

	defines("LUA_USE_POSIX")
	
-- these are application specific and need proper paths so will get overidden
AND_LIB_DIR=AND_LIB_DIR or path.getabsolute("android")

	if CPU=="32" then

		platforms { "android-x86" } --hax
	
		androidsys=path.getabsolute("../sdks/android-9-x86/sysroot/usr")
		
		AND_OUT_DIR=AND_OUT_DIR or path.getabsolute("android/libs/x86")

--		buildoptions{ "-m32" }
--		linkoptions{ "-m32" }
		
	elseif CPU=="arm" then
	
		platforms { "android" } --hax
		
		buildoptions{ "-mthumb" }
		
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
	
		platforms { "mingw" } --hax

		local w32api=path.getabsolute("../sdks/w32api")
		
		includedirs { w32api.."/include" }
		libdirs { w32api.."/lib" }
		
		linkoptions { "-static-libgcc" }
		
	end

elseif OSX then
	
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
	
		buildoptions{"-m32 -msse -msse2"}
		linkoptions {"-m32 -msse -msse2"}
		
	elseif CPU=="64" then
	
		buildoptions{"-m64"}
		linkoptions {"-m64"}
		
	end
	
elseif NIX then
	
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
	
		buildoptions{"-m32 -msse -msse2"}
		linkoptions{"-m32 -msse -msse2"}
		
	elseif CPU=="64" then
	
		buildoptions{"-m64"}
		linkoptions{"-m64"}
	
	elseif CPU=="native" then
	
		buildoptions{"-march=native"}
		linkoptions{"-march=native"}
				
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


EXE_OUT_DIR=path.getabsolute("../bin/exe")
DBG_OUT_DIR=path.getabsolute("../bin/dbg")



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
-- which lua version should we usr
------------------------------------------------------------------------

LIB_LUA="lib_lua" -- default 

if RASPI or ANDROID or NIX or MINGW or OSX then -- luajit is working for these builds

	LIB_LUA="lib_luajit"
	defines( "LIB_LUAJIT" )

end

-- make sure we have the right headers
if LIB_LUA=="lib_lua" then

	includedirs { "lib_lua/src" }
	LUALINKS= nil

-- assume we have a prebuilt luajit, use asm.lua in luajit to build them all.
else
	includedirs { "lib_luajit/src" }
	
	LUA_LINKS= { "luajit" }


	if RASPI then -- hardfloat for raspbian

		LUA_LIBDIRS={ "../lib_luajit/libs/armhf/" }

	elseif ANDROID then

		LUA_LIBDIRS={ "../lib_luajit/libs/arm/" }

	elseif MINGW then

		LUA_LIBDIRS={ "../lib_luajit/libs/win32/" }

	elseif OSX then

		LUA_LIBDIRS={ "../lib_luajit/libs/osx/" }

	elseif CPU=="64" then
		
		LUA_LIBDIRS={ "../lib_luajit/libs/x64/" }

	elseif CPU=="32" then
	
		LUA_LIBDIRS= { "../lib_luajit/libs/x86/"  }
	
	else
	
		LUA_LIBDIRS= { "../lib_luajit/libs/native/"  }

	end

end


-- many many versions of GL to suport, these make this work -> #include INCLUDE_GLES_GL
if RASPI then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	
elseif ANDROID then

--	defines{ "LUA_GLES_GLES1" }
--	defines{ "INCLUDE_GLES_GL=\\\"GLES/gl.h\\\"" }

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	
elseif NACL or EMCC then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
--	defines{ "INCLUDE_GLES_GL=\\\"ppapi/c/ppb_opengles2.h\\\"" }

elseif WINDOWS then -- need windows GL hacks

--[[
	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	includedirs { "lib_angle/include" }
]]

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
WEB=NACL or EMCC or PEPPER

all_includes=all_includes or {

-- lua bindings
	{"lua_bit",		   (WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		)
																						and		(LIB_LUA=="lib_lua") 	},
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
	{"lua_lfs",			WINDOWS		or		NIX		or		EMCC	or		ANDROID		or		RASPI		or	OSX		},
	{"lua_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		or	OSX		},
	{"lua_profiler",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_posix",		nil			or		NIX		or		nil		or		nil			or		RASPI		or	OSX		},
	{"lua_lash",		WINDOWS		or		NIX		or		EMCC	or		nil			or		nil			or	OSX		},
	{"lua_win_windows",	WINDOWS		or		nil		or		nil		or		nil			or		nil			or	nil		},
	{"lua_win_linux",	nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
	{"lua_win_nacl",	nil			or		nil		or		NACL	or		nil			or		nil			or	nil		},
	{"lua_win_emcc",	nil			or		nil		or		EMCC	or		nil			or		nil			or	nil		},
	{"lua_win_android",	nil			or		nil		or		nil		or		ANDROID		or		nil			or	nil		},
	{"lua_win_raspi",	nil			or		nil		or		nil		or		nil			or		RASPI		or	nil		},
	{"lua_win_osx",		nil			or		nil		or		nil		or		nil			or		nil			or	OSX		},
	{"lua_sdl2",	   (nil			or		NIX		or		EMCC	or		nil			or		nil			or	nil		)
																						and		(not LSB) 				},

--new lua bindings and libs (maybe be buggy unfinshed or removed at anytime)
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
	{"lib_openal",	   (WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	nil		)
																						and		(not PEPPER) 			},
	{"lib_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		or	OSX		},
	{"lib_pcre",		nil			or		NIX		or		nil		or		nil			or		nil			or	OSX		},

-- pepper.js hacks
	{"lib_pepperjs",	nil			or		nil		or		nil		or		nil			or		PEPPER		or	nil		},

-- dont think building this is a good idea?
--	{"lib_angle",		nil			or		nil		or		nil		or		nil			or		nil			or	nil		},

-- old/broken and no longer supported but will probably still build
--	{"lua_speak",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	OSX		},
--	{"lua_lanes",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		or	OSX		},
--	{"lua_sec",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
--	{"lib_openssl",		nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},

-- need to lazy link to .so before we can add these back in otherwise they force unwanted dependencies
--	{"lua_hid",			nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
--	{"lib_hidapi",		nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},

-- the output executables
	{"exe_gamecake",	WINDOWS		or		NIX		or		WEB		or		ANDROID		or		RASPI		or	OSX		},
	{"exe_pagecake",	nil			or		NIX		or		nil		or		nil			or		nil			or	nil		},
}

------------------------------------------------------------------------
-- include sub projects depending on above build tests or you could choose
-- to set all_includes before including this file to choose your own config
------------------------------------------------------------------------

for i,v in ipairs(all_includes) do
	if v[2] then
		include(v[1])
	end
end




