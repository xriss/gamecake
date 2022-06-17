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


B.hook_resize=function(widget)

	widget.hx=widget.parent.hx
	widget.hy=32

	local font=oven.cake.canvas.font

	local fy=widget:bubble("text_size") or 16
	local f=widget:bubble("font") or 4
	local fontfix=0.6 -- this fixes the baseline
	if type(f)=="number" then fontfix=0.4 end -- builtin fonts look better like this
	font.set(oven.cake.fonts.get(f))
	font.set_size(fy,0)
	local s=widget.text
	local lines=font.wrap(s,{w=widget.hx-fy}) -- break into lines
	
	widget.hy=(#lines+0.5)*fy
					
					
end

--[[#lua.wetgenes.gamecake.widgets.paragraph.setup

	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

]]
function B.setup(widget)

	widget.class="paragraph"
	
	widget.hook_resize=B.hook_resize
	
	return widget
end

return B
end
