--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require




--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wpan)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")

wpan=wpan or {}

function wpan.mouse(widget,act,_x,_y,keyname)

	local x,y=widget:mousexy(_x,_y)
	local tx=x-(widget.pan_px or 0)
	local ty=y-(widget.pan_py or 0)
	if tx>=0 and tx<widget.hx and ty>=0 and ty<widget.hy then
		if widget and widget.parent and widget.parent.daty then
			if keyname=="wheel_add" and act==-1 then
				widget.parent.daty:dec()
				return
			elseif keyname=="wheel_sub" and act==-1  then
				widget.parent.daty:inc()
				return
			elseif keyname=="wheel_left" and act==-1 then
				widget.parent.datx:dec()
				return
			elseif keyname=="wheel_right" and act==-1  then
				widget.parent.datx:inc()
				return
			end
		end
	end
	return widget.meta.mouse(widget,act,_x,_y,keyname)
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
	widget.layout=wfill.layout
	
	widget.fbo=framebuffers.create(0,0,0)

--	widget.clip=true

	return widget
end

return wpan
end
