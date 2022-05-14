--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.widgets.drag

	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="drag",...}

A button to drag arround.

]]

-- module bake
local M={ modname = (...) } package.loaded[M.modname] = M function M.bake(oven,B) B = B or {}

local wdrag=B


--[[#lua.wetgenes.gamecake.widgets.drag.drag

]]
function wdrag.drag(widget,x,y)
	local parent=widget.parent
	local master=widget.master

	local rx,ry=parent:mousexy(x,y)
	local x,y=rx-master.active_xy[1],ry-master.active_xy[2]

	local maxx=parent.hx-widget.hx
	local maxy=parent.hy-widget.hy
	
	widget.px=math.floor(x)
	widget.py=math.floor(y)
	
	if widget.px<0    then widget.px=0 end
	if widget.px>maxx then widget.px=maxx end
	if widget.py<0    then widget.py=0 end
	if widget.py>maxy then widget.py=maxy end
	
	if parent.snap then
		parent:snap(true)
	end
	
	widget:call_hook_later("slide")
	
	widget:set_dirty()
	widget.master.request_layout=true
	
--print(x,y,widget.px,widget.py)
end

--[[#lua.wetgenes.gamecake.widgets.drag.update

]]
function wdrag.update(widget)

	if widget.data then
		widget.text=widget.data.str
	end

	return widget.meta.update(widget)
end


--[[#lua.wetgenes.gamecake.widgets.drag.setup

]]
function wdrag.setup(widget,def)

	widget.class="drag"
	
	widget.drag=wdrag.drag
	widget.update=wdrag.update

	widget.cursor=widget.cursor or "hand"

	widget.solid=true

	return widget
end

return wdrag
end
