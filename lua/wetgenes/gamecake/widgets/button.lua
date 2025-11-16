--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.button

	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="button",...}

A button for pressing and such.

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

--[[#lua.wetgenes.gamecake.widgets.button.update

	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this button then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?

]]
function B.update(widget)

	if widget.data then
		if widget.data_selected then
			if widget.data:tostring()==widget.data_selected then
				widget.state="selected"
			else
				widget.state="none"
			end
		else
			widget.text=widget.data:tostring()
		end
	end

	return widget.meta.update(widget)
end

--[[#lua.wetgenes.gamecake.widgets.button.setup

	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

Also cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.

]]
function B.setup(widget)

	widget.class="button"
	
	widget.update=B.update

	widget.solid=true
	widget.cursor=widget.cursor or "hand"

	return widget
end

return B
end
