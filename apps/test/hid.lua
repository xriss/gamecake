#!../../bin/exe/lua

-- setup some default search paths,
require("apps").default_paths()

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local wwin=require("wetgenes.win")

local posix=require("posix")

--find device in /proc/bus/input/devices  ?

local fp=assert(posix.open("/dev/input/event12", posix.O_NONBLOCK + posix.O_RDONLY ))

while true do
	wwin.sleep(0.0001)

	local pkt=posix.read(fp,16)
	
	if pkt then
	
		 local tab=pack.load(pkt,{"u32","secs","u32","micros","u16","type","u16","code","u32","value"})
		
		print(tab.type,tab.code,tab.value)
	
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

