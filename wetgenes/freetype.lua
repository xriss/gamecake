--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local freetype={}

local core=require("wetgenes.freetype.core")

local base={}
local meta={}
meta.__index=base

function freetype.create()

	local ft={}
	
	ft[0]=core.create()	
	setmetatable(ft,meta)
	
	core.info(ft[0],ft)
	
	return ft
end

base.load_file=function(ft,s)
	local r,err=core.load_file(ft[0],s)
	core.info(ft[0],ft)
end

base.load_data=function(ft,s)
	local r,err=core.load_data(ft[0],s)
	core.info(ft[0],ft)
end

base.destroy=function(ft)
	local r,err=core.destroy(ft[0])
	core.info(ft[0],ft)
end

base.destroy=function(ft)
	local r,err=core.destroy(ft[0])
	core.info(ft[0],ft)
end

base.size=function(ft,x,y)
	local r,err=core.size(ft[0],x,y)
	core.info(ft[0],ft)
	return ft
end

base.glyph=function(ft,id)
	local r,err=core.glyph(ft[0],id)
	core.info(ft[0],ft)
	return ft
end

base.render=function(ft,id)
	local r,err=core.render(ft[0],id)
	core.info(ft[0],ft)
	return ft
end

base.tab=function(ft)
	local r,err=core.tab(ft[0])
	core.info(ft[0],ft)
	return ft
end

base.grd=function(ft,g)
	local r,err=core.grd(ft[0],g[0])
	core.info(ft[0],ft)
	g:info()
	return ft
end


return freetype
