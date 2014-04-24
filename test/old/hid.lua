#!../../bin/exe/lua

-- setup some default search paths,
require("apps").default_paths()

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local wwin=require("wetgenes.win")

local posix=require("posix")

--find device in /proc/bus/input/devices  ?

local fp=assert(posix.open("/dev/input/event12", posix.O_NONBLOCK + posix.O_RDONLY ))

local hist={}
local deadzone=24

while true do
	wwin.sleep(0.0001)

--	local pkt=posix.read(fp,16) -- 32bit
	local pkt=posix.read(fp,24) -- 64bit hax

--print(pkt)
	
	if pkt then
	
		local Isecs,Imicros,Itype,Icode,Ivalue
		if #pkt==24 then
			Isecs=pack.read(pkt,"u32",0)
			Imicros=pack.read(pkt,"u32",8)
			Itype=pack.read(pkt,"u16",16)
			Icode=pack.read(pkt,"u16",18)
			Ivalue=pack.read(pkt,"u16",20)
		elseif #pkt==16 then
			Isecs=pack.read(pkt,"u32",0)
			Imicros=pack.read(pkt,"u32",4)
			Itype=pack.read(pkt,"u16",8)
			Icode=pack.read(pkt,"u16",10)
			Ivalue=pack.read(pkt,"u16",12)
		end
		
		if Itype==3 then
			if Ivalue>128-deadzone and Ivalue<128+deadzone then Ivalue=128 end
		end
		
		local key=Itype..":"..Icode
		local v=hist[key]
		if v and (v ~= Ivalue) then v=false end

		if not v then -- *new* values only, ignore most junk packets
			hist[key]=Ivalue
			
			local Itime=Isecs+(Imicros/1000000)		
			print(Itime,Itype,Icode,Ivalue)
		end
	end
	
	
--[[

struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};

]]


end

