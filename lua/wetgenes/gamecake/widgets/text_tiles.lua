--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtext_tiles)
wtext_tiles=wtext_tiles or {}

function wtext_tiles.update(widget)

--	if widget.data then
--		widget.text=widget.data.str
--	end

	return widget.meta.update(widget)
end

--function wtext_tiles.draw(widget)
--	return widget.meta.draw(widget)
--end

function wtext_tiles.skin(widget)
	return function()
		print("skinnnnn")
	end
end


function wtext_tiles.setup(widget,def)
--	local it={}
--	widget.button=it
	widget.class="text_tiles"

	widget.pan_px=0
	widget.pan_py=0

	widget.hx_max=widget.hx
	widget.hy_max=widget.hy

	widget.key=wtext_tiles.key
	widget.mouse=wtext_tiles.mouse
	widget.update=wtext_tiles.update
	widget.draw=wtext_tiles.draw
	widget.skin=wtext_tiles.skin

	return widget
end

return wtext_tiles
end
