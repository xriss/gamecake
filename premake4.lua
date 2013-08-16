
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
    prefix = "arm-raspi-linux-gnueabi-",
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
    name = "nacl",
    description = "nacl",
    prefix = "i686-nacl-",
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


------------------------------------------------------------------------
-- work out what we should be building for
------------------------------------------------------------------------

solution("gamecake")

-- work out build type and set flags
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
elseif t:sub(1,5)=="clang" then
	TARGET="NIX"
	CPU=t:sub(6)
	NIX=true
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

if NACL then

	local naclsdk=path.getabsolute("../sdks/naclsdk/pepper_25")

	platforms { "nacl" } --hax
	
	defines "NACL"
	
	if CPU=="32" then
	
		buildoptions{"-m32"}
		linkoptions{"-m32"}
		
	elseif CPU=="64" then
	
		buildoptions{"-m64"}
		linkoptions{"-m64"}
		
	end
	
elseif RASPI then

	local raspisdk=path.getabsolute("../sdks/raspi")
	local raspxsdk=path.getabsolute("../sdks/raspx")

	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include" }
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include/interface/vcos/pthreads"} -- bugfix?
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

	defines "_WIN32_WINNT=0x0500"
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

	if MINGW then
	
		platforms { "mingw" } --hax

		local w32api=path.getabsolute("../sdks/w32api")
		
		includedirs { w32api.."/include" }
		libdirs { w32api.."/lib" }
		
	end

elseif NIX then
	
	defines("LUA_USE_MKSTEMP") -- remove warning
	defines("LUA_USE_POPEN") -- we want to enable popen

	defines "X11"
--	defines	"LUA_USE_DLOPEN"
	linkoptions "-Wl,-rpath=\\$$ORIGIN:."

	if CLANG then
		platforms { "clang" } --hax
	end

	if CPU=="32" then
	
		buildoptions{"-m32"}
		linkoptions{"-m32"}
		
	elseif CPU=="64" then
	
		buildoptions{"-m64"}
		linkoptions{"-m64"}
		
	end
	
end






if not BUILD_DIR_BASE then

	local t=TARGET
	if CLANG then t="clang" end

	BUILD_DIR_BASE=("build-"..(_ACTION or "gmake").."-"..t.."-"..CPU):lower()
	
end

if not BUILD_DIR then

	BUILD_DIR=BUILD_DIR_BASE

end

location( BUILD_DIR )


	

configurations { "Debug", "Release" }

includedirs { "lib_lua/src" }


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
	flags {"Symbols"} -- keep symbols to help with release only crashes
	
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

if RASPI or ANDROID then -- luajit is working for these builds

	LIB_LUA="lib_luajit"
	defines( "LIB_LUAJIT" )
	
elseif NIX then

	if CPU=="32" then
		LIB_LUA="lib_luajit"
		defines( "LIB_LUAJIT" )
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
	
elseif NACL then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }

elseif WINDOWS then -- need windows GL hacks

	includedirs { "lua_gles/code" }
	defines{ "LUA_GLES_GLES2" }
	if GCC then
		defines{ "INCLUDE_GLES_GL=\\\"GL3/gl3w.h\\\"" }
	else
		defines{ "INCLUDE_GLES_GL=\"GL3/gl3w.h\"" }
	end
	
else -- use GL 

	includedirs { "lua_gles/code" }
	defines{ "LUA_GLES_GLES2" }

	defines{ "INCLUDE_GLES_GL=\\\"GL3/gl3w.h\\\"" }
--	defines{ "INCLUDE_GLES_GL=\\\"GL/gl.h\\\"" }

end



all_includes=all_includes or {
	{"lua_profiler",	WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		},
	{"lua_pack",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_zip",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_zlib",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_freetype",	WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_bit",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_ogg",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
--	{"lua_hid",			nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lua_al",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_tardis",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_gles",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_grd",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_grdmap",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_sod",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_speak",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_lash",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
--	{"lua_sec",			nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lua_lfs",			WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_socket",		WINDOWS		or		NIX		or		nil		or		nil			or		RASPI		},
	{"lua_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_lanes",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_posix",		nil			or		NIX		or		nil		or		nil			or		RASPI		},
	{"lua_gamecake",	WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_win",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_win_windows",	WINDOWS		or		nil		or		nil		or		nil			or		nil			},
	{"lua_win_linux",	nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lua_win_nacl",	nil			or		nil		or		NACL	or		nil			or		nil			},
	{"lua_win_android",	nil			or		nil		or		nil		or		ANDROID		or		nil			},
	{"lua_win_raspi",	nil			or		nil		or		nil		or		nil			or		RASPI		},
	{LIB_LUA,			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
--	{"lib_openssl",		nil			or		NIX		or		nil		or		nil			or		nil			},
--	{"lib_hidapi",		nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lib_zzip",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_png",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_jpeg",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_gif",			WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lib_z",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_freetype",	WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lib_pcre",		nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lib_vorbis",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_ogg",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_openal",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua",				WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"nginx",			nil			or		NIX		or		nil		or		nil			or		nil			},
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




