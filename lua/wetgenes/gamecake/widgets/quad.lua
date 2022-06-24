--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wquad)

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")
local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wquad=wquad or {}

function wquad.update(widget)

	local oldx=widget.knob.px
	local oldy=widget.knob.py
	widget:snap()
	if oldx~=widget.knob.px or oldy~=widget.knob.py then
		widget:build_m4()
	end

	return widget.meta.update(widget)
end

function wquad.draw(widget)
	return widget.meta.draw(widget)
end

function wquad.layout(widget)

	local fx=widget.datx:value()
	local fy=widget.daty:value()

	widget[1].px=0
	widget[1].py=0
	widget[1].hx=widget.hx*fx
	widget[1].hy=widget.hy*fy

	widget[2].px=widget.hx*fx
	widget[2].py=0
	widget[2].hx=widget.hx*(1-fx)
	widget[2].hy=widget.hy*fy

	widget[3].px=0
	widget[3].py=widget.hy*fy
	widget[3].hx=widget.hx*fx
	widget[3].hy=widget.hy*(1-fy)

	widget[4].px=widget.hx*fx
	widget[4].py=widget.hy*fy
	widget[4].hx=widget.hx*(1-fx)
	widget[4].hy=widget.hy*(1-fy)


	widget.meta.layout(widget)

end

function wquad.snap(it,useloc)


	if not useloc then
		it.knob.px=it.datx:get_pos(it.hx,it.knob.hx,it.datxrev)
		it.knob.py=it.daty:get_pos(it.hy,it.knob.hy,it.datyrev)
	end
	
-- auto snap positions when draged
	it.knob.px=it.datx:snap( it.hx , it.knob.hx , it.knob.px , it.datxrev )
	
-- y is now the right way up
	it.knob.py=it.daty:snap( it.hy , it.knob.hy , it.knob.py , it.datyrev )
	
end



function wquad.setup(widget,def)

	local ss=widget.master.theme.grid_size

	widget.class="quad"
	
	widget.snap=wquad.snap

	widget.update=wquad.update
	widget.draw=wquad.draw
	widget.layout=wquad.layout
	
	widget.datx=widget.datx or widget_data.new_data({max=1,num=0.5,master=widget.master})
	widget.daty=widget.daty or widget_data.new_data({max=1,num=0.5,master=widget.master})

	widget:add({})
	widget:add({})
	widget:add({})
	widget:add({})

	widget.knob=widget:add({style="button",class="drag",solid=true,
		hx=ss/2,hy=ss/2,
		data=widget.data,skin=widget.skin})


	return widget
end

return wquad
end
