
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

-- work out build type and set flag
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

	platforms { "nacl" } --hax
	
	defines "NACL"
	
	local naclsdk=path.getabsolute("../sdks/naclsdk/pepper_15")
	
--	includedirs { naclsdk.."/toolchain/linux_x86/nacl/include" }

	buildoptions{"-m32"}
	
elseif ANDROID then

	platforms { "android" } --hax

	defines "ANDROID"

	defines("LUA_USE_POSIX")

	local androidsdk=path.getabsolute("../sdks/android-sdk")
	
--	includedirs { naclsdk.."/toolchain/linux_x86/nacl/include" }
	
	buildoptions{ "-mthumb" }
	
--	buildoptions{  "-nostdlib" ,"-fno-exceptions"}
--	buildoptions{ "-fno-rtti", "-Wa", "--noexecstack" }
--	linkoptions{ "-Wl","--no-undefined","-z","--error-unresolved-symbols", "--no-allow-shlib-undefined", "-Bsymbolic","-Bstatic"}
--	linkoptions{ "noexecstack" }


--	flags{"StaticRuntime"}

--[[
 0x00000001 (NEEDED)                     Shared library: [libGLESv1_CM.so]
 0x00000001 (NEEDED)                     Shared library: [libdl.so]
 0x00000001 (NEEDED)                     Shared library: [liblog.so]
 0x00000001 (NEEDED)                     Shared library: [libstdc++.so]
 0x00000001 (NEEDED)                     Shared library: [libm.so]
 0x00000001 (NEEDED)                     Shared library: [libc.so]
]]


elseif WINDOWS then
	
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

elseif NIX then
	
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

ALL_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj")
EXE_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Release")
DBG_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Debug")

AND_OUT_DIR=path.getabsolute("lib_android/libs/armeabi")


lua_lib_names={}
lua_lib_loads={}

function SET_KIND(name,luaname,luafname)
	if name=="luamain" then

		kind("StaticLib")

	elseif name=="lua" then

		kind("StaticLib")
		lua_lib_names[#lua_lib_names+1]=project().name
		lua_lib_loads[#lua_lib_loads+1]={luaname,luafname}

	else
		kind(name)
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

	include("lib_lua")

--	include("lua_nacl")

-- we might static link with all the above libs
	include("lua_main")
	
elseif ANDROID then

	include("lib_lua")

--	include("lua_android")

--	include("lua_bit")

	
-- we might static link with all the above libs
	include("lua_main")
	
else

	include("lib_lua")
	include("lib_z")
	
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


-- lanes has trouble with multipple CPUs for some reason?
-- or my code is not really thread safe?
-- tis bugs that need to be fixed
-- just disabled for now as a quick hack fix

--	include("lua_lanes")


-- security is always a clusterfuck, need openssl workingcross platform
-- and its not building right now, so disable for now

--	include("lua_sec")


-- not using these so avoid the dependencies
-- should probably setup sql as a dll since its useless on consoles etc
-- as there is no full source to build, well there is source...
-- but fuck me if I can build the shits

--	include("lua_sql")


	if NIX then

		include("lua_posix")
--		include("lib_sx")
		
	end
	
-- we might static link with all the above libs here, producing a final exe
	include("lua_main")

end




