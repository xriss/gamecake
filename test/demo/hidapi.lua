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




print(wstr.dump(hid))

print(wstr.dump(hid.enumerate()))

local dev = hid.open_path("/dev/hidraw1") -- first dev
print(dev)
if dev then

print( "manufacturer" , hid.get_string(dev,"manufacturer") )
print( "product" , hid.get_string(dev,"product") )
print( "serial_number" , hid.get_string(dev,"serial_number") )


	hid.close(dev)

end

local dev = hid.open_id(0x2434,0x5303, nil)	-- the oculus rift
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

--		bdump(buf)
		
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
		
		local function sign31(n)
			if n>0x0fffff then return n-0x200000 end
			return n
		end

		local function debit(base)
			local b={} for i=1,8 do b[i]=pack.read(buf,"u8",base+i-1) end
			
			local t1=sign31( lshift(b[1],13) + lshift(b[2],5) + rshift(band(b[3],0xf8),3) )
			local t2=sign31( lshift(band(b[3],0x07),18) + lshift(b[4],10) + lshift(b[5],2) + rshift(band(b[6],0xc0),6) )
			local t3=sign31( lshift(band(b[6],0x3f),15) + lshift(b[7],7) + rshift(band(b[8],0xfe),1) )
--[[			
    t[1] = (buffer[0] << 13) | (buffer[1] << 5) | ((buffer[2] & 0xF8) >> 3);
    t[2] = ((buffer[2] & 0x07) << 18) | (buffer[3] << 10) | (buffer[4] << 2) | ((buffer[5] & 0xC0) >> 6);
    t[3] = ((buffer[5] & 0x3F) << 15) | (buffer[6] << 7) | (buffer[7] >> 1);
]]
			return t1,t2,t3
		end
		
		local data={}
		if head.count>2 then head.count=3 end
		for i=1,head.count do
			local d={}
			data[i]=d
			d.accx , d.accy , d.accz = debit( 8 +     16*(i-1) )
			d.gyrx , d.gyry , d.gyrz = debit( 8 + 8 + 16*(i-1) )
		end
		
		print( data[1].accx , data[1].accy , data[1].accz , ".", data[1].gyrx , data[1].gyry , data[1].gyrz , ".", 
		tail.magx, tail.magy, tail.magz , "-" ,  head.last*65536 + head.time, head.temp )
	
	end
	
end

os.exit(0)

