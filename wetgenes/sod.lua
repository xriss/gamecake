--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local sod={}

local core=require("wetgenes.sod.core")

local meta={__index=sod}

sod.create=function()

	local sd={}
	setmetatable(sd,meta)
	
	sd[0]=assert( core.create() )
	
	return sd
end

sod.destroy=function(sd)
	core.destroy(sd[0])
end


sod.load=function(sd,name)
	assert(core.load(sd[0],name))
	return 0
end


return sod
