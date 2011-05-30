
solution("wetlua")

-- build using NACL?
NACL=false
if _ARGS[1]=="nacl" then
NACL=true
end


BUILD_DIR="build-"..(_ACTION or "")
if NACL then BUILD_DIR=BUILD_DIR.."-nacl" end
location( BUILD_DIR )

configurations { "Debug", "Release" }

includedirs { "lua/src" }


EXE_OUT_DIR=path.getabsolute("bin/exe")
DBG_OUT_DIR=path.getabsolute("bin/dbg")

ALL_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj")
EXE_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Release")
DBG_OBJ_DIR=path.getabsolute(BUILD_DIR.."/obj/Debug")


if NACL then

	defines "NACL"
	
elseif os.get() == "windows" then

	defines "WIN32"
	defines "_CRT_SECURE_NO_WARNINGS"
--	defines	"LUA_BUILD_AS_DLL"

else -- nix

	defines "X11"
	defines	"LUA_USE_DLOPEN"
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
		targetdir(DBG_OBJ_DIR)

		configuration {"Release"}
		targetdir(EXE_OBJ_DIR)
		return
	end

dir=dir or ""

	if name then
		targetprefix ("")
		targetname (name)
	end

	configuration {"Debug"}
	targetdir(DBG_OUT_DIR..dir)

	configuration {"Release"}
	targetdir(EXE_OUT_DIR..dir)
end

if NACL then

	include("lua_nacl")

-- we might static link with all the above libs
	include("lua")
	
else

	include("lua_bit")
	include("lua_box2d")
	include("lua_gl")
	include("lua_grd")
	include("lua_lanes")
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




