#!/usr/local/bin/gamecake

-- setup some default search paths,
require("apps").default_paths()

-- grab some libs
--local pack=require("wetgenes.pack")             -- raw memory packing
local wstr=require("wetgenes.string")   -- string helpers
--local tardis=require("wetgenes.tardis") -- matrix/vector math


local gl=assert(require("gles").gles1) -- something that works like gles1 please

-- a window/screen handler this works in windows/linux/nacl/android/raspi
local wwin=require("wetgenes.win")
local win=wwin.create({})

-- select a standard gles context
win:context({})

print(wstr.dump(win))

for i=1,1000 do
--print(i)
	win:sleep(0.02)
	local m=win:msg()
	if m then
		print(wstr.dump(m))
	end

	
	win:info()
	gl.Viewport(0,0,win.width,win.height)

	gl.ClearColor(0,0,1/4,1/4)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	win:swap()

end

