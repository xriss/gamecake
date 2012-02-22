--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- the core module previously lived in "grd" now it is in "wetgenes.grd.core" with this wrapper code

module("wetgenes.grdmap")

local core=require("wetgenes.grdmap.core")

base={}
meta={}
meta.__index=base

function create(...)

	local gm=core.create(...)
	
	if gm then
		setmetatable(gm,meta)
	end
	
	return gm
end

base.setup=function(gm,g)
	return core.setup(gm,g[0])
end

base.cutup=function(...)
	return core.cutup(...)
end

base.tile=function(...)
	return core.tile(...)
end

base.merge=function(...)
	return core.merge(...)
end

base.shrink=function(...)
	return core.shrink(...)
end

base.keymap=function(...)
	return core.keymap(...)
end
