--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[

A button for pressing and such.

]]


--module
local M={ modname = (...) } package.loaded[M.modname] = M

-- and bake
function M.bake(oven,B) B = B or {}

--[[

If we have a data assigned to this button then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is coppied here instead?

]]
function B.update(widget)

	if widget.data then
		widget.text=widget.data:tostring()
	end

	return widget.meta.update(widget)
end

--[[

we need to be solid and have a hand cursor

]]
function B.setup(widget,def)

	widget.class="button"
	
	widget.update=B.update

	widget.solid=true
	widget.cursor=widget.cursor or "hand"

	return widget
end

return B
end
