-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- two basic widgets merged together to give a simple slide or scrollbar 


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wslide)
wslide=wslide or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")


function wslide.mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	if act==1 and widget.master.over==widget then
--print(x,y,widget.drag.px,widget.drag.py)
		if x<widget.drag.px then widget:key("","left",1) return
		elseif x>widget.drag.px then widget:key("","right",1) return
		elseif y<widget.drag.py then widget:key("","up",1) return
		elseif y>widget.drag.py then widget:key("","down",1) return
		end
	end
	return widget.meta.mouse(widget,act,x,y,key)
end


function wslide.key(widget,ascii,key,act)

	if key=="enter" or key=="return" or key=="space" then
		
		if act==-1 then -- ignore repeats on enter key
			widget:call_hook("click")				
			widget.master.focus=nil
		end
			
	elseif key=="left" then
		if act==1 then -- ignore repeats on enter key
			widget.datx:dec()
			widget.drag.px=widget.datx:get_pos(widget.hx,widget.drag.hx)
			widget:snap()
		end
	elseif key=="right" then
		if act==1 then -- ignore repeats on enter key
			widget.datx:inc()
			widget.drag.px=widget.datx:get_pos(widget.hx,widget.drag.hx)
			widget:snap()
		end
	elseif key=="up" then
		if act==1 then -- ignore repeats on enter key
			widget.daty:dec()
			widget.drag.py=widget.daty:get_pos(widget.hy,widget.drag.hy)
			widget:snap()
		end
	elseif key=="down" then
		if act==1 then -- ignore repeats on enter key
			widget.daty:inc()
			widget.drag.py=widget.daty:get_pos(widget.hy,widget.drag.hy)
			widget:snap()
		end
	end

	return true

--	return widget.meta.key(widget,ascii,key,act)
end


function wslide.update(widget)
--	local it=widget.slide


	widget:snap()

--	it.drag.text=it.datx:get_string()
	
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

	widget.can_focus=true
	
	return widget
end

return wslide
end
