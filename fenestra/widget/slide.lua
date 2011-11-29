-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- two basic widgets merged together to give a simple slide or scrollbar 


module("fenestra.widget.slide")

local widget_data=require("fenestra.widget.data")


function mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function update(widget)
	local it=widget.slide


	it:snap()

--	it.drag.text=it.datx:get_string()
	
	return widget.meta.update(widget)
end


function slide_snap(it)

-- auto snap positions when draged
	it.drag.px=it.datx:snap( it.widget.hx , it.drag.hx , it.drag.px )
	it.drag.pxd=it.widget.pxd+it.drag.px
	
-- upside down y so need to twiddle it, pyr is "the right way up"
	it.drag.py=it.daty:snap( it.widget.hy , it.drag.hy , it.drag.py )
	it.drag.pyd=it.widget.pyd-it.drag.py
	
end
	
function setup(widget,def)
	local it={} -- our main data so as not to clobber widget values
	it.widget=widget
	it.snap=slide_snap
	widget.slide=it
	widget.class="slide"
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update
	
--setup constraints in x and y 
	it.datx=widget_data.new_data(def.datx)
	it.daty=widget_data.new_data(def.daty)

-- auto add the draging button as a child
	it.drag=widget:add({class="drag",color=0xffffffff,hy=it.daty:get_size(widget.hy),hx=it.datx:get_size(widget.hx),pxf=0,pyf=0,data=widget.data})

	return widget
end
