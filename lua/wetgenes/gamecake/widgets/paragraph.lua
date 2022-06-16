--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.paragraph

	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="paragraph",...}

A layout class for a paragraph of wordwrapped text that ignores 
quantity of whitespace and possibly child widgets insereted into the 
text and apropriate control points.

We will be the full width of our parent and as tall as we need to be to 
fit the given text.

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}


--[[#lua.wetgenes.gamecake.widgets.paragraph.setup

	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

]]
function B.setup(widget)

	widget.class="paragraph"
	
	return widget
end

return B
end
