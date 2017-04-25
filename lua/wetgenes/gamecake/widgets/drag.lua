--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdrag)
wdrag=wdrag or {}

local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")

function wdrag.drag(widget,x,y)

	local parent=widget.parent
	local master=widget.master

	local rx,ry=parent:mousexy(x,y)
	local x,y=rx-master.active_xy[1],ry-master.active_xy[2]

	local maxx=parent.hx-widget.hx
	local maxy=parent.hy-widget.hy
	
	widget.px=x
	widget.py=y
	
	if widget.px<0    then widget.px=0 end
	if widget.px>maxx then widget.px=maxx end
	if widget.py<0    then widget.py=0 end
	if widget.py>maxy then widget.py=maxy end
	
	if parent.snap then
		parent:snap(true)
	end
	
	widget:call_hook_later("slide")
	
	widget:set_dirty()
	
	widget:layout()
	widget:build_m4()
end

function wdrag.update(widget)

	if widget.data then
		widget.text=widget.data.str
	end

	return widget.meta.update(widget)
end

function wdrag.draw(widget)
	return widget.meta.draw(widget)
end


function wdrag.setup(widget,def)

	widget.class="drag"
	
	widget.drag=wdrag.drag
	widget.key=wdrag.key
	widget.mouse=wdrag.mouse
	widget.update=wdrag.update
	widget.draw=wdrag.draw
	widget.layout=wfill.layout

	widget.solid=true

	return widget
end

return wdrag
end
