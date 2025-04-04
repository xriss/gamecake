--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtext)
wtext=wtext or {}

function wtext.update(widget)

	if widget.data then
		if widget.text~=widget.data.str then
			widget.text=widget.data.str
			widget:resize()
		end
	end

	return widget.meta.update(widget)
end


function wtext.draw(widget)
	return widget.meta.draw(widget)
end


-- auto resize to text contents horizontally
function wtext.resize(widget)
	widget.hx,widget.hy=widget:sizeof_text() -- auto set to size of text
	widget.meta.resize(widget) -- maybe changed here
end


function wtext.setup(widget,def)
--	local it={}
--	widget.button=it
	widget.class="text"
	
	widget.key=wtext.key
	widget.mouse=wtext.mouse
	widget.update=wtext.update
	widget.resize=wtext.resize
	widget.draw=wtext.draw

	return widget
end

return wtext
end
