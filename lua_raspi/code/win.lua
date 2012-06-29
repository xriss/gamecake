-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local win={}

local core -- we may use different cores depending on the system we compiled for and are running on

local args={...}

if type(args[2]=="table" ) then -- you can force a core by using a second arg to require
	core=args[2]
end

if not core then
	local suc,dat=pcall(function() return require("wetgenes.win.windows") end )
	if suc then core=dat end
end

if not core then
	local suc,dat=pcall(function() return require("wetgenes.win.linux") end )
	if suc then core=dat end
end

if not core then
	local suc,dat=pcall(function() return require("wetgenes.win.raspi") end )
	if suc then core=dat end
end



local base={}
local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	it={}
	if core.screen then
		it.width,it.height=core.screen()
	else
		it.width,it.height=0,0
	end
	return it
end

-- key names are given in raw OS flavour,
-- this maps these raw names to more generic names
win.generic_keymap={

}

function win.keymap(key)
	return win.generic_keymap[key] or key
end

function win.create(opts)

	local w={}
	setmetatable(w,meta)
	
	if core.create then
		w[0]=assert( core.create(opts) )
	end
	
	base.info(w)
	return w
end

function base.destroy(w)
	if core.destroy then
		core.destroy(w[0],w)
	end
end

function base.info(w)
	if core.info then
		core.info(w[0],w)
	end
end

function base.context(w,opts)
	if core.context then
		core.context(w[0],opts)
	end
end

function base.swap(w)
	if core.swap then
		core.swap(w[0])
	end
end

function base.peek(w)
	if core.peek then
		return core.peek(w[0])
	end
end

function base.wait(w,t)
	if core.wait then
		core.wait(w[0],t)
	end
end

function base.msg(w)
	if core.msg then
		return core.msg(w[0])
	end
end

function base.sleep(...)
	if core.sleep then
		for i,v in ipairs({...}) do
			if type(v)=="number" then
				core.sleep(v)
			end
		end
	end
end
win.sleep=base.sleep

function base.time()
	if core.time then
		return core.time()
	else
		return os.time()
	end
end
win.time=base.time

return win
