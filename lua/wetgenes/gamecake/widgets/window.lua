--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- scale or scroll the *SINGLE* child to fit within this panels size

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wwindow)

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

wwindow=wwindow or {}

function wwindow.mouse(widget,act,_x,_y,keyname)

-- check children first
	local ret=widget.meta.mouse(widget,act,_x,_y,keyname)
--[[
	if widget.panel_mode=="window" then -- window move?

		local x,y=widget.parent:mousexy(_x,_y)

		if widget.drag_mouse then
			if act==-1 and keyname=="left" then
				widget.drag_mouse=nil
			else
				widget.px=widget.drag_mouse[1]+x
				widget.py=widget.drag_mouse[2]+y
				widget:set_dirty()
				widget.meta.build_m4(widget)
			end
		else
			if act==1 and keyname=="left" then
				if widget.master.over==widget then --clicking on us?
					widget.drag_mouse={widget.px-x,widget.py-y}
					widget.parent:insert(widget) -- move to top
				end
			end
		end

print(ret,x,y,act,keyname)

	end
]]
	return ret
end

function wwindow.update(widget)
	return widget.meta.update(widget)
end

function wwindow.draw(widget)
	return widget.meta.draw(widget)
end

-- this is a magic layout that sizes panels

function wwindow.layout(widget)

	local v=widget.win_fbo
	if v then
		if widget.panel_mode=="scale" then -- maintain aspect

			v.sx=widget.hx/v.hx
			v.sy=widget.hy/v.hy

			if v.sx<v.sy then v.sy=v.sx else v.sx=v.sy end
			
			v.px=(widget.hx-v.hx*v.sx)/2
			v.py=(widget.hy-v.hy*v.sy)/2

		elseif widget.panel_mode=="stretch" then -- stretch to fit any area

			v.sx=widget.hx/v.hx
			v.sy=widget.hy/v.hy

		end
	end

-- also layout any other children
	widget.meta.layout(widget,1)

end

wwindow.win_hooks=function(widget,act,w)
--print(act,w.id)
	if act=="click" then
		if w.id=="win_hide" then
		
			widget.hidden=true
			
		elseif w.id=="win_grow" then
			widget.hx=widget.hx*1.5
			widget.hy=widget.hy*1.5
			widget:layout()
			widget:build_m4()

		elseif w.id=="win_shrink" then
			widget.hx=widget.hx/1.5
			widget.hy=widget.hy/1.5
			widget:layout()
			widget:build_m4()
		end
	end
end

function wwindow.setup(widget,def)

	widget.class="window"

	widget.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

--	widget.key=wwindow.key
	widget.mouse=wwindow.mouse
	widget.update=wwindow.update
	widget.draw=wwindow.draw
	widget.layout=wwindow.layout
	
	widget.win_hooks = function(act,w) return wwindow.win_hooks(widget,act,w) end

	widget.menu_data=widget.menu_data or {
		{	id="win_hide",		text="Hide Window",		},
		{	id="win_shrink",	text="Shrink Window",	},
		{	id="win_grow",		text="Grow Window",		},
		hooks=widget.win_hooks,
	}
	
	local ss=24
	local color=0




-- add all the trimmings
	widget.win_fbo=widget:add({
				hx=def.hx,
				hy=def.hy+ss,
				px=0,
				py=0,
--				class="fill",
				color=color,
				fbo=true,
				style="flat",
				skin=0,
				highlight="none",
				smode="topleft",
			})

	widget.win_canvas=widget.win_fbo:add({
				class="fill",
				px=0,
				py=ss,
				hx=def.hx,
				hy=def.hy,
				color=color,
				skin=0,
--				solid=true,
				highlight="none",
			})

	widget.win_menu=widget.win_fbo:add({
				class="menuitem",
				px=0,
				py=0,
				hx=ss,
				hy=ss,
				text=".",
				color=color,
				skin=0,
				solid=true,
				menu_data=widget.menu_data,
			})

	widget.win_title=widget.win_fbo:add({
				px=ss,
				py=0,
				hx=def.hx-ss*3,
				hy=ss,
				text=def.title or "...",
				color=color,
				skin=0,
			})

	widget.win_shrink=widget.win_fbo:add({
				px=def.hx-ss*2,
				py=0,
				hx=ss,
				hy=ss,
				text="-",
				color=color,
				skin=0,
				solid=true,
				hooks=widget.win_hooks,
				id="win_shrink",
			})

	widget.win_grow=widget.win_fbo:add({
				px=def.hx-ss,
				py=0,
				hx=ss,
				hy=ss,
				text="+",
				color=color,
				skin=0,
				solid=true,
				hooks=widget.win_hooks,
				id="win_grow",
			})

	widget.hy=def.hy+ss
	
	return widget
end

return wwindow
end
