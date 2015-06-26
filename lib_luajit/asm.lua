#!/usr/bin/env gamecake


local function build(mode)

	print("")
	print("################")
	print("## BUILD MODE ## = "..mode)
	print("################")
	print("")

	os.execute("mkdir -p libs/"..mode)
	os.execute("mkdir -p asm/"..mode)

	os.execute("make clean ")

	if mode=="native" then -- these are local hacks for when I bump the luajit version

		os.execute("make amalg HOST_LUA=luajit CFLAGS=\" -march=native \" ")
		
	elseif mode=="x86" then -- these are local hacks for when I bump the luajit version

		os.execute("make amalg HOST_LUA=luajit CFLAGS=\" -m32 -msse -msse2 \" LDFLAGS=\" -m32 \" ")
		
	elseif mode=="x64" then

		os.execute("make amalg HOST_LUA=luajit CFLAGS=\" -m64  \" LDFLAGS=\" -m64 \" ")
		
	elseif mode=="lsb32" then -- these are local hacks for when I bump the luajit version

		os.execute([[make amalg TARGET_STATIC_CC="lsbcc"
DYNAMIC_CC="lsbcc -fPIC" CFLAGS="-m32 -msse -msse2" ]])
		
	elseif mode=="lsb64" then

		os.execute([[make amalg STATIC_CC="lsbcc"
DYNAMIC_CC="lsbcc -fPIC" CFLAGS="-m64" ]])

	elseif mode=="arm" then -- android, make sure the flags are shared with the main build

		os.execute("make amalg HOST_LUA=luajit TARGET_CFLAGS=\" -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3  \" HOST_CC=\"gcc -m32 \" CROSS=/home/kriss/hg/sdks/android-9-arm/bin/arm-linux-androideabi- ")

	elseif mode=="armhf" then -- raspi, make sure the flags are shared with the main build

		os.execute("make amalg HOST_LUA=luajit  TARGET_CFLAGS=\" -mfpu=vfp -mfloat-abi=hard -marm -mcpu=arm1176jzf-s -mtune=arm1176jzf-s \" HOST_CC=\"gcc -m32 \" CROSS=/home/kriss/hg/sdks/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- ")

	elseif mode=="win32" then -- windows 32bit

		os.execute("make amalg HOST_LUA=luajit  CFLAGS=\" -m32 -msse -msse2 \" LDFLAGS=\" -m32 \" CROSS=i586-mingw32msvc- TARGET_SYS=Windows ")

	elseif mode=="osx" then -- osx 32bit, this must be run on the mac...

		os.execute("make amalg CFLAGS=\" -m32 -msse -msse2 \" LDFLAGS=\" -m32 \" ")

	end

	for i,v in ipairs{
		"lj_bcdef.h",
		"lj_ffdef.h",
		"lj_folddef.h",
		"lj_libdef.h",
		"lj_recdef.h",
		"lj_vm.s",			-- this is missing on windows build... 
	} do
		os.execute("cp src/"..v.." asm/"..mode.."/"..v)
	end
	os.execute("cp src/jit/vmdef.lua asm/"..mode.."/vmdef.lua")


	if mode=="win32" then
		os.execute("cp src/luajit.o libs/"..mode.."/")		-- windows .o
	else
		os.execute("cp src/libluajit.a libs/"..mode.."/")	-- everyone else is .a
	end

	os.execute("make clean ")

end


local args={...}

if args[1]=="all" then

	build("x86")

	build("arm")

	build("armhf")

	build("x64")

	build("win32")

--	build("lsb32")
--	build("lsb64")

elseif args[1] then

	build(args[1])
	
else

	build("native")

end
