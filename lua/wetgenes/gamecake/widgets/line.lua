--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wline)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wline=wline or {}

-- this is a fixed layout that works like a line pf text
-- we place all widgets next to each other on the same line
-- so our width is all widths added together and the height is the max height

function wline.layout(widget)
	local hx=0
	for i,v in ipairs(widget) do -- position children
		v.px=hx
		v.py=0		
		hx=hx+v.hx
	end
	return widget.meta.layout(widget)
end

function wline.resize(widget)
	widget.meta.resize(widget)
	local hx,hy=0,0
	for i,v in ipairs(widget) do -- find size of children
		v:resize()
		hx=hx+v.hx
		if v.hy>hy then hy=v.hy end
	end
	widget.hx=hx
	widget.hy=hy
end


function wline.setup(widget,def)

	widget.class="line"

	widget.resize=wline.resize
	widget.layout=wline.layout
	
	return widget
end

return wline
end
