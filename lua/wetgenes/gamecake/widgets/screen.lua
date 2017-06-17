--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[

a screen is a window container, windows added to a screen will be 
constrained to within its bounds, a window can also contain a screen 
that contains other windows. So it is a window grouping system.

Screens provide a background window layer that allow windows to snap 
into edges and exist as tiles without overlap, as in a tiling window 
manager.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wscreen)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wscreen=wscreen or {}

function wscreen.update(widget)
	return widget.meta.update(widget)
end

function wscreen.draw(widget)
	return widget.meta.draw(widget)
end

function wscreen.layout(widget)
	return widget.meta.layout(widget)
end

function wscreen.setup(widget,def)

	widget.class="screen"
	
	widget.update=wscreen.update
	widget.draw=wscreen.draw
	widget.layout=wscreen.layout
	
	return widget
end

return wscreen
end
