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

function wslide.mouse(widget,act,_x,_y,keyname)

	local x,y=widget:mousexy(_x,_y)
	local tx=x-(widget.pan_px or 0)
	local ty=y-(widget.pan_py or 0)
	if tx>=0 and tx<widget.hx and ty>=0 and ty<widget.hy then
		local d=widget.data or widget.daty
		if d --[[and ( not widget.not_mousewheel )]] then
			if keyname=="wheel_add" and act==-1 then
				d:inc()
				return
			elseif keyname=="wheel_sub" and act==-1  then
				d:dec()
				return
			end
		end
	end
	return widget.meta.mouse(widget,act,_x,_y,keyname)
end

function wslide.update(widget)
--	local it=widget.slide

	if widget.master.active==widget  --[[and ( not widget.not_mousewheel )]] then
--print("slide update")
--			local ups=srecaps.ups()

			if widget.master.keyset["left"]  then
				widget.datx:dec()
			end
			if widget.master.keyset["right"] then
				widget.datx:inc()
			end
			if widget.master.keyset["up"]    then
				widget.daty:dec()
			end
			if widget.master.keyset["down"]  then
				widget.daty:inc()
			end
	end

	local oldx=widget.knob.px
	local oldy=widget.knob.py
	widget:snap()
	if oldx~=widget.knob.px or oldy~=widget.knob.py then
		widget:build_m4()
	end

	return widget.meta.update(widget)
end

function wslide.layout(widget)

	widget.knob.hx=widget.datx:get_size(widget.hx)
	widget.knob.hy=widget.daty:get_size(widget.hy)

	widget.meta.layout(widget)

end

function wslide.slide_snap(it,useloc)

	it.knob.hx=it.datx:get_size(it.hx)
	it.knob.hy=it.daty:get_size(it.hy)

	if not useloc then
		it.knob.px=it.datx:get_pos(it.hx,it.knob.hx,it.datxrev)
		it.knob.py=it.daty:get_pos(it.hy,it.knob.hy,it.datyrev)
	end

-- auto snap positions when draged
	it.knob.px=it.datx:snap( it.hx , it.knob.hx , it.knob.px , it.datxrev )

-- y is now the right way up
	it.knob.py=it.daty:snap( it.hy , it.knob.hy , it.knob.py , it.datyrev )

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
	widget.datx=widget.datx or widget_data.new_data({max=0,master=widget.master})
	widget.daty=widget.daty or widget_data.new_data({max=0,master=widget.master})

	-- shorthand "datx" or "daty" rather than repeating
	if type(widget.data)=="string" then widget.data=widget[widget.data] end

	widget.style=widget.style or "indent"

-- auto add the draging button as a child
	widget.knob=widget:add({style="button",class="drag",color=widget.color,solid=true,
		data=widget.data,skin=widget.skin})

-- set size and position of child
	widget.knob.hx=widget.datx:get_size(widget.hx)
	widget.knob.hy=widget.daty:get_size(widget.hy)
	widget.knob.px=widget.datx:get_pos(widget.hx,widget.knob.hx,widget.datxrev)
	widget.knob.py=widget.daty:get_pos(widget.hy,widget.knob.hy,widget.datyrev)
	widget:snap()

	widget.solid=false
--	widget.can_focus=true

	return widget
end

return wslide
end
