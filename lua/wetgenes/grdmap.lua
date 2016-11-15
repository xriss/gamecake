--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local grdmap={}

local core=require("wetgenes.grdmap.core")

local base={}
local meta={}
meta.__index=base

function grdmap.create()

	local gm={}
	
	gm[0]=core.create()	
	setmetatable(gm,meta)
	
	core.info(gm[0],gm)
	
	return gm
end

base.setup=function(gm,g)
	local r=core.setup(gm[0],g[0])
	core.info(gm[0],gm)
	return r
end

base.cutup=function(gm,...)
	local r=core.cutup(gm[0],...)
	core.info(gm[0],gm)
	return r
end

base.tile=function(gm,...)
	local r=core.tile(gm[0],...)
	return r
end

base.merge=function(gm,...)
	local r=core.merge(gm[0],...)
	core.info(gm[0],gm)
	return r
end

base.shrink=function(gm,...)
	local r=core.shrink(gm[0],...)
	core.info(gm[0],gm)
	return r
end

base.keymap=function(gma,gmb)
	local r=core.keymap(gma[0],gmb[0])
	core.info(gma[0],gma)
	return r
end

return grdmap
