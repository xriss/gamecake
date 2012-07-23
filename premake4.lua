
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

newplatform {
    name = "raspi",
    description = "raspi",
    gcc = {
        cc = "gcc",
        cxx = "g++",
        cppflags = "",
    }
}
newgcctoolchain {
    name = "raspi",
    description = "raspi",
--    prefix = "arm-bcm2708-linux-gnueabi-",
    prefix = "arm-raspi-linux-gnueabi-",
    cppflags = "",
}

newplatform {
    name = "android",
    description = "android",
    gcc = {
        cc = "gcc",
        cxx = "g++",
        cppflags = "",
    }
}
newgcctoolchain {
    name = "android",
    description = "android",
    prefix = "arm-linux-androideabi-",
    cppflags = "",
}

newplatform {
    name = "nacl",
    description = "nacl",
    gcc = {
        cc = "gcc",
        cxx = "g++",
        cppflags = "",
    }
}
newgcctoolchain {
    name = "nacl",
    description = "nacl",
    prefix = "i686-nacl-",
    cppflags = "",
}


newplatform {
    name = "mingw",
    description = "mingw",
    gcc = {
        cc = "gcc",
        cxx = "g++",
        cppflags = "",
    }
}
newgcctoolchain {
    name = "mingw",
    description = "mingw",
    prefix = "i586-mingw32msvc-",
    cppflags = "",
}


------------------------------------------------------------------------
-- work out what we should be building for
------------------------------------------------------------------------

solution("wetlua")

-- work out build type and set flags
NACL=false
ANDROID=false
WINDOWS=false
MINGW=false
NIX=false

if _ARGS[1]=="raspi" then
	TARGET="RASPI"
	RASPI=true
elseif _ARGS[1]=="nacl" then
	TARGET="NACL"
	NACL=true
elseif _ARGS[1]=="android" then
	TARGET="ANDROID"
	ANDROID=true
elseif _ARGS[1]=="mingw" then
	TARGET="WINDOWS"
	WINDOWS=true
	MINGW=true
elseif os.get() == "windows" then
	TARGET="WINDOWS"
	WINDOWS=true
else
	TARGET="NIX"
	NIX=true
end

if _ARGS[1] then
print(_ARGS[1].." == "..TARGET)
end

if NACL then

	local naclsdk=path.getabsolute("../sdks/naclsdk/pepper_15")

	platforms { "nacl" } --hax
	
	defines "NACL"
		
	buildoptions{"-m32"}
	
elseif RASPI then

	local raspisdk=path.getabsolute("../sdks/raspi")
	includedirs { raspisdk.."/firmware/hardfp/opt/vc/include" }
	libdirs { raspisdk.."/firmware/hardfp/opt/vc/lib" }

	platforms { "raspi" } --hax

	defines "RASPI"

	defines("LUA_USE_POSIX")
	
--	buildoptions{"-march=armv6zk -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s" }



elseif ANDROID then

	local androidsdk=path.getabsolute("../sdks/android-sdk")

	platforms { "android" } --hax

	defines "ANDROID"

	defines("LUA_USE_POSIX")
	
	buildoptions{ "-mthumb" }

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
	
end


if not BUILD_DIR_BASE then

	BUILD_DIR_BASE="build-"..(_ACTION or "")
end

if not BUILD_DIR then

	BUILD_DIR=BUILD_DIR_BASE
	
	if NACL then BUILD_DIR=BUILD_DIR_BASE.."-nacl" end
	if ANDROID then BUILD_DIR=BUILD_DIR_BASE.."-android" end
	if MINGW then BUILD_DIR=BUILD_DIR_BASE.."-mingw" end
	if RASPI then BUILD_DIR=BUILD_DIR_BASE.."-raspi" end

end

location( BUILD_DIR )


	

configurations { "Debug", "Release" }

includedirs { "lib_lua/src" }


EXE_OUT_DIR=path.getabsolute("bin/exe")
DBG_OUT_DIR=path.getabsolute("bin/dbg")

-- these are application specific and need proper paths so will get overidden
AND_LIB_DIR=AND_LIB_DIR or path.getabsolute("android")
AND_OUT_DIR=AND_OUT_DIR or path.getabsolute("android/libs/armeabi")

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

if NIX then -- luajit is working for these builds

	LIB_LUA="lib_luajit"
	defines( "LIB_LUAJIT" )
	
end

all_includes=all_includes or {
	{"lua_pack",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_zip",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_zlib",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_freetype",	WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_bit",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
--	{"lua_box2d",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_ogg",			WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_al",			nil			or		NIX		or		nil		or		ANDROID		or		nil			},
	{"lua_gl",			WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_gles",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_grd",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_grdmap",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_sod",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_speak",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_lash",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_lfs",			WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_socket",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
--	{"lua_fenestra",	WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lua_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_lanes",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lua_posix",		nil			or		NIX		or		nil		or		nil			or		RASPI		},
	{"lua_win",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lua_win_windows",	WINDOWS		or		nil		or		nil		or		nil			or		nil			},
	{"lua_win_linux",	nil			or		NIX		or		nil		or		nil			or		nil			},
	{"lua_win_nacl",	nil			or		nil		or		NACL	or		nil			or		nil			},
	{"lua_win_android",	nil			or		nil		or		nil		or		ANDROID		or		nil			},
	{"lua_win_raspi",	nil			or		nil		or		nil		or		nil			or		RASPI		},
	{AND_LIB_DIR,		nil			or		nil		or		nil		or		ANDROID		or		nil			},
	{LIB_LUA,			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_zzip",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_png",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_jpeg",		WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_gif",			WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lib_z",			WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_freetype",	WINDOWS		or		NIX		or		NACL	or		ANDROID		or		RASPI		},
	{"lib_sqlite",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		RASPI		},
	{"lib_pcre",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
--	{"lib_ogg",			WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
--	{"lib_vorbis",		WINDOWS		or		NIX		or		nil		or		nil			or		nil			},
	{"lib_openal",		WINDOWS		or		NIX		or		nil		or		ANDROID		or		nil			},
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




