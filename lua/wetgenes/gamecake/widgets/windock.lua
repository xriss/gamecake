--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- handle a collection of windocks that all live in the same place

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wwindock)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

wwindock=wwindock or {}

function wwindock.update(widget)
	return widget.meta.update(widget)
end

function wwindock.draw(widget)
	return widget.meta.draw(widget)
end

function wwindock.layout(widget)
	return widget.meta.layout(widget)
end

function wwindock.setup(widget,def)

	widget.class="windock"
	widget.windock=def.windock or "drag" -- type of dock, drag is a collection of dragable windows

	widget.update=wwindock.update
	widget.draw=wwindock.draw
	widget.layout=wwindock.layout

	return widget
end

return wwindock
end
