--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmenuitem)
wmenuitem=wmenuitem or {}

function wmenuitem.update(widget)


	if not widget.hidden then
		if widget.hide_when_not and not widget.master.press then -- must stay over widget unless holding button
			if widget:isover(widget.hide_when_not) then
				widget.over_time=wwin.time()
			elseif (not widget.over_time) or (wwin.time() >= widget.over_time+0.25) then -- delay hide
				widget.hidden=true
				widget.hide_when_not=nil
				widget.master.request_layout=true
--				widget.master:layout()
			end
		end
	end

	return widget.meta.update(widget)
end

function wmenuitem.draw(widget)
	return widget.meta.draw(widget)
end

function wmenuitem.draw_text(widget,opts)

	local gl=oven.gl
	local font=oven.cake.canvas.font

	local text=widget.text or ""
	local text_right=""

	if widget.menu_data then
		if widget.parent.class~="menubar" then
			text_right="->"
		end
	elseif widget.action and widget.action.key then -- key
		text_right=widget.action.key
	end

	local gs=widget.grid_size or widget.parent.grid_size or widget.master.grid_size or font.size*1.5

	if opts.size then
		local x=text_right~="" and gs*2 or gs*1
		return font.width( text..text_right ) + x, gs
	end

	local w=font.width(text)
	local tx=gs*0.5+(opts.txp or 0)
	local ty=(widget.hy/2)+(opts.typ or 0)
	if widget.parent.class=="menubar" then
		tx=(widget.hx-w)*0.5+(opts.txp or 0)
	end

	gl.Color( unpack(widget.master.get_color(nil,widget.text_color)) )

	font.set_xy(tx,ty)
	font.draw(text)
	
	if text_right~="" then
--		font.set_size(font.size*0.8,0)
		local w=font.width(text_right)
		local tx=widget.hx-w-(gs*0.5)+(opts.txp or 0)
		font.set_xy(tx,ty)
		font.draw(text_right)
	end
	
end

function wmenuitem.menu_add(widget,opts)

	opts=opts or {}
	if type(opts)=="function" then opts=opts() end
	local md=opts.menu_data or widget.menu_data
	if type(md)=="function" then md=md() end
	
	if md.inherit then -- perform inheritence copies
		local bubble_menudata
		bubble_menudata=function(md)
			for i,v in ipairs(md) do
				if type(v.menu_data)=="table" then -- copy string values down into sub menu tables
					for n,m in pairs(md) do
						if type(n)=="string" then -- skip top_* flags
							if "top_"~=string.sub(1,4) then
								v.menu_data[n]=v.menu_data[n] or m
							end
						end
					end
					bubble_menudata(v.menu_data)
				end
			end
		end
		bubble_menudata(md)
	end
	
	local window,screen=widget:window_screen()

	if not opts.top then

		if widget.menu then
			widget.menu:remove()
			widget.menu=nil
		end

		local ss=opts.grid_size or md.grid_size or widget.master.theme.grid_size

--		local screen;widget.master:call_descendents(function(it) if it.class=="screen" then screen=it end end)
--		screen=screen or widget.master
		
		widget.menu=screen:add({
			class="menu",
			grid_size=ss,
			color=opts.color or md.color or 0,
			style=opts.style or md.style or "button",
			skin= opts.skin  or md.skin  or 0,
			solid=true,
			cursor="hand",
			highlight="none",
			fbo=true,
			smode="topleft",
			outline_size=ss/8,
			outline_color=0x44000000,
			outline_fade_color=0x00000000,
		})
		
	end
	local top=opts.top or widget.menu
	top.px,top.py=widget:get_master_xy(opts.px or widget.hx*(widget.menu_px or 0),opts.py or widget.hy*(widget.menu_py or 0))

-- slight nudge to prevent dropdown gap
	top.px=math.floor(top.px-2)
	top.py=math.floor(top.py-2)

	top.window=window
	top.screen=screen


	for i,v in ipairs(md) do
	
		local it={} for a,b in pairs(v) do it[a]=b end

		it.class="menuitem"
		it.action = it.action or widget.master.get_action(it.id,it.user) -- lookup action
		it.text=it.text or ( it.action and it.action.text ) -- use action text if text is missing
		it.draw_text=it.draw_text or opts.draw_text or md.draw_text or wmenuitem.draw_text
		it.text_align=it.text_align or "left"
		it.hooks=it.hooks or opts.hooks     or md.hooks
		it.hide_when_clicked=it.hide_when_clicked or true
		it.color     = it.color       or opts.color     or md.color     or 0
		it.style     = it.style       or opts.style     or md.style     or "button"
		it.skin      = it.skin        or opts.skin      or md.skin      or 0
		it.cursor    = it.cursor      or opts.cursor    or md.cursor    or "hand"
		it.text_size = it.text_size   or opts.text_size or md.text_size
		it.menu_px   = it.menu_px     or opts.menu_px   or md.menu_px
		it.menu_py   = it.menu_py     or opts.menu_py   or md.menu_py

		local fixup=it.fixup or opts.fixup or md.fixup
		if fixup then fixup(it) end

		top:add(it)
	end
	
	top.also_over={top,widget} -- include these widgets in over test
	top.hidden=false
	top.hide_when_not="menu"

	widget.master.request_layout=true
--	widget.master:layout()
--	widget.master.focus=nil

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
				p.also_over[top]=top
			end
			p=p.parent
		end
	end
	bubble(top)

	widget.master:call_descendents(function(w)
		if w.menu and not w.menu.hidden then
-- if not part of this menu chain then hide the menu
			if not w.menu:isover(top) then
				w.menu.hidden=true
			end
		end
	end)


	return top

end

function wmenuitem.hooks(hook,widget,dat)

local showmenu=function()

	if widget.menu_data then -- add a sub menu using this data
		
		local w=widget:menu_add(widget.menu_data)

	end
end

local showmenu_delay=function()

	local t=wwin.time()

	local f
	f=function()
		if widget.master.over==widget then -- must hover
			if wwin.time() >= t+0.25 then
					showmenu()
			else
				widget.master.later_append(f) -- check again later
			end
		end
	end
	widget.master.later_append(f)

end

local togglemenu=function()
	if widget.menu and ( not widget.menu.hidden ) then -- already visible so hide it
		widget.menu:remove()
		widget.menu=nil
	else
		showmenu()
	end
end


	if hook=="over" then
		if widget.master.press then
			showmenu()
		else
			showmenu_delay()
		end
	end

	if hook=="active" then
		if widget.parent.class=="menubar" then
			showmenu()
		end
	end
	
	if hook=="click" or hook=="release" then
	
		if widget.parent.class~="menubar" then
	
			if widget.menu_data then -- add a sub menu using this data
				
				if widget.parent.class~="menubar" then
					togglemenu()
				end

			elseif widget.hide_when_clicked then
				if widget.parent.class=="menu" then -- only the menu?
					widget.parent.hidden=true
					widget.parent.hide_when_not=nil
					widget.master.request_layout=true
--					widget.master:layout()
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

	widget.class_hooks={wmenuitem.hooks}
	
--	widget.hide_when_clicked=widget.hide_when_clicked
--	widget.remove_when_clicked=widget.remove_when_clicked

	widget.menu_add=wmenuitem.menu_add

	if widget.top_menu then -- part of top menu bar
		widget.menu_px=widget.menu_px or 0 -- where to display any sub menu
		widget.menu_py=widget.menu_py or 1
	else
		widget.menu_px=widget.menu_px or 1 -- where to display any sub menu
		widget.menu_py=widget.menu_py or 0
	end
	
	return widget
end

return wmenuitem
end
