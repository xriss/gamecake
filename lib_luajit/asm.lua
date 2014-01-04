#!/usr/bin/env gamecake



--[[

when bumping the source
make sure lib_init.c has the following code patch

--

LUALIB_API void luaL_openlibs(lua_State *L)
{
  const luaL_Reg *lib;
  for (lib = lj_lib_load; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
#ifdef LUA_PRELOADLIBS
        LUA_PRELOADLIBS(L);
#endif
  luaL_findtable(L, LUA_REGISTRYINDEX, "_PRELOAD",
		 sizeof(lj_lib_preload)/sizeof(lj_lib_preload[0])-1);
  for (lib = lj_lib_preload; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_setfield(L, -2, lib->name);
  }
  lua_pop(L, 1);
}


]]

local function build(mode)

	os.execute("mkdir libs")
	os.execute("mkdir libs/"..mode)

	os.execute("mkdir asm")
	os.execute("mkdir asm/"..mode)

	os.execute("make clean ")

	if mode=="x86" then -- these are local hacks for when I bump the luajit version

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs \" HOST_CC=\"gcc -m32 \" ")
		
	elseif mode=="x64" then

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs \" HOST_CC=\"gcc -m64  \" ")
		
	elseif mode=="arm" then -- android, make sure the flags are shared with the main build

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3  \" HOST_CC=\"gcc -m32 \" CROSS=/home/kriss/hg/sdks/android-9-arm/bin/arm-linux-androideabi- ")

	elseif mode=="armhf" then -- raspi, make sure the flags are shared with the main build

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s \" HOST_CC=\"gcc -m32 \" CROSS=/home/kriss/hg/sdks/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- ")

	elseif mode=="win32" then -- windows 32bit

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs \" HOST_CC=\"gcc -m32 \" CROSS=i586-mingw32msvc- TARGET_SYS=Windows ")

	elseif mode=="osx" then -- osx 32bit, this must be run on the mac...

		os.execute("make amalg TARGET_CFLAGS=\" -DLUA_PRELOADLIBS=lua_preloadlibs \" CFLAGS=\" -m32 \" LDFLAGS=\" -m32 \" ")

	end

	for i,v in ipairs{
		"lj_bcdef.h",
		"lj_ffdef.h",
		"lj_folddef.h",
		"lj_libdef.h",
		"lj_recdef.h",
		"lj_vm.s",
	} do
		os.execute("cp src/"..v.." asm/"..mode.."/"..v)
	end
	os.execute("cp src/jit/vmdef.lua asm/"..mode.."/vmdef.lua")

	os.execute("cp src/libluajit.a libs/"..mode.."/")
	os.execute("cp src/luajit.o libs/"..mode.."/")		-- windows only


	os.execute("make clean ")

end


local args={...}

if args[1] then

	build(args[1])

else

	build("x86")

	build("arm")

	build("armhf")

	build("x64")

	build("win32")

end
