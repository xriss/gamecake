
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

------------------------------------------------------------------------
-- work out what we should be building for
------------------------------------------------------------------------

solution("wetlua")

-- work out build type and set flags
NACL=false
ANDROID=false
WINDOWS=false
NIX=false

if _ARGS[1]=="nacl" then
	TARGET="NACL"
	NACL=true
elseif _ARGS[1]=="android" then
	TARGET="ANDROID"
	ANDROID=true
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
	
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

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

-- need to clean this up and merge SET_KIND and SET_TARGET into one function...
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


------------------------------------------------------------------------
-- include sub projects depending on build
------------------------------------------------------------------------

if NACL then

	
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

	include("lib_lua")
	include("lib_z")
--	include("lib_sqlite")

	include("lib_nacl")

-- we probably static link with all the above libs so this should go last
	include("lua")
	
elseif ANDROID then

	
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

	include("lib_lua")
	include("lib_z")
	include("lib_sqlite")
	include("lib_android")
	
-- we probably static link with all the above libs so this should go last
	include("lua")
	
else -- windows or linux

	
	include("lua_zip")
	include("lua_zlib")
	include("lua_freetype")
	include("lua_bit")
	include("lua_box2d")
	include("lua_gl")
	include("lua_grd")
	include("lua_lash")
	include("lua_lfs")
	include("lua_socket")
	include("lua_fenestra")
	include("lua_sqlite")


-- lanes has trouble with multipple CPUs for some reason?
-- or my code is not really thread safe?
-- tis bugs that need to be fixed
-- just disabled for now as a quick hack fix

--	include("lua_lanes")


-- security is always a clusterfuck, need openssl working cross platform
-- and its not building right now, so disable for now

--	include("lua_sec")


-- not using these to avoid the dependencies
-- should probably setup sql as a dll since its useless on consoles etc
-- as there is no full source to build, well there is source...
-- but fuck me if I can build the shits

--	include("lua_sql")


	if NIX then

		include("lua_posix")
--		include("lib_sx")
		
	end
	
	include("lib_lua")
	include("lib_z")
	include("lib_sqlite")
	include("lib_pcre")

-- we probably static link with all the above libs so this should go last
	include("lua")

-- build webserver
	include("nginx")

end




