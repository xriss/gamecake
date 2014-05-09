--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdrag)
wdrag=wdrag or {}

function wdrag.mouse(widget,act,x,y,key)
	return widget.meta.mouse(widget,act,x,y,key)
end


function wdrag.key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function wdrag.update(widget)

	if widget.data then
		widget.text=widget.data:tostring()
	end

	return widget.meta.update(widget)
end

function wdrag.draw(widget)
	return widget.meta.draw(widget)
end


function wdrag.setup(widget,def)
--	local it={}
--	widget.drag=it
	widget.class="drag"
	
	widget.key=wdrag.key
	widget.mouse=wdrag.mouse
	widget.update=wdrag.update
	widget.draw=wdrag.draw

	widget.solid=true

	return widget
end

return wdrag
end
