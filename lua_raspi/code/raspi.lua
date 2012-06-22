--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local raspi={}

local core=require("wetgenes.raspi.core")

local wstr=require("wetgenes.string")

local base={}
local meta={}
meta.__index=base

setmetatable(raspi,meta)


function raspi.screen()
	it={}
	it.width,it.height= core.screen()
	return it
end

function raspi.create(opts)

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

function base.time()
	return core.time()
end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not raspi[n] then -- only if not prewrapped
			raspi[n]=v
		end
	end

end

return raspi
