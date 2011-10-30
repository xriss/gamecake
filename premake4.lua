
solution("wetlua")

-- build using NACL?
NACL=false
if _ARGS[1]=="nacl" then
NACL=true
end

-- build using ANDROID?
ANDROID=false
if _ARGS[1]=="android" then
ANDROID=true
end

BUILD_DIR="build-"..(_ACTION or "")

if NACL then BUILD_DIR=BUILD_DIR.."-nacl" end
if ANDROID then BUILD_DIR=BUILD_DIR.."-android" end

location( BUILD_DIR )

configurations { "Debug", "Release" }

includedirs { "lua/src" }


EXE_OUT_DIR=path.getabsolute("bin/exe")
DBG_OUT_DIR=path.getabsolute("bin/dbg")

ALL_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj")
EXE_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Release")
DBG_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Debug")

AND_OUT_DIR=path.getabsolute("android/libs/armeabi")


if NACL then

	defines "NACL"
	
	local naclsdk=os.getenv("naclsdk") or "../sdks/naclsdk"
	
	includedirs { naclsdk.."/toolchain/linux_x86/nacl/include" }
	
elseif ANDROID then

	defines "ANDROID"
	
	local androidsdk=os.getenv("androidsdk") or "../sdks/android-sdk"
	
--	includedirs { naclsdk.."/toolchain/linux_x86/nacl/include" }
	
elseif os.get() == "windows" then
	WINDOWS=true
	
	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

else -- nix
	NIX=true
	
	defines "X11"
--	defines	"LUA_USE_DLOPEN"
	linkoptions "-Wl,-rpath=\\$$ORIGIN:."
	
end

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

	else

	configuration {"Debug"}
	flags {"Symbols"} -- blue debug needs symbols badly
	targetdir(DBG_OUT_DIR..dir)

	configuration {"Release"}
	flags {"Optimize"}
	targetdir(EXE_OUT_DIR..dir)

	end
end

if NACL then

	include("lua_nacl")

-- we might static link with all the above libs
	include("lua")
	
elseif ANDROID then

	include("lua_android")

	include("lua_bit")

-- we might static link with all the above libs
	include("lua")
	
else

	include("lib_z")
	
	include("lua_zip")
	include("lua_zlib")
	include("lua_freetype")
	include("lua_bit")
	include("lua_box2d")
	include("lua_gl")
	include("lua_grd")
--	include("lua_lanes")
	include("lua_lash")
	include("lua_lfs")
	include("lua_socket")
	--include("lua_sql")
	--include("lua_sec")
	include("lua_fenestra")

	if os.get() == "windows" then

	else -- nix

		include("lua_posix")
		
	end
	
-- we might static link with all the above libs
	include("lua")

end




