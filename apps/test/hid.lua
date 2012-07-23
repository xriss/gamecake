#!../../bin/exe/lua

-- setup some default search paths,
require("apps").default_paths()

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")


--find device in /proc/bus/input/devices  ?

local fp=io.open("/dev/input/event6","r")

while true do

	local pkt=fp:read(16)
	
	if pkt then
	
		local tab=pack.load(pkt,{"s32","secs","s32","msecs","u16","type","u16","code","u32","value"})
		
		print(wstr.dump(tab))
	
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

