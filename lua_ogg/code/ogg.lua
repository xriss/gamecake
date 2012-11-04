--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local ogg={}

local core=require("wetgenes.ogg.core")

local meta={__index=ogg}

ogg.create=function()
	local od={}
	setmetatable(od,meta)
	
	od[0]=assert( core.create() )
	
	core.info(od[0],od)
	return od
end

ogg.destroy=function(od)
	core.destroy(od[0])
end

ogg.open=function(od)
	core.open(od[0])
	core.info(od[0],od)
	return od
end
ogg.close=function(od)
	core.close(od[0])
	core.info(od[0],od)
	return od
end

ogg.push=function(od,dat)
	if not dat then return end
	local r1,r2=core.push(od[0],dat)
	core.info(od[0],od)
	return r1,r2
end

ogg.pull=function(od)
	local r1,r2=core.pull(od[0])
	core.info(od[0],od)
	return r1,r2
end

return ogg
