--
-- (C) 2017 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wpages)

wpages=wpages or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")

function wpages.update(widget)
	return widget.meta.update(widget)
end

function wpages.draw(widget)
	return widget.meta.draw(widget)
end

-- assume that only one child is visible at once so we toggle them on and off

function wpages.layout(widget)

	widget.hx=0
	widget.hy=0
	for i,v in ipairs(widget) do
		v.px=0
		v.py=0
		if v.hx>widget.hx then widget.hx=v.hx end
		if v.hy>widget.hy then widget.hy=v.hy end
--		v:layout()

		if widget.data and widget.data.num==i then
			v.hidden=false
		elseif widget.data then
			v.hidden=true
		end
	end

	widget.meta.layout(widget)
	
end

function wpages.setup(widget,def)

	widget.class="pages"
	
--	widget.data=widget.data or widget_data.new_data({master=widget.master})

	widget.update=wpages.update
	widget.draw=wpages.draw
	widget.layout=wpages.layout
	
	return widget
end

return wpages
end
