--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wbutton)
wbutton=wbutton or {}

function wbutton.update(widget)

	if widget.data then
		widget.text=widget.data.str
	end

	return widget.meta.update(widget)
end

function wbutton.draw(widget)
	return widget.meta.draw(widget)
end


function wbutton.setup(widget,def)
--	local it={}
--	widget.button=it
	widget.class="button"
	
	widget.key=key
	widget.mouse=wbutton.mouse
	widget.update=wbutton.update
	widget.draw=wbutton.draw

	widget.solid=true
	widget.cursor=widget.cursor or "hand"

	return widget
end

return wbutton
end
