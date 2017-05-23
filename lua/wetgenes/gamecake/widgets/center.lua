--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wcenter)

wcenter=wcenter or {}

-- place any children in the center of this widget
function wcenter.layout(widget)
	
	for i,w in ipairs(widget) do
		if not w.hidden then
			w.px=(widget.hx-w.hx)/2
			w.py=(widget.hy-w.hy)/2
		end
	end
	
-- layout sub sub widgets	
	for i,v in ipairs(widget) do
		if not v.hidden then v:layout() end
	end
end

function wcenter.setup(widget,def)

	widget.class="center"
	widget.size=def.size or "full" -- makes sense for this to usually be the size of its parent

	widget.layout=wcenter.layout
	
	return widget
end

return wcenter
end
