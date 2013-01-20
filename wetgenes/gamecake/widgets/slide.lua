-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- two basic widgets merged together to give a simple slide or scrollbar 


module("wetgenes.gamecake.widgets.slide")

function bake(oven,wslide)
wslide=wslide or {}

local widget_data=oven.rebake("wetgenes.gamecake.widgets.data")


function wslide.mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function wslide.key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
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

-- auto add the draging button as a child
	widget.drag=widget:add({class="drag",color=widget.color,hy=widget.daty:get_size(widget.hy),hx=widget.datx:get_size(widget.hx),pxf=widget.datx:get_pos(),pyf=widget.daty:get_pos(),data=widget.data})
	
	return widget
end

return wslide
end
