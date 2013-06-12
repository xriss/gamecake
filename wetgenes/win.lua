-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local jit=jit

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local bit=require("bit")

local win={}
local base={}

local softcore=require("wetgenes.win.core") -- we keep some generic C functions here

local hardcore -- but use different hardcores depending on the system we compiled for and are running on

local posix -- set if we are a posix system

local args={...}


base.noblock=false
base.flavour="raw"

if type(args[2]=="table" ) then -- you can force a core by using a second arg to require
	hardcore=args[2]
end


if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.nacl") end )
	if suc then hardcore=dat base.flavour="nacl" base.noblock=true end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.android") end )
	if suc then hardcore=dat base.flavour="android"
--		posix=require("posix")
	end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.raspi") end )
	if suc then hardcore=dat base.flavour="raspi"
		posix=require("posix")
	end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.windows") end )
	if suc then hardcore=dat base.flavour="windows" end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.linux") end )
	if suc then hardcore=dat base.flavour="linux"
		posix=require("posix")
	end
end

win.hardcore=hardcore
win.softcore=softcore
win.posix=posix

-- a dir to store config or user generated files in
win.files_prefix="./files/"
-- a dir to store cache files in, may auto delete but also should be deleted by app
win.cache_prefix="./cache/"


local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	local it={}
	if hardcore.screen then
		it.width,it.height=hardcore.screen()
	else
		it.width,it.height=0,0
	end
	return it
end

-- key names are given in a raw OS flavour,
-- this maps tries to map these names to more generic ones
-- really you can never be sure what name a key has...
win.generic_keymap={


-- android patch
	["android_04"]="escape",
	["android_6f"]="escape",
	["android_42"]="return",
	["android_43"]="back",
	["android_70"]="delete",
	["android_13"]="up",
	["android_14"]="down",
	["android_15"]="left",
	["android_16"]="right",
	["android_17"]="return",


-- windows patch
	["backspace"]="back",
	["kp_enter"]="enter",
	["oem_3"]="`",
	["esc"]="escape",
	
-- linux patch
	["grave"]="`",
	
}

function win.keymap(key)
	key=key:lower()
	return win.generic_keymap[key] or key
end

function win.load_run_init()

	local zips=require("wetgenes.zips")

-- Now load and run lua/init.lua which initalizes the window and runs the app

	local s=zips.readfile("lua/init.lua")
	
	assert(s) -- sanity, may want a seperate path for any missing init.lua ?

	if s:sub(1,2)=="#!" then
		s="--"..s -- ignore hashbang on first line
	end
	
	local f=assert(loadstring(s))
	
	return f()

end

--
-- Special android entry points, we pass in the location of the apk
-- this does things that must only happen once
--
function win.android_start(apk)

	if jit and jit.off then
		jit.off()
		hardcore.print("LUA JIT OFF")
	end -- jit breaks stuff?

-- replace print
	_G.print=hardcore.print
	print=_G.print

	win.apk=apk
	local zips=require("wetgenes.zips")
	zips.add_apk_file(win.apk)
	
	win.files_prefix=hardcore.get_files_prefix().."/"
	win.cache_prefix=hardcore.get_cache_prefix().."/"

	win.smell=hardcore.smell_check()

--print(win.files_prefix)
--print(win.cache_prefix)

--print("ANDROID_SETUP with ",apk)
	
	return win.load_run_init()
end


--
-- Special nacl entry points, we pass in the url of the main zip we wish to load
-- this does things that must only happen once
--
local main -- gonna have to cache the main state here
function win.nacl_start(url)
--	_G.print=hardcore.print
--	print=_G.print
	
--	print("nacl start ",url)

	local zips=require("wetgenes.zips")
	
-- we want nacl msgs to go here.
	_G.nacl_input_event=function(...) return hardcore.input_event(...) end

	hardcore.getURL(url,function(size,mem)	
--		print("nacl callback",size,mem)
		
		zips.add_zip_data(mem)
		main=win.load_run_init()

	end)

--	print("nacl start done")

end

function win.nacl_pulse() -- called 60ish times a second depending upon how retarted the browser is

	if main then
		main:serv_pulse()
	end
end


function win.create(opts)

	local w={}
	setmetatable(w,meta)
	
	if hardcore.create then
		w[0]=assert( hardcore.create(opts) )
	end
	w.msgstack={} -- can feed "fake" msgs into here (fifo stack) with table.push
	w.width=0
	w.height=0
	
	base.info(w)
	return w
end

function base.destroy(w)
	if hardcore.destroy then
		hardcore.destroy(w[0],w)
	end
end

function base.show(w,s)
	if hardcore.show then
		hardcore.show(w[0],s)
	end
end

function base.info(w)
	if hardcore.info then
		hardcore.info(w[0],w)
--		print("WH",w.width,w.height)
	end
end

function base.context(w,opts)
	if hardcore.context then
		hardcore.context(w[0],opts)
	end
end

function base.start(w)
	if hardcore.start then
		hardcore.start(w[0])
	end
end

function base.stop(w)
	if hardcore.stop then
		hardcore.stop(w[0])
	end
end

function base.swap(w)
	if hardcore.swap then
		hardcore.swap(w[0])
	end
end

function base.peek(w)
	if hardcore.peek then
		return hardcore.peek(w[0])
	end
end

function base.wait(w,t)
	if hardcore.wait then
		hardcore.wait(w[0],t)
	end
end

-- a msg iterator
function base.msgs(w)
	return function() return w:msg() end
end

-- push a msg onto the msgstack, so it feeds back into the input loop
function base.push_msg(w,m)
	w.msgstack[#w.msgstack+1]=m
end

-- get the next msg or return nil if there are no more
function base.msg(w)

	local m
	
	if not m and w.msgstack[1] then
		m=table.remove(w.msgstack,1)
	end
	if not m and hardcore.msg then
		m=hardcore.msg(w[0])
	end
	if not m and posix then
		m=base.posix_msg(w)
	end
	if not m and hardcore.smell_msg then
		m=hardcore.smell_msg() --hardcoded stuff
		if m then
			print(wstr.dump(m))
			m=nil
		end
	end

	if m then -- proccess the msg some more
	
		if m.keyname then -- run it through our keymap probably just force it to lowercase.
			m.keyname=win.keymap(m.keyname)
		end
		
	end
	
	return m
end

function base.jread(w,n)
	if hardcore.jread then
--print("jread")	
		local pkt=hardcore.jread(w[0],n)
		local tab
		if pkt then
			tab=pack.load(pkt,{"u32","time","s16","value","u8","type","u8","number"})
			tab.class="joy"
		end
		return tab
	end
end


function base.sleep(...)
	if hardcore.sleep then
		for i,v in ipairs({...}) do	-- ignore first arg if it is a table so we can call with :
			if type(v)=="number" then
				hardcore.sleep(v)
			end
		end
	end
end
win.sleep=base.sleep

function base.time()
	if hardcore.time then
		return hardcore.time()
	else
		return os.time()
	end
end
win.time=base.time


function base.glyph_8x8(n)
	return softcore.glyph_8x8(n)
end
win.glyph_8x8=base.glyph_8x8


function base.posix_open_events(w)
	if not posix then return end

	base.posix_events={}
	local events=base.posix_events
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
	fp:close()
	
--	print(wstr.dump(events))

	local kbdcount=0
	for i=0,#events do local v=events[i]
		if v then
			if v.handlers.kbd then -- open as keyboard, there may be many of these and it is all a hacky
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened keyboard "..kbdcount.." on event"..v.event.." "..v.name)
					v.fd_device=kbdcount
					v.fd_type="keyboard"
					kbdcount=kbdcount+1
				else
					print("failed to open keyboard "..kbdcount.." on event"..v.event.." "..v.name)
				end
			elseif v.js then -- open as joystick	
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened joystick "..v.js.." on event"..v.event.." "..v.name)
					v.fd_device=v.js
					v.fd_type="joystick"
				else
					print("failed to open joystick "..v.js.." on event"..v.event.." "..v.name)
				end
			elseif v.mouse then -- open as mouse
				v.fd=posix.open("/dev/input/event"..v.event, bit.bor(posix.O_NONBLOCK , posix.O_RDONLY) )
				if v.fd then
					print("opened mouse "..v.mouse.." on event"..v.event.." "..v.name)
					v.fd_device=v.mouse
					v.fd_type="mouse"
				else
					print("failed to open mouse "..v.mouse.." on event"..v.event.." "..v.name)
				end
			end
		end
	end

--	print(wstr.dump(events))

	
end
function base.posix_read_events(w) -- call this until it returns nil to get all events
	if not posix then return end
	
	local events=base.posix_events
	for i=0,#events do local v=events[i]
		if v then
			if v.fd then
				local pkt=posix.read(v.fd,16)
				if pkt then
					local tab=pack.load(pkt,{"u32","secs","u32","micros","u16","type","u16","code","u32","value"})
					tab.time=tab.secs+(tab.micros/1000000)
					tab.secs=nil
					tab.micros=nil
					tab.class="posix_"..v.fd_type
					tab.posix_device=v -- please do not edit this
	--print(tab.class)
					return tab
				end
			end
		end
	end
	
end
function base.posix_close_events(w)
	if not posix then return end
	base.posix_events=nil
end

function base.posix_msg(w)
	if not base.posix_events then -- need to initialize
		base.posix_open_events(w)
	end
	return base.posix_read_events(w)
end


return win
