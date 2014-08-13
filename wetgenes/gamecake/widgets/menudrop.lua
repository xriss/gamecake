--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenudrop)
wmenudrop=wmenudrop or {}

local cake=oven.cake
local canvas=cake.canvas
local font=canvas.font

function wmenudrop.update(widget)
	if widget.data and widget.data.class=="list" then
		widget.text=widget.data.list[ widget.data.num ].str
	end
	return widget.meta.update(widget)
end

function wmenudrop.draw(widget)
	return widget.meta.draw(widget)
end

function wmenudrop.drop(widget)

	local def=widget.def

	if widget.master.menu then
		widget.master.menu:remove()
		widget.master.menu=nil
	end
	
	widget.master.menu=widget.master:add({
		class="menu",
		color=def.color or 0xffaaaaaa,
		style=def.style or "button",
		skin=def.skin or 1,
		solid=true,
		highlight="none",
	})
	
	local top=widget.master.menu
--	top:clean_all()
	
	top.px,top.py=widget:get_master_xy(0,widget.hy)
	
	local hooks=function(hook,w,dat)
		if hook=="click" then
			widget.data:value(w.id)
			widget:update()
		end
	end

	for i,v in ipairs(widget.data and widget.data.list or {}) do
		top:add({
			class="menuitem",
			user=v.user,
			id=i,
			text=v.str,

			hide_when_clicked=true,

			color=def.color or 0xffcccccc,
			style=def.style or "button",
			skin=def.skin or 1,
			text_size=def.text_size or 16,
			hooks=hooks,
		})
	end
	
	top.also_over={top,widget} -- include these widgets in over test
	top.hidden=false
	top.hide_when_not_over=true


	widget.master:layout()
	widget.master.focus=nil
		
end



function wmenudrop.class_hooks(hook,widget,dat)
	if hook=="active" then
		wmenudrop.drop(widget)
	end
	
end


function wmenudrop.setup(widget,def)

	widget.def=def

	widget.class="menudrop"
	
	widget.update=wmenudrop.update
	widget.draw=wmenudrop.draw
	widget.layout=wmenudrop.layout

	widget.solid=true

	widget.class_hooks=wmenudrop.class_hooks
	
	return widget
end

return wmenudrop
end
