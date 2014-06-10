--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- two basic widgets merged together to give a simple slide or scrollbar 


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wslide)
wslide=wslide or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")
local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")

function wslide.update(widget)
--	local it=widget.slide

	if widget.master.active==widget then
--print("slide update")
			local ups=srecaps.ups()

			if ups.button("left_set")  then
				widget.datx:dec()
			end
			if ups.button("right_set") then
				widget.datx:inc()
			end
			if ups.button("up_set")    then
				widget.daty:dec()
			end
			if ups.button("down_set")  then
				widget.daty:inc()
			end
	end
	
	local oldx=widget.drag.px
	local oldy=widget.drag.py
	widget.drag.px=widget.datx:get_pos(widget.hx,widget.drag.hx)
	widget.drag.py=widget.daty:get_pos(widget.hy,widget.drag.hy)
	widget:snap()
	if oldx~=widget.drag.px or oldy~=widget.drag.py then
		widget:build_m4()
	end
	
	return widget.meta.update(widget)
end

function wslide.layout(widget)

	widget.drag.hx=widget.datx:get_size(widget.hx)
	widget.drag.hy=widget.daty:get_size(widget.hy)

	widget.meta.layout(widget)	
	
end

function wslide.slide_snap(it)

	it.drag.hx=it.datx:get_size(it.hx)
	it.drag.hy=it.daty:get_size(it.hy)

-- auto snap positions when draged
	it.drag.px=it.datx:snap( it.hx , it.drag.hx , it.drag.px )
	it.drag.pxd=it.pxd+it.drag.px
	
-- y is now the right way up
	it.drag.py=it.daty:snap( it.hy , it.drag.hy , it.drag.py )
	it.drag.pyd=it.pyd+it.drag.py
	
end
	
function wslide.setup(widget,def)
--	local it={} -- our main data so as not to clobber widget values
--	it.widget=widget
--	widget.slide=it
	widget.class="slide"
	
	widget.snap=wslide.slide_snap
	
	widget.key=wslide.key
	widget.mouse=wslide.mouse
	widget.update=wslide.update
	widget.layout=wslide.layout
	
--setup constraints in x and y 
	widget.datx=def.datx or widget_data.new_data({max=0})
	widget.daty=def.daty or widget_data.new_data({max=0})
	widget.data=def.data -- or def.datx or def.daty
	
	widget.style=def.style or "indent"

-- auto add the draging button as a child
	widget.drag=widget:add({style="button",class="drag",color=widget.color,solid=true,
		data=widget.data})

-- set size and position of child
	widget.drag.hx=widget.datx:get_size(widget.hx)
	widget.drag.hy=widget.daty:get_size(widget.hy)
	widget.drag.px=widget.datx:get_pos(widget.hx,widget.drag.hx)
	widget.drag.py=widget.daty:get_pos(widget.hy,widget.drag.hy)
	widget:snap()

	widget.solid=true
--	widget.can_focus=true
	
	return widget
end

return wslide
end
