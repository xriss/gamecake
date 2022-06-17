--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- a scrolling area, the widget is biger than display area but scroll bars allow you to see it all



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wscroll)
wscroll=wscroll or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")

function wscroll.mouse(widget,act,_x,_y,keyname)
	return widget.meta.mouse(widget,act,_x,_y,keyname)
end

function wscroll.update(widget)

	local pan=widget.pan
	
	local pan_px=widget.datx.num
	local pan_py=widget.daty.num
	
	if pan_px~=pan.pan_px or pan_py~=pan.pan_py then
	
--print("update",pan_px,pan_py)

		pan.pan_px=pan_px
		pan.pan_py=pan_py
		
		if pan.pan_refresh then pan:pan_refresh() end -- update
		pan:set_dirty()
		widget.meta.build_m4(widget)
	end

	return widget.meta.update(widget)
end

function wscroll.draw(widget)

--	local it=widget.scroll
	
	return widget.meta.draw(widget)
end

function wscroll.layout(widget)

-- fix child locations

	local ss=16
	if widget.hx<ss*2 then ss=widget.hx/2 end
	if widget.hy<ss*2 then ss=widget.hy/2 end	
	local s2=math.ceil(ss/2)

	widget.pan.hx=widget.hx-s2
	widget.pan.hy=widget.hy-s2
	widget.pan.px=0
	widget.pan.py=0

	widget.slidey.hx=s2
	widget.slidey.hy=widget.hy-s2
	widget.slidey.px=widget.hx-s2
	widget.slidey.py=0

	widget.slidex.hx=widget.hx
	widget.slidex.hy=s2
	widget.slidex.px=0
	widget.slidex.py=widget.hy-s2


--	local it=widget.scroll
	
	widget.pan:layout() -- creates hx_max,hy_max

--	widget.datx.step= widget:bubble("text_size") or 16
	widget.datx.max=math.ceil((widget.pan.hx_max-widget.pan.hx)/widget.datx.step)*widget.datx.step
	if widget.datx.max<0 then widget.datx.max=0  end
	widget.datx.size=widget.pan.hx/widget.pan.hx_max
	widget.datx:value() -- clamp

--	widget.daty.step= widget:bubble("text_size") or 16
	widget.daty.max=math.ceil((widget.pan.hy_max-widget.pan.hy)/widget.daty.step)*widget.daty.step
	if widget.daty.max<0 then widget.daty.max=0 end
	widget.daty.size=widget.pan.hy/widget.pan.hy_max
	widget.daty:value() -- clamp
	
--	widget.meta.layout(widget)
--	widget.meta.build_m4(widget)
end

function wscroll.setup(widget,def)
--	local it={}
--	widget.scroll=it
	widget.class="scroll"
	
	widget.key=wscroll.key
	widget.mouse=wscroll.mouse
	widget.update=wscroll.update
	widget.layout=wscroll.layout
	widget.draw=wscroll.draw

-- auto add the draging button as a child
	local ss=16
	if widget.hx<ss*2 then ss=widget.hx/2 end
	if widget.hy<ss*2 then ss=widget.hy/2 end
	local s2=math.ceil(ss/2)

	widget.datx=widget_data.new_data{max=1,master=widget.master}
	widget.daty=widget_data.new_data{max=1,master=widget.master}
	widget.solid=true
	
	widget.datx.step=1
	widget.datx.scroll=32
	widget.daty.step=1
	widget.daty.scroll=32
	
	widget.color=widget.color or 0
	
	widget.pan=		widget:add({class=def.scroll_pan or "pan",	hx=widget.hx-s2,	hy=widget.hy-s2	,color=widget.color})
	widget.slidey=	widget:add({class="slide",	hx=s2,				hy=widget.hy-s2,	px=widget.hx-s2,	py=0,
		daty=widget.daty,color=widget.color})
	widget.slidex=	widget:add({class="slide",	hx=widget.hx,	hy=s2,           	px=0,           	py=widget.hy-s2,
		datx=widget.datx,color=widget.color})

	return widget
end

return wscroll
end
