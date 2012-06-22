-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local win={}

local core -- we may use different cores depending on the system we compiled for so check for the right one

core=require("wetgenes.win.core")
--core=require("wetgenes.nix.core")
--core=require("wetgenes.nacl.core")
--core=require("wetgenes.raspi.core")


local base={}
local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	it={}
	it.width,it.height= core.screen()
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
	
	w[0]=assert( core.create(opts) )
	
	core.info(w[0],w)
	return w
end

function base.destroy(w)
	core.destroy(w[0],w)
end

function base.info(w)
	core.info(w[0],w)
end

function base.context(w,opts)
	core.context(w[0],opts)
end

function base.swap(w)
	core.swap(w[0])
end

function base.peek(w)
	return core.peek(w[0])
end

function base.wait(w,t)
	core.wait(w[0],t)
end

function base.msg(w)
	return core.msg(w[0])
end

function base.sleep(...)
	for i,v in ipairs({...}) do
		if type(v)=="number" then
			core.sleep(v)
		end
	end
end
win.sleep=base.sleep

function base.time()
	return core.time()
end
win.time=base.time

return win
