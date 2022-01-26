--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.center

	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="center",...}

A layout class for very centered children.

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

--[[#lua.wetgenes.gamecake.widgets.center.layout

	this function will also call lua.wetgenes.gamecake.widgets.meta.layout

Place any children in the center of this widget. Multiple children will 
overlap so probably best to only have one child.

]]
function B.layout(widget)
	
	for i,w in ipairs(widget) do
		if not w.hidden then
			w.px=(widget.hx-w.hx)/2
			w.py=(widget.hy-w.hy)/2
		end
	end
	
-- layout sub sub widgets	
	widget.meta.layout(widget)

end

--[[#lua.wetgenes.gamecake.widgets.center.setup

	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

If not explicetly set we will use a size of "full" ie the size of 
parent as that is probably how this class will always be used.

]]
function B.setup(widget)

	widget.class="center"
	widget.size=widget.size or "full" -- makes sense for this to usually be the size of its parent

	widget.layout=B.layout
	
	return widget
end

return B
end
