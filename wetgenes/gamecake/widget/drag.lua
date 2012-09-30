-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.widget.drag")

function bake(state,wdrag)
wdrag=wdrag or {}

function wdrag.mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function wdrag.key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function wdrag.update(widget)

	if widget.data then
		widget.text=widget.data:get_string()
	end

	return widget.meta.update(widget)
end

function wdrag.draw(widget)
	return widget.meta.draw(widget)
end


function wdrag.setup(widget,def)
--	local it={}
--	widget.drag=it
	widget.class="drag"
	
	widget.key=key
	widget.mouse=mouse
	widget.update=update
	widget.draw=draw

	return widget
end

return wdrag
end
