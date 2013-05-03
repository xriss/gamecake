#!/usr/local/bin/gamecake

local bit=require("bit")

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




print(wstr.dump(hid))

print(wstr.dump(hid.enumerate()))

local dev = hid.open_path("/dev/hidraw0") -- first dev
print(dev)
if dev then hid.close(dev) end

local dev = hid.open_id(0x2833, 0x0001, nil)	-- the oculus rift
print(dev)


print( "manufacturer" , hid.get_string(dev,"manufacturer") )
print( "product" , hid.get_string(dev,"product") )
print( "serial_number" , hid.get_string(dev,"serial_number") )

print( "error" , hid.error(dev) )

--if dev then hid.close(dev) end

hid.set_nonblocking(dev,1)

assert(hid.send_feature_report(dev, string.char(0x2,0xa0, 0x0a)..string.rep("\0", 14)))

local buf = assert(hid.get_feature_report(dev,0x6, 16))
bdump(buf)


-- Read a data
while true do
	local buf = hid.read(dev,62)
--	printf("Data\n   ");

	if buf then

		bdump(buf)
		
		local head=pack.load(buf,{
			"u8","count",
			"u16","time",
			"u16","last",
			"u16","temp",
		},0)

		local tail=pack.load(buf,{
			"u16","magx",
			"u16","magy",
			"u16","magz",
		},56)
		
		local function debit(base)
--print(base,8,#buf)	
			local b={}
			for i=1,8 do b[i]=pack.read(buf,"u8",base+i-1) end
			local t={}
--[[			
    t[1] = (buffer[0] << 13) | (buffer[1] << 5) | ((buffer[2] & 0xF8) >> 3);
    t[2] = ((buffer[2] & 0x07) << 18) | (buffer[3] << 10) | (buffer[4] << 2) | ((buffer[5] & 0xC0) >> 6);
    t[3] = ((buffer[5] & 0x3F) << 15) | (buffer[6] << 7) | (buffer[7] >> 1);
]]
			return t
		end
		
		local data={}
		for i=1,1 do
			data[ (i-1)*2 + 0]= debit( 8 +     16*(i-1) )
			data[ (i-1)*2 + 1]= debit( 8 + 8 + 16*(i-1) )
		end
		
		
		print( head.count, head.time, head.last, head.temp , tail.magx, tail.magy, tail.magz )
	
	end
	
end

os.exit(0)

