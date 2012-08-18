-- setup some default search paths,
require("apps").default_paths()

-- grab some libs
--local pack=require("wetgenes.pack")             -- raw memory packing
local wstr=require("wetgenes.string")   -- string helpers
--local tardis=require("wetgenes.tardis") -- matrix/vector math

-- a window/screen handler this works in windows/linux/nacl/android/raspi

local wwin=require("wetgenes.win")

print( wstr.dump(wwin.screen()) )

local win=wwin.create({})

print(wstr.dump(win))

for i=1,1000 do
--print(i)
	win:sleep(0.02)
	local m=win:msg()
	if m then
		print(wstr.dump(m))
	end
end

