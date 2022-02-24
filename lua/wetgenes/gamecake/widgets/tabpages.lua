--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtabs)
wtabs=wtabs or {}

function wtabs.update(widget)
	return widget.meta.update(widget)
end

function wtabs.draw(widget)
	return widget.meta.draw(widget)
end

function wtabs.layout(widget)

	widget.meta.layout(widget)
end

function wtabs.setup(widget,def)

	widget.class="tabpages"
	
	local ss=widget.master.theme.grid_size

	widget.update = wtabs.update
	widget.draw   = wtabs.draw
	widget.layout = wtabs.layout

	if not widget.data then
		local list={}
		for i,v in ipairs( widget.list or {} ) do
			list[i]={str=v,num=i}
		end
		widget.data = widget.master.new_data{class="list",num=1,list=list}
	end

	widget.tabs  = widget:add({class="tabs",	hx=widget.hx,	hy=ss,				py=0,	color=widget.color, data=widget.data })
	widget.pages = widget:add({class="pages",	hx=widget.hx,	hy=widget.hy-ss,	py=ss,	color=widget.color, data=widget.data })


	widget.children=widget.pages

	return widget
end

return wtabs
end
