--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtabs)
wtabs=wtabs or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")

function wtabs.mouse(widget,act,_x,_y,keyname)
	return widget.meta.mouse(widget,act,_x,_y,keyname)
end

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

	widget.class="tabs"
	
--[[
	widget.key=wtabs.key
	widget.mouse=wtabs.mouse
	widget.update=wtabs.update
	widget.layout=wtabs.layout
	widget.draw=wtabs.draw

-- auto add the draging button as a child
	local ss=16
	if widget.hx<ss*2 then ss=widget.hx/2 end
	if widget.hy<ss*2 then ss=widget.hy/2 end
	local s2=math.ceil(ss/2)

	widget.datx=widget_data.new_data{max=1,master=widget.master}
	widget.daty=widget_data.new_data{max=1,master=widget.master}
	widget.solid=true
	
	widget.color=widget.color or 0
	
	widget.pan=		widget:add({class=def.scroll_pan or "pan",	hx=widget.hx-s2,	hy=widget.hy-s2	,color=widget.color})
	widget.slidey=	widget:add({class="slide",	hx=s2,				hy=widget.hy-s2,	px=widget.hx-s2,	py=0,
		daty=widget.daty,color=widget.color})
	widget.slidex=	widget:add({class="slide",	hx=widget.hx,	hy=s2,           	px=0,           	py=widget.hy-s2,
		datx=widget.datx,color=widget.color})
]]

	return widget
end

return wtabs
end
