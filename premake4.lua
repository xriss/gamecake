
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

if _ARGS[1]=="nacl" then
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



if NACL then

	local naclsdk=path.getabsolute("../sdks/naclsdk/pepper_15")

	platforms { "nacl" } --hax
	
	defines "NACL"
		
	buildoptions{"-m32"}
	
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


BUILD_DIR="build-"..(_ACTION or "")

if NACL then BUILD_DIR=BUILD_DIR.."-nacl" end
if ANDROID then BUILD_DIR=BUILD_DIR.."-android" end
if MINGW then BUILD_DIR=BUILD_DIR.."-mingw" end

location( BUILD_DIR )

configurations { "Debug", "Release" }

includedirs { "lib_lua/src" }


EXE_OUT_DIR=path.getabsolute("bin/exe")
DBG_OUT_DIR=path.getabsolute("bin/dbg")
AND_OUT_DIR=path.getabsolute("lib_android/libs/armeabi")

ALL_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj")
EXE_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Release")
DBG_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Debug")



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
	
		configuration {"Debug"}
		targetdir(DBG_OBJ_DIR)

		configuration {"Release"}
		targetdir(EXE_OBJ_DIR)
	end
	
end


-- need to clean this up and merge SET_KIND and SET_TARGET into one function...
--[[
function SET_KIND(kindof,luaname,luafname)
	if kindof=="lua" then -- special laulib kind that keeps a list of libs

		kind("StaticLib")
		lua_lib_names[#lua_lib_names+1]=project().name
		lua_lib_loads[#lua_lib_loads+1]={luaname,luafname}

	else
		kind(kindof)
	end
end

function SET_TARGET(dir,name,force)

	if not force then
		kind("StaticLib")
		configuration {"Debug"}
		flags {"Symbols"} -- blue debug needs symbols badly
		targetdir(DBG_OBJ_DIR)

		configuration {"Release"}
		flags {"Optimize"}
		targetdir(EXE_OBJ_DIR)
		return
	end

dir=dir or ""

	if name then
		targetprefix ("")
		targetname (name)
	end

	if ANDROID then
	
		configuration {"Debug"}
		flags {"Symbols"} -- blue debug needs symbols badly
		targetdir(AND_OUT_DIR..dir)

		configuration {"Release"}
		flags {"Optimize"}
		targetdir(AND_OUT_DIR..dir)
		
	elseif NACL then
	
		configuration {"Debug"}
		flags {"Symbols"} -- blue debug needs symbols badly
		targetdir(DBG_OUT_DIR..dir)

	else

		configuration {"Debug"}
		flags {"Symbols"} -- blue debug needs symbols badly
		targetdir(DBG_OUT_DIR..dir)

		configuration {"Release"}
		flags {"Optimize"}
		targetdir(EXE_OUT_DIR..dir)

	end
end
]]

------------------------------------------------------------------------
-- include sub projects depending on build
------------------------------------------------------------------------

if NACL then

LIB_LUA="lib_lua"
	
--	include("lua_zip")
	include("lua_zlib")
	include("lua_freetype")
	include("lua_bit")
--	include("lua_box2d")
	include("lua_gl")
--	include("lua_grd")
--	include("lua_lash")
--	include("lua_lfs")
--	include("lua_sqlite")
--	include("lua_socket")
--	include("lua_fenestra")

	include(LIB_LUA)
	include("lib_zzip")
--	include("lib_png")
	include("lib_z")
--	include("lib_sqlite")

	include("lib_nacl")

-- we probably static link with all the above libs so this should go last
	include("lua")
	
elseif ANDROID then

LIB_LUA="lib_lua"
	
--	include("lua_zip")
	include("lua_zlib")
	include("lua_freetype")
	include("lua_bit")
--	include("lua_box2d")
	include("lua_gl")
--	include("lua_grd")
--	include("lua_lash")
	include("lua_lfs")
--	include("lua_sqlite")
--	include("lua_socket")
--	include("lua_fenestra")

--	include("lua_android")

--	include("lua_bit")

	include(LIB_LUA)
	include("lib_zzip")
	include("lib_png")
	include("lib_z")
	include("lib_sqlite")
	include("lib_android")
	
-- we probably static link with all the above libs so this should go last
	include("lua")
	
else -- windows or linux

-- a define to choose vanilla lua or luajit...
if WINDOWS then
	LIB_LUA="lib_lua"
else
	LIB_LUA="lib_luajit"
end
	
	include("lua_pack")
	include("lua_zip")
	include("lua_zlib")
	include("lua_freetype")
	include("lua_bit")
	include("lua_box2d")
	include("lua_ogg")
	include("lua_al")
	include("lua_gl")
	include("lua_grd")
	include("lua_grdmap")
	include("lua_sod")
	include("lua_speak")
	include("lua_lash")
	include("lua_lfs")
	include("lua_socket")
	include("lua_fenestra")
	include("lua_sqlite")
	include("lua_lanes")

-- security is always a clusterfuck, need openssl working cross platform
-- and its not building right now, so disabled

--	include("lua_sec")


-- not using this to avoid the dependencies
-- should probably setup sql as a dll since its useless on consoles etc
-- as there is no full source to build, well there is source...
-- but fuck me if I can build the shits

--	include("lua_sql")


	if NIX then

		include("lua_posix")
--		include("lib_sx")
		
	end
	
	include(LIB_LUA)
	include("lib_zzip")
	include("lib_png")
	include("lib_jpeg")
	include("lib_z")
	include("lib_sqlite")
	include("lib_pcre")
	
	include("lib_ogg")
	include("lib_vorbis")
	
	include("lib_openal")

-- we probably static link with all the above libs so this should go last
	include("lua")

-- build webserver
if not WINDOWS then
	include("nginx")
end

end




