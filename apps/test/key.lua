#!../../bin/exe/lua

-- setup some default search paths,
require("apps").default_paths()

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local bit = require("bit")
local posix=require("posix")

--find device in /proc/bus/input/devices  ?

local fp=assert(posix.open("/dev/input/event3", bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) ))

print("reading ",fp)

while true do

	local pkt=(posix.read(fp,16))
	
	if pkt then
	
		local tab=pack.load(pkt,{"u32","secs","u32","micro","s16","value","u8","type","u8","number"})
		
--if tab.type==57 then
		print(wstr.dump(tab))
--end
	
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

