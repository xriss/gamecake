#!/usr/local/bin/gamecake



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

	os.execute("make clean ")

	if mode=="x86" then -- these are local hacks for when I bump the luajit version

		os.execute("make HOST_CC=\"gcc -m32\" ")

	elseif mode=="arm" then -- android, make sure the flags are shared with the main build

		os.execute("make HOST_CC=\"gcc -m32\" CROSS=/home/kriss/hg/sdks/android-9-arm/bin/arm-linux-androideabi- TARGET_CFLAGS=\" -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3 \" ")

	elseif mode=="armhf" then -- raspi, make sure the flags are shared with the main build

		os.execute("make HOST_CC=\"gcc -m32\" CROSS=/home/kriss/hg/sdks/gcc/prefix/bin/arm-raspi-linux-gnueabi- TARGET_CFLAGS=\" -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s \" ")

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

	os.execute("make clean ")

end



build("x86")

build("arm")

build("armhf")


