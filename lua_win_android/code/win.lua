-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

local win={}
local base={}

local softcore=require("wetgenes.win.core") -- we keep some generic C functions here

local hardcore -- but use different cores depending on the system we compiled for and are running on

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
	if suc then hardcore=dat base.flavour="android" base.noblock=true end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.raspi") end )
	if suc then hardcore=dat base.flavour="raspi" end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.windows") end )
	if suc then hardcore=dat base.flavour="windows" end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.linux") end )
	if suc then hardcore=dat base.flavour="linux" end
end

local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	it={}
	if hardcore.screen then
		it.width,it.height=hardcore.screen()
	else
		it.width,it.height=0,0
	end
	return it
end

-- key names are given in raw OS flavour,
-- this maps these raw names to more generic names
win.generic_keymap={

	["android_04"]="escape",
	["android_6f"]="escape",
	["android_42"]="return",
	["android_43"]="backspace",
	["android_70"]="delete",
	["android_13"]="up",
	["android_14"]="down",
	["android_15"]="left",
	["android_16"]="right",
	["android_17"]="return",
}

function win.keymap(key)
	key=key:lower()
	return win.generic_keymap[key] or key
end

function win.android_setup(apk)

	if win.flavour=="android" and hardcore.print then
	
		win.apk=apk
		
		_G.print=hardcore.print
		print=_G.print


print("ANDROID_SETUP with ",apk,4)
		
	end
	
end

function win.create(opts)

	local w={}
	setmetatable(w,meta)
	
	if hardcore.create then
		w[0]=assert( hardcore.create(opts) )
	end
	w.msgs={} -- can feed msgs into here (fifo stack) with table.push
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

function base.info(w)
	if hardcore.info then
		hardcore.info(w[0],w)
	end
end

function base.context(w,opts)
	if hardcore.context then
		hardcore.context(w[0],opts)
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

function base.msg(w)
	if w.msgs[1] then
		local m=table.remove(w.msgs,1)
		if m[1]=="key" then
			if m[5] then m[5]=win.keymap(m[5]) end -- patch key name to lowercase and rename if we have to
		end
--		_G.print(m)
		return m
	end
	if hardcore.msg then
		local m={hardcore.msg(w[0])}
		if m[1]=="key" then
			if m[5] then m[5]=win.keymap(m[5]) end -- patch key name to lowercase and rename if we have to
		end
		return m
	end
end

function base.sleep(...)
	if hardcore.sleep then
		for i,v in ipairs({...}) do
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

return win
