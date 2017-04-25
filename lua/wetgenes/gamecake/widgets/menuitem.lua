--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenuitem)
wmenuitem=wmenuitem or {}

function wmenuitem.update(widget)

	if not widget.hidden then
		if widget.hide_when_not then -- must stay over widget
			if not widget:isover(widget.hide_when_not) then
				widget.hidden=true
				widget.hide_when_not=nil
				widget.master:layout()
			end
		end
	end
	
	return widget.meta.update(widget)
end

function wmenuitem.draw(widget)
	return widget.meta.draw(widget)
end

function wmenuitem.menu_add(widget,opts)
	opts=opts or {}
	if type(opts)=="function" then opts=opts() end
	
	if not opts.top then
		if widget.menu then
			widget.menu:remove()
			widget.menu=nil
		end

		widget.menu=widget.master:add({
			class="menu",
			grid_size=opts.grid_size or widget.master.grid_size,
			color=opts.color or 0,
			style=opts.style or "button",
			skin=opts.skin or 0,
			solid=true,
			highlight="none",
			fbo=true,
		})

	end
	local top=opts.top or widget.menu
	local md=opts.menu_data or widget.menu_data
	if type(md)=="function" then md=md() end

	top.px,top.py=widget:get_master_xy(widget.hx*(widget.menu_px or 0),widget.hy*(widget.menu_py or 0))

	local func_text=opts.func_text or md.func_text or function(a) return tostring(a) end 
	for i,v in ipairs(md) do
		if type(v.menu_data)=="table" then -- copy opts down to submenu
			v.menu_data.func_text=v.menu_data.func_text or md.func_text
			v.menu_data.hooks    =v.menu_data.hooks     or md.hooks
		end
		top:add({
			class="menuitem",
			user=v.user,
			id=v.id,
			text=v.text or func_text(v),
			text_align="left_center",
			hooks     = v.hooks or opts.hooks     or md.hooks,

			hide_when_clicked=true,

			color     = opts.color     or md.color     or 0,
			style     = opts.style     or md.style     or "button",
			skin      = opts.skin      or md.skin      or 0,
			text_size = opts.text_size or md.text_size,
			menu_px   = opts.menu_px   or md.menu_px,
			menu_py   = opts.menu_py   or md.menu_py,
			menu_data = v.menu_data ,
		})
	end
	
	top.also_over={top,widget} -- include these widgets in over test
	top.hidden=false
	top.hide_when_not="menu"

	widget.master:layout()
	widget.master.focus=nil

	return top

end

function wmenuitem.hooks(hook,widget,dat)

local showmenu=function()

	if widget.menu_data and widget.master.menu~=widget then -- add a sub menu using this data

		widget.master.menu=widget

		local w=widget:menu_add(widget.menu_data)
		local map={}
		local bubble
		bubble=function(p)
			if not p then return end
			if map[p] then return end
			map[p]=true
			while p and p~=p.parent do
				if p.also_over then
					for i,v in pairs(p.also_over) do
						if not v.parent then p.also_over[i]=nil end -- its been deleted, clear it
					end
					for i,v in pairs(p.also_over) do
						bubble(v)
					end
					p.also_over[w]=w
				end
				p=p.parent
			end
		end
		bubble(w)

	end
end

local togglemenu=function()
	if widget.menu and ( not widget.menu.hidden ) then -- already visible so hide it
		widget.menu:remove()
		widget.menu=nil
		widget.master.menu=nil
	else
		showmenu()
	end
end


	if hook=="over" and widget.master.menu then
--		if widget.parent.class~="menubar" then -- menubar needs clicks
		if widget.parent.class=="menu" or widget.parent.class=="menubar" then -- only the menu?
			showmenu()
		end
--		end
	end

	if hook=="active" then
		if widget.parent.class=="menubar" then
			if widget.master.menu then
				widget.master.menu=nil
			else
				showmenu()
			end
		end
	end
	
	if hook=="click" then
	
		if widget.parent.class~="menubar" then
			widget.master.menu=nil
	
			if widget.menu_data then -- add a sub menu using this data
				
				if widget.parent.class~="menubar" then
					togglemenu()
				end

			elseif widget.hide_when_clicked then
				if widget.parent.class=="menu" then -- only the menu?
					widget.parent.hidden=true
					widget.parent.hide_when_not=nil
					widget.master:layout()
				end
	--		elseif widget.remove_when_clicked then
	--			if widget.parent.class=="menu" then -- only the menu?
	--				widget.parent:remove()
	--			end
			end
		end

	end
	
end

function wmenuitem.setup(widget,def)

	widget.class="menuitem"
	
	widget.update=wmenuitem.update
	widget.draw=wmenuitem.draw


	widget.solid=true
	widget.style=widget.style or "button"

	widget.class_hooks=wmenuitem.hooks
	
	widget.hide_when_clicked=def.hide_when_clicked
--	widget.remove_when_clicked=def.remove_when_clicked

	widget.menu_add=wmenuitem.menu_add

	widget.menu_px=def.menu_px or 1 -- where to display any sub menu
	widget.menu_py=def.menu_py or 0

	widget.menu_data=def.menu_data

	return widget
end

return wmenuitem
end
