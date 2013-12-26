--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- an fbo cached area, that is an area which should always be drawn to a special buffer
-- and then the special buffer should be displayed



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wpan)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

wpan=wpan or {}

function wpan.mouse(widget,act,x,y,key)
--	widget.master.focus=widget
	return widget.meta.mouse(widget,act,x,y,key)
end


function wpan.key(widget,ascii,key,act)
	return widget.meta.key(widget,ascii,key,act)
end


function wpan.update(widget)
	return widget.meta.update(widget)
end

function wpan.draw(widget)
	return widget.meta.draw(widget)
end


function wpan.setup(widget,def)
--	local it={}
--	widget.pan=it
	widget.class="pan"
	
	widget.pan_px=0
	widget.pan_py=0
--	widget.clip=true
	
	widget.key=wpan.key
	widget.mouse=wpan.mouse
	widget.update=wpan.update
	widget.draw=wpan.draw
	
--	widget.fbo=_G.win.fbo(0,0,0)
	widget.fbo=framebuffers.create(0,0,0)

--	widget.clip=true

	return widget
end

return wpan
end
