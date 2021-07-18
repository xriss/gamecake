#!/usr/local/bin/gamecake

local bit=require("bit")

local bnot = bit.bnot
local band, bor, bxor = bit.band, bit.bor, bit.bxor
local lshift, rshift, rol = bit.lshift, bit.rshift, bit.rol


local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local hid=require("wetgenes.hid")


local bdump=function(buf)
	if buf then
		local t={}
		for i=1,#buf do t[#t+1]=(string.format("%02x", string.byte(buf, i))) end
		print(#buf,table.concat(t))
	end
end



--[[
print(wstr.dump(hid))

print(wstr.dump(hid.enumerate()))
]]


local dev = hid.open_id(0x14b7,0x0982, nil)	-- Game-Trak V1.3


print( "manufacturer" , hid.get_string(dev,"manufacturer") )
print( "product" , hid.get_string(dev,"product") )
print( "serial_number" , hid.get_string(dev,"serial_number") )


--print( "error" , hid.error(dev) )

--if dev then hid.close(dev) end

hid.set_nonblocking(dev,1)

--assert(hid.write(dev, string.char(0x02, 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00)))


--[[
for i=0,255 do
	print(i)
--	hid.send_feature_report(dev, string.char(i,0,0,0,0,0,0,0))

	local buf = hid.get_feature_report(dev,i, 16)
	if buf then bdump(buf) end
end
print("NEXT")
]]

-- Read a data
while true do
	local buf = hid.read(dev,16)
	if buf then
		print( buf and #buf or 0 )
		bdump(buf)
	end
end

os.exit(0)

