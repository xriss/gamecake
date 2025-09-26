--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local jit=jit

local wpath=require("wetgenes.path")
local wsbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local bit=require("bit")

local M={ modname=(...) } ; package.loaded[M.modname]=M
local win = M

local log,dump=require("wetgenes.logs"):export("log","dump")

local base={}

local steam=require("wetgenes.win.steam") -- try and setup steam 
if steam.loaded then win.steam=steam end  -- put steam inside win if it got loaded so we can use flag if(win.steam)

local flavour_request=flavour_request or os.getenv("gamecake_flavour")

local softcore=require("wetgenes.win.core") -- we keep some generic C functions here

local hardcore -- but use different hardcores depending on the system we compiled for and are running on

local posix -- set if we are a posix system

local args={...}

-- only SDL

base.flavour="sdl"
base.noblock=false
hardcore=require("wetgenes.win.sdl")
base.sdl_platform=hardcore.platform

if base.sdl_platform=="Emscripten" then
	base.noblock=true
end

--[[
if type(args[2]=="table" ) then -- you can force a core by using a second arg to require
	hardcore=args[2]
elseif type(args[2]=="string" ) then
	flavour_request=args[2]
end

if flavour_request then print("The requested flavour of win is "..(flavour_request or "any")) end

-- probe for available hardcores, 
local hardcores={}
for _,it in ipairs({
		{name="sdl",		noblock=false,	posix=false,	}, -- we are probably using this one
--		{name="emcc",		noblock=false,	posix=false,	}, -- this is a slightly modified version of sdl
--		{name="nacl",		noblock=true,	posix=false,	}, -- the rest are old and probably broken
--		{name="android",	noblock=false,	posix=false,	},
--		{name="windows",	noblock=false,	posix=false,	},
--		{name="linux",		noblock=false,	posix=true,		},
--		{name="osx",		noblock=false,	posix=true,		},
--		{name="raspi",		noblock=false,	posix=true,		},
	}) do
	

	local suc,lib=pcall(function() return require("wetgenes.win."..it.name) end )
	
	if suc then
		it.lib=lib
		hardcores[it.name]=it
		
		hardcores["any"]=it
	end

end

-- choose hardcore
local info

--if flavour_request and hardcores[flavour_request] then -- force this one
--	info=hardcores[flavour_request]
--end


if (not info) and hardcores.sdl and ( pcall(function() return require("SDL") end ) ) then -- prefer sdl if we have it available

	info=hardcores.sdl
	
	if hardcores.emcc then info=hardcores.emcc end -- emcc is a tweaked sdl and should be used instead

end

if not info then info=hardcores.any end -- otherwise use any core


if info then -- finish setup

	base.flavour=info.name
	base.noblock=info.noblock
	hardcore=info.lib

	if info.name=="sdl" then

		base.sdl_platform=hardcore.platform

	end
	
--	if info.posix then
--		posix=require("wetgenes.win.posix")
--	end
	
end

]]

--log("oven","The flavour of win is "..base.flavour)

win.hardcore=hardcore
win.softcore=softcore
win.posix=posix

-- a dir to store config or user generated files in
win.files_prefix=wpath.resolve("./files/")
-- a dir to store cache files in, may auto delete but also should be deleted by app
win.cache_prefix=wpath.resolve("./cache/")


local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	local it={}
	if hardcore and hardcore.screen then
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

-- windows/nacl/emcc/osx/raspi patch, as we aim to use the linux key names
-- might not catch everything here its mostly added when I see its wrong

	["backspace"]="back",
	["kp_enter"]="enter",
	["oem_3"]="grave",
	["esc"]="escape",

	["lshift"]="shift_l",
	["rshift"]="shift_r",

	["left shift"]="shift_l",
	["right shift"]="shift_r",
	
	["lcontrol"]="control_l",
	["rcontrol"]="control_r",
	
	["left control"]="control_l",
	["right control"]="control_r",

	["left ctrl"]="control_l",
	["right ctrl"]="control_r",

	["lmenu"]="alt_l",
	["rmenu"]="alt_r",

	["left alt"]="alt_l",
	["right alt"]="alt_r",

	["page up"]="prior",
	["page_up"]="prior",

	["page down"]="next",
	["page_down"]="next",

	["keypad /"]="kp_divide",
	["keypad *"]="kp_multiply",
	["keypad -"]="kp_subtract",
	["keypad +"]="kp_add",
	
	["`"]="grave",

}


function win.keymap(key)
	key=key:lower()
	return win.generic_keymap[key] or key
end

function win.load_run_init(args)

	local zips=require("wetgenes.zips")

-- Now load and run lua/init.lua which initalizes the window and runs the app

	local s=args.init or zips.readfile("lua/init.lua")
	
	assert(s) -- sanity, may want a seperate path for any missing init.lua ?

	if s:sub(1,2)=="#!" then
		s="--"..s -- ignore hashbang on first line
	end
	
	local f=assert(loadstring(s))
	
	return f(args)

end


function win.create(opts)

	local w={}
	setmetatable(w,meta)
	
	if hardcore and hardcore.create then
		w[0]=assert( hardcore.create(opts) )
	end
	w.msgstack={} -- can feed "fake" msgs into here (fifo stack) with table.push
	w.width=0
	w.height=0
	w.overscale=opts.overscale or 1
	
	-- keep track of keyboard qualifiers
	w.qualifiers={false,false,false,false} -- flags of "alt" , "ctrl" , "shift" , "win"  in 1,2,3,4 so we know which keys are held down
	w.qualifiers_text=nil -- string of the above flags joined by + or nil if no curretn qualifiers

	base.info(w)

	if posix and base.flavour=="raspi" then
		posix.win_translate_msg=function(m) return posix.win_translate_msg_keys_and_mouse(w,m) end -- need to make real keyboard/mouse msgs
	end

	-- run main_update often from this point on, which then runs the global OVEN
	if base.noblock then
		require("global").main_update=function() if OVEN then OVEN:serv_pulse() end end
		softcore.set_main_loop()
	end
	
	return w
end


function win.js_post(m,d)
	if hardcore and hardcore.js_post then
		hardcore.js_post(m,d)
	end
end


function base.destroy(w)
	if hardcore and hardcore.destroy then
		hardcore.destroy(w[0],w)
	end
end

function base.show(w,s)
	w.view=s
	if hardcore and hardcore.show then
		hardcore.show(w[0],s)
	end
end

function base.info(w)
	if hardcore and hardcore.info then
		hardcore.info(w[0],w)
--		print("WH",w.width,w.height)
	end
end

function base.resize(w,width,height)
	if hardcore and hardcore.resize then
		hardcore.resize(w[0],width,height)
	end
end

function base.context(w,opts)
	if hardcore and hardcore.context then
		hardcore.context(w[0],opts)
	end
end

function base.start(w)
	if hardcore and hardcore.start then
		hardcore.start(w[0])
	end
end

function base.stop(w)
	if hardcore and hardcore.stop then
		hardcore.stop(w[0])
	end
end

function base.swap(w)
	if hardcore and hardcore.swap then
		hardcore.swap(w[0])
	end
end

function base.peek(w)
	if hardcore and hardcore.peek then
		return hardcore.peek(w[0])
	end
end

function base.wait(w,t)
	if hardcore and hardcore.wait then
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
	if not m and hardcore and hardcore.msg then
		m=hardcore.msg(w[0])
	end
	if not m and posix then
		m=base.posix_msg(w)
	end
	if not m and hardcore and hardcore.smell_msg then
		m=hardcore.smell_msg() --hardcoded stuff
		if m then
			m=wsbox.lson(m)
--			print(wstr.dump(m))
		end
	end

	if m then -- proccess the msg some more
	
		if m.class=="mouse" then
			if not m.keyname then -- apply mouse buttn names if all we have is a keycode
				if m.keycode==1 then m.keyname="left"
				elseif m.keycode==2 then m.keyname="middle"
				elseif m.keycode==3 then m.keyname="right"
				elseif m.keycode==4 then m.keyname="wheel_add"
				elseif m.keycode==5 then m.keyname="wheel_sub"
				elseif m.keycode==6 then m.keyname="wheel_right"
				elseif m.keycode==7 then m.keyname="wheel_left"
				else m.keyname="mouse"
				end
			end
		end
	
		if m.keyname then -- run it through our keymap probably just force it to lowercase.
			m.keyname=win.keymap(m.keyname)
		end
		
		local build_qualifiers_text=function()
			local s
			local add=function(key) if key then if s then s=s.."+"..key else s=key end end end
			add(w.qualifiers[1])
			add(w.qualifiers[2])
			add(w.qualifiers[3])
			add(w.qualifiers[4])
			w.qualifiers_text=s
		end
		if m.class=="key" then
			if     ( m.keyname=="alt" or m.keyname=="alt_l" or m.keyname=="alt_r" ) then
				if     m.action== 1 then w.qualifiers[1]="alt"
				elseif m.action==-1 then w.qualifiers[1]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="control" or m.keyname=="control_l" or m.keyname=="control_r" ) then
				if     m.action== 1 then w.qualifiers[2]="ctrl"
				elseif m.action==-1 then w.qualifiers[2]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) then
				if     m.action== 1 then w.qualifiers[3]="shift"
				elseif m.action==-1 then w.qualifiers[3]=false end
				build_qualifiers_text()
			elseif ( m.keyname=="gui" or m.keyname=="left gui" or m.keyname=="right gui" ) then -- gui/command/windows key?
				if     m.action== 1 then w.qualifiers[4]="win"
				elseif m.action==-1 then w.qualifiers[4]=false end
				build_qualifiers_text()
			end
			m.qualifiers=w.qualifiers_text -- add our cached qualifiers to key messages
		elseif m.class=="mouse" then
			m.qualifiers=w.qualifiers_text -- add our cached qualifiers to mouse messages ( so we can easily catch ctrl click etc )
		end

	end
	
	return m
end

function base.jread(w,n)
	if hardcore and hardcore.jread then
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
	if hardcore and hardcore.sleep then
		for i,v in ipairs({...}) do	-- ignore first arg if it is a table so we can call with :
			if type(v)=="number" then
				hardcore.sleep(v)
			end
		end
	end
end
win.sleep=base.sleep

do	-- get best time we can, should have at least ms accuracy, possibly slightly more
	local ok=pcall(function()
		local socket = require("socket")
		base.time=function() return socket.gettime() end
	end) or pcall(function()
		local lanes = require("lanes")
		base.time=function() return lanes.now_secs() end
	end) or pcall(function()
		base.time=function()
			if hardcore and hardcore.time then
				return hardcore.time()
			else
				return os.time()
			end
		end
	end)
end
win.time=base.time

function base.smell_check()
	if hardcore and hardcore.smell_check then
		return hardcore.smell_check()
	end
end

function base.glyph_8x8(n)
	return softcore.glyph_8x8(n)
end
win.glyph_8x8=base.glyph_8x8

function base.get_exe_path()
	return softcore.get_exe_path()
end
win.get_exe_path=base.get_exe_path

function base.get_mod_path()
	return softcore.get_mod_path()
end
win.get_mod_path=base.get_mod_path


function base.posix_open_events(w)
	if not posix then return end
	return posix.win_open_events(w)
end

function base.posix_read_events(w)
	if not posix then return end
	return posix.win_read_events(w)
end

function base.posix_close_events(w)
	if not posix then return end
	return posix.win_close_events(w)
end

function base.posix_msg(w)
	if not posix then return end
	return posix.win_msg(w)
end

function base.get_memory_class(w)
	if hardcore.get_memory_class then
		return hardcore.get_memory_class()
	end
end


-- clipboard (only SDL2 and only text)

function base.get_clipboard()
	if hardcore and hardcore.get_clipboard then
		return hardcore.get_clipboard()
	end
end

function base.set_clipboard(w,s)
	s=s or w -- first arg may be window for : calling
	if hardcore and hardcore.set_clipboard then
		return hardcore.set_clipboard(s)
	end
end

function base.has_clipboard()
	if     hardcore and hardcore.has_clipboard then
		return hardcore.has_clipboard()
	elseif hardcore and hardcore.get_clipboard then
		return hardcore.get_clipboard() and true or false
	end
end


-- again SDL only
-- x and y are windows coords, as if from an input mouse position

function base.warp_mouse(w,x,y)
	if     hardcore and hardcore.warp_mouse then
		return hardcore.warp_mouse(w[0],x,y)
	end
end

function base.relative_mouse(w,b)
	if     hardcore and hardcore.relative_mouse then
		return hardcore.relative_mouse(w[0],b)
	end
end

-- set the mouse cursor to one from the available theme
function base.cursor(...)
	if     hardcore and hardcore.cursor then
		return hardcore.cursor(...)
	end
end

-- set hints
function base.hints(...)
	if     hardcore and hardcore.hints then
		return hardcore.hints(...)
	end
end


-- show/hide onscreen keyboard
function base.StartTextInput(...)
	if     hardcore and hardcore.StartTextInput then
		return hardcore.StartTextInput(...)
	end
end
function base.StopTextInput(...)
	if     hardcore and hardcore.StopTextInput then
		return hardcore.StopTextInput(...)
	end
end

function base.GetPrefPath(...)
	if     hardcore and hardcore.GetPrefPath then
		return hardcore.GetPrefPath(...)
	end
end

