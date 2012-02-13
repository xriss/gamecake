--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
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
	
	if grd then
		setmetatable(grd,meta)
	end
	
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

base.blit=function(ga,gb,x,y,cx,cy,cw,ch)

	if cx then -- autoclip
		if cx<0 then cw=cw+cx cx=0 end
		if cy<0 then ch=ch+cy cy=0 end
		if (cx+cw)>gb.width  then cw=gb.width -cx end
		if (cy+ch)>gb.height then ch=gb.height-cy end
	else -- auto build
		cx=0
		cy=0
		cw=gb.width
		ch=gb.height
	end
	
	if x<0 then cx=cx-x cw=cw+x x=0 end
	if y<0 then cy=cy-y ch=ch+y y=0 end	
	if (x+cw)>ga.width  then cw=ga.width -x end
	if (y+ch)>ga.height then ch=ga.height-y end

	if cw<=0 or ch<=0 then return true end -- nothing to draw

	return core.blit(ga,gb,x,y,cx,cy,cw,ch)
end
