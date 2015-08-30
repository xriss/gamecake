--
-- (C) 2013 Kriss@XIXs.com
--
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
	
--	ft:info()
	
	return ft
end

base.load_file=function(ft,s)
	local r,err=core.load_file(ft[0],s)
	ft:info()
end

base.load_data=function(ft,s)
	local r,err=core.load_data(ft[0],s)
	ft:info()
end

base.destroy=function(ft)
	local r,err=core.destroy(ft[0])
	ft:info()
end

base.size=function(ft,x,y)
	local r,err=core.size(ft[0],x,y)
	ft:info()
	return ft
end

base.glyph=function(ft,id)
	local r,err=core.glyph(ft[0],id)
	ft:info()
	return ft
end

base.render=function(ft,id)
	local r,err=core.render(ft[0],id)
	ft:info()
	return ft
end

base.tab=function(ft)
	local r,err=core.tab(ft[0])
	ft:info()
	return ft
end

base.grd=function(ft,g)
	local r,err=core.grd(ft[0],g[0])
	ft:info()
	g:info()
	return ft
end

base.info=function(ft)
	core.info(ft[0],ft)

	if ft.error then
		assert(not ft.error,ft.error)
	end

	return ft
end

return freetype
