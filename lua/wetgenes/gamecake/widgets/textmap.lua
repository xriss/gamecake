--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstring=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wutf=require("wetgenes.txt.utf")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wtextmap)

	wtextmap=wtextmap or {}
	wtextmap.modname=M.modname

	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")
	local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")



wtextmap.draw=function(widget)
	return widget.meta.draw(widget)
end

wtextmap.refresh=function(widget)
end

wtextmap.mouse=function(pan,act,_x,_y,keyname)
end


wtextmap.msg=function(pan,m)
end

wtextmap.key=function(pan,ascii,key,act)
end

wtextmap.layout=function(widget)

	widget.scroll_widget.hx=widget.hx
	widget.scroll_widget.hy=widget.hy

	return widget.meta.layout(widget)
end


wtextmap.setup=function(widget,def)

-- options about how we behave

	local opts=def.opts or {}
	widget.opts={}

	widget.class="textmap"
	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",scroll_pan="tiles",color=widget.color})

	widget.scroll_widget.datx.step=8
	widget.scroll_widget.daty.step=16
	widget.scroll_widget.datx.scroll=1
	widget.scroll_widget.daty.scroll=1
	
	widget.scroll_widget.pan.skin=wtextmap.pan_skin( widget.scroll_widget.pan )
	widget.scroll_widget.pan.textmap=widget

	widget.scroll_widget.pan.solid=true
	widget.scroll_widget.pan.can_focus=true

	widget.scroll_widget.pan.msg=wtextmap.msg
	widget.scroll_widget.pan.key=wtextmap.key
	widget.scroll_widget.pan.mouse=wtextmap.mouse

	widget.scroll_widget.pan.drag=function()end -- fake drag so we are treated as drag able

	widget.px=0
	widget.py=0

	return widget
end


	return wtextmap
end
