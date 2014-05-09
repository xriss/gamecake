--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- a scrolling area, the widget is biger than display area but scroll bars allow you to see it all



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wfile)
wfile=wfile or {}

function wfile.mouse(widget,act,x,y,key)
	return widget.meta.mouse(widget,act,x,y,key)
end


function wfile.key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function wfile.update(widget)
	return widget.meta.update(widget)
end

function wfile.draw(widget)
	return widget.meta.draw(widget)
end

function wfile.setup(widget,def)
	widget.class="file"
	
	widget.key=wfile.key
	widget.mouse=wfile.mouse
	widget.update=wfile.update
	widget.draw=wfile.draw

-- auto add the draging button as a child
--[[
	local ss=16
	if widget.hx<ss*2 then ss=widget.hx/2 end
	if widget.hy<ss*2 then ss=widget.hy/2 end
	
	widget.pan=		widget:add({class="pan",	hx=widget.hx-ss,	hy=widget.hy-ss,	})
	widget.slidey=	widget:add({class="slide",	hx=ss,				hy=widget.hy-ss,	px=widget.hx-ss,	py=0,
		datx={max=0},daty={max=1},color=0xffffffff})
	widget.slidex=	widget:add({class="slide",	hx=widget.hx-ss,	hy=ss,           	px=0,           	py=widget.hy-ss,
		datx={max=1},daty={max=0},color=0xffffffff})
]]
	return widget
end

return wfile
end
