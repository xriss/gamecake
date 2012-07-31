#!../../bin/exe/lua

-- setup some default search paths,
require("apps").default_paths()

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local wwin=require("wetgenes.win")
local bit = require("bit")
local posix=require("posix")

--[[
I: Bus=0003 Vendor=054c Product=0268 Version=0111
N: Name="Sony PLAYSTATION(R)3 Controller"
P: Phys=usb-0000:00:1d.0-1.5/input0
S: Sysfs=/devices/pci0000:00/0000:00:1d.0/usb2/2-1/2-1.5/2-1.5:1.0/input/input12
U: Uniq=
H: Handlers=event12 js0 
B: PROP=0
B: EV=1b
B: KEY=70000 0 0 0 0 0 0 0 0 0 0 0 0 ffff 0 0 0 0 0 0 0 0 0
B: ABS=7fffff00 27
B: MSC=10
]]


local events={}
local fp=io.open("/proc/bus/input/devices","r")
local tab={}
for l in fp:lines() do
	local t=l:sub(1,3)
	local v=l:sub(4)
	if t=="I: " then
		tab={} -- start new device
		tab.bus		=string.match(v,"Bus=([^%s]+)")
		tab.vendor	=string.match(v,"Vendor=([^%s]+)")
		tab.product	=string.match(v,"Product=([^%s]+)")
		tab.version	=string.match(v,"Version=([^%s]+)")
	end
	if t=="N: " then
		tab.name=string.match(v,"Name=\"([^\"]+)")
	end
	if t=="H: " then
		local t=string.match(v,"Handlers=(.+)")
		tab.event=tonumber(string.match(t,"event(%d+)"))
		tab.js=tonumber(string.match(t,"js(%d+)"))
		tab.mouse=tonumber(string.match(t,"mouse(%d+)"))
		events[tab.event]=tab
		tab.handlers={}
		for n in string.gmatch(t,"[^%s]+") do
			tab.handlers[n]=true
			if n:sub(1,5)~="event" then -- we already have events
				events[n]=tab
			end
		end
	end
end

print(wstr.dump(events))

local keydev
for i=0,#events do local v=events[i]
	if string.find(v.name:lower(),"keyboard") then
		keydev="/dev/input/event"..v.event
	end
end

print(keydev)

--find device in /proc/bus/input/devices  ?

local fd=assert(posix.open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) ))
--local fd=assert(posix.open("/dev/input/event3", bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) ))
--local fd=assert(posix.open("/dev/stdin", bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) ))

print("ret ",wwin.hardcore.ioctl(fd,0x4B45,0x00)) -- request raw mode

--print("ret ",wwin.hardcore.ioctl(fd,0x5564,0x01))
--print("ret ",wwin.hardcore.ioctl(fd,0x5501,0x00))

print("reading ",fd)

local pkt

while true do
	wwin.sleep(0.001)
	
	pkt=(posix.read(fd,16))
	
	if pkt then
	
		local tab=pack.load(pkt,{"u32","secs","u32","micros","u16","type","u16","code","u32","value"})
		
		print(tab.type,tab.code,tab.value)
		
--if tab.type==57 then
--		print(wstr.dump(tab))
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

