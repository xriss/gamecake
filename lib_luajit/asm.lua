#!/usr/local/bin/gamecake

local wbake=require("wetgenes.bake")
local wstr=require("wetgenes.string")

local function cleandir()
	os.execute("rm src/host/*.o")
	os.execute("rm src/*.o")

for i,v in ipairs{
"src/host/buildvm",
"src/host/buildvm_arch.h",
"src/host/minilua",
"src/jit/vmdef.lua",
"src/libluajit.a",
"src/libluajit.so",
"src/lj_bcdef.h",
"src/lj_ffdef.h",
"src/lj_folddef.h",
"src/lj_libdef.h",
"src/lj_recdef.h",
"src/lj_vm.s",
"src/luajit",
} do

	os.execute("rm "..v)
end
	
end

local function build(mode)

	if mode=="x86" then -- these are local hacks for when I bump the luajit version

		os.execute("make")

	elseif mode=="arm" then

		os.execute("make CROSS=/home/kriss/hg/sdks/android-9-arm/bin/arm-linux-androideabi-")

	elseif mode=="armhf" then

		os.execute("make CROSS=/home/kriss/hg/sdks/gcc/prefix/bin/arm-raspi-linux-gnueabi-")

	end


	cleandir()

end




build("x86")


