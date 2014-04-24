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

--	local pkt=posix.read(fp,16) -- 32bit
	local pkt=posix.read(fp,24) -- 64bit hax

--print(pkt)
	
	if pkt then
	
		local tab
		if #pkt==24 then
			tab=pack.load(pkt,{"u32","secs","u32","secs64","u32","micros","u32","micros64","u16","type","u16","code","u32","value"})
		elseif #pkt==16 then
			tab=pack.load(pkt,{"u32","secs","u32","micros","u16","type","u16","code","u32","value"})
		end

		if tab then
			print(tab.secs,tab.type,tab.code,tab.value)
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

