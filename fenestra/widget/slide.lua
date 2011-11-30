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
--	local it=widget.slide


	widget:snap()

--	it.drag.text=it.datx:get_string()
	
	return widget.meta.update(widget)
end


function slide_snap(it)

-- auto snap positions when draged
	it.drag.px=it.datx:snap( it.hx , it.drag.hx , it.drag.px )
	it.drag.pxd=it.pxd+it.drag.px
	
-- upside down y so need to twiddle it, pyr is "the right way up"
	it.drag.py=it.daty:snap( it.hy , it.drag.hy , it.drag.py )
	it.drag.pyd=it.pyd-it.drag.py
	
end
	
function setup(widget,def)
--	local it={} -- our main data so as not to clobber widget values
--	it.widget=widget
--	widget.slide=it
	widget.class="slide"
	
	widget.snap=slide_snap
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update
	
--setup constraints in x and y 
	widget.datx=widget_data.new_data(def.datx)
	widget.daty=widget_data.new_data(def.daty)

-- auto add the draging button as a child
	widget.drag=widget:add({class="drag",color=0xffffffff,hy=widget.daty:get_size(widget.hy),hx=widget.datx:get_size(widget.hx),pxf=widget.datx:get_pos(),pyf=widget.daty:get_pos(),data=widget.data})
	
	return widget
end
