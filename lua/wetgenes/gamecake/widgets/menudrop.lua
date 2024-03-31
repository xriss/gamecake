--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenudrop)
wmenudrop=wmenudrop or {}

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font

local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

function wmenudrop.mouse(widget,act,_x,_y,keyname)

	if widget.master.old_over==widget and widget.data then
		if keyname=="wheel_add" and act==-1 then
			widget.data:inc()
			return
		elseif keyname=="wheel_sub" and act==-1  then
			widget.data:dec()
			return
		end
	end

	return widget.meta.mouse(widget,act,_x,_y,keyname)
end

function wmenudrop.update(widget)
	if not widget.def.text then -- auto text only if not given
		if widget.data and widget.data.class=="list" then
			widget.text=widget.data.list[ widget.data.num ].str
		end
	end
	return widget.meta.update(widget)
end

function wmenudrop.draw(widget)
	return widget.meta.draw(widget)
end

function wmenudrop.drop(widget)

	local def=widget.def
	
	if widget.data then -- build a menu for this data
	
		local hooks=function(hook,w,dat)
			if hook=="click" or hook=="release" then
				widget.data:value(w.id)
				widget:update()
				widget:call_hook_later("menudrop",{value=widget.data:value()}) -- tell the master widget that we dropped and changed
			end
		end
		local d={}
		for i,v in ipairs(widget.data and widget.data.list or {}) do
			d[#d+1]={
				id=i,
				user=v.user,
				text=v.str,
				hooks=hooks
			}
		end
		widget.menu=widgets_menuitem.menu_add(widget,{menu_data=d})
	
	elseif widget.menu_data then -- use this custom menu
		widget.menu=widgets_menuitem.menu_add(widget,widget.menudata)
	end

end



function wmenudrop.class_hooks(hook,widget,dat)
	if widget.drop then
		if hook==widget.drop then
			wmenudrop.drop(widget)
		end
	else
		if hook=="active" or hook=="over" then
			wmenudrop.drop(widget)
		end
	end
end


function wmenudrop.setup(widget,def)

	widget.def=def

	widget.class="menudrop"
	
	widget.mouse=wmenudrop.mouse
	widget.update=wmenudrop.update
	widget.draw=wmenudrop.draw
	widget.layout=wmenudrop.layout

	widget.solid=true
	widget.style=widget.style or "button"

	widget.menu_px=def.menu_px or 1 -- where to drop
	widget.menu_py=def.menu_py or 0

	widget.menu_data=def.menu_data -- data to display

	widget.class_hooks={wmenudrop.class_hooks}
	
	widget.fbo=widget.fbo or framebuffers.create(0,0,0)

	return widget
end

return wmenudrop
end
