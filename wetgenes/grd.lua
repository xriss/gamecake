-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- the core module previously lived in "grd" now it is in "wetgenes.grd.core" with this wrapper code

module("wetgenes.grd")

local core=require("wetgenes.grd.core")

base={}
meta={}
meta.__index=base

function create(...)

	local grd=core.create(...)
	setmetatable(grd,meta)

	return grd
end

base.destroy=function(...)
	return core.destroy(...)
end


base.reset=function(...)
	return core.reset(...)
end

base.load=function(...)
	return core.load(...)
end

base.save=function(...)
	return core.save(...)
end

base.convert=function(...)
	return core.convert(...)
end

base.quant=function(...)
	return core.quant(...)
end

base.pixels=function(...)
	return core.pixels(...)
end

base.palette=function(...)
	return core.palette(...)
end

base.scale=function(...)
	return core.scale(...)
end

base.flipy=function(...)
	return core.flipy(...)
end

base.blit=function(...)
	return core.blit(...)
end
