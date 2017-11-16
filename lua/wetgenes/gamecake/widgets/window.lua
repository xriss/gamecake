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

-- search upwards and return window,screen (so this could be a widget in the window)
-- screen should always be the widgets parent
	local window_screen=function(it)
		local window=it
		local screen=it.parent
		while it.parent ~= it do
			if it.class=="window" then window=it screen=it.parent end -- found window
			it=it.parent
			if it.class=="screen" then screen=it break end -- double check screen
		end
		return window,screen
	end
	wwindow.window_screen=window_screen -- export
	
	local winclamp=function(it)
		local window,screen=window_screen(it)
		local lpx,lpy,lhx,lhy=window.px,window.py,window.hx,window.hy
		if window.hx > screen.windows.hx then window.hx = screen.windows.hx end
		if window.hy > screen.windows.hy then window.hy = screen.windows.hy end
		if window.px<0    then window.px=0 end
		if window.py<0    then window.py=0 end
		if window.panel_mode=="scale" then -- maintain aspect
			local sx=window.hx/window.win_fbo.hx
			local sy=window.hy/window.win_fbo.hy
			local s=sx<sy and sx or sy
			if s<1/8 then s=1/8 end
			window.hx=window.win_fbo.hx*s
			window.hy=window.win_fbo.hy*s
		else
			local sx=window.hx/window.win_fbo.hx
			local sy=window.hy/window.win_fbo.hy
			if sx<1/8 then sx=1/8 end
			if sy<1/8 then sy=1/8 end
			window.hx=window.win_fbo.hx*sx
			window.hy=window.win_fbo.hy*sy
		end
		if window.px<0 then window.px=0 end
		if window.py<0 then window.py=0 end
		if window.px+window.hx>screen.windows.hx then window.px=screen.windows.hx-window.hx end
		if window.py+window.hy>screen.windows.hy then window.py=screen.windows.hy-window.hy end
		return window.px-lpx,window.py-lpy,window.hx-lhx,window.hy-lhy
	end


function wwindow.edge_drag(widget,x,y)

	local window,screen=window_screen(widget)

	if window.flags.nodrag then return end	

	local master=widget.master
	local active_xy=master.active_xy
	
	local windock= (window.parent.class=="windock") and window.parent or nil
	
	if windock and windock.windock~="drag" then -- we are docked
		return
	end

	screen.windows:insert(window) -- move to top

	if not active_xy.edge then -- fill in starting edge on the first call

		active_xy.edge=widget.id
		
		active_xy.wx,active_xy.wy=screen.windows:mousexy(active_xy.mx,active_xy.my)

		active_xy.px=window.px
		active_xy.py=window.py
		active_xy.hx=window.hx
		active_xy.hy=window.hy

	end

	local mx,my=screen.windows:mousexy(x,y)

	if	master.active_xy.edge=="win_edge_br" or 
		master.active_xy.edge=="win_edge_bl" or 
		master.active_xy.edge=="win_edge_tr" or 
		master.active_xy.edge=="win_edge_b"  or 
		master.active_xy.edge=="win_edge_r"  then
		
		if master.active_xy.edge=="win_edge_br" or master.active_xy.edge=="win_edge_r" or master.active_xy.edge=="win_edge_tr" then
			window.hx=active_xy.hx+(mx-active_xy.wx)
		end
		
		if master.active_xy.edge=="win_edge_br" or master.active_xy.edge=="win_edge_b" or master.active_xy.edge=="win_edge_bl" then
			window.hy=active_xy.hy+(my-active_xy.wy)
		end

	end

	if	master.active_xy.edge=="win_edge_tl" or
		master.active_xy.edge=="win_edge_tr" or
		master.active_xy.edge=="win_edge_bl" or
		master.active_xy.edge=="win_edge_t" or
		master.active_xy.edge=="win_edge_l" then
		
		if master.active_xy.edge=="win_edge_tl" or master.active_xy.edge=="win_edge_l" or master.active_xy.edge=="win_edge_bl" then
			window.px=active_xy.px+(mx-active_xy.wx)
			window.hx=active_xy.hx-(mx-active_xy.wx)
		end
		
		if master.active_xy.edge=="win_edge_tl" or master.active_xy.edge=="win_edge_t" or master.active_xy.edge=="win_edge_tr" then
			window.py=active_xy.py+(my-active_xy.wy)
			window.hy=active_xy.hy-(my-active_xy.wy)
		end

	end
	
	if window.hx > screen.windows.hx then window.hx=screen.windows.hx end
	if window.hy > screen.windows.hy then window.hy=screen.windows.hy end
	
	if window.panel_mode=="scale" then -- keep aspect when scaling

		local sx=window.hx/window.win_fbo.hx
		local sy=window.hy/window.win_fbo.hy

		if window.win_fbo.hx*sx > screen.windows.hx then sx=screen.windows.hx/window.win_fbo.hx end
		if window.win_fbo.hy*sx > screen.windows.hy then sx=screen.windows.hy/window.win_fbo.hy end
		if window.win_fbo.hx*sy > screen.windows.hx then sy=screen.windows.hx/window.win_fbo.hx end
		if window.win_fbo.hy*sy > screen.windows.hy then sy=screen.windows.hy/window.win_fbo.hy end

		local s=sx<sy and sx or sy
		if	master.active_xy.edge=="win_edge_t" or master.active_xy.edge=="win_edge_b" then s=sy end
		if	master.active_xy.edge=="win_edge_l" or master.active_xy.edge=="win_edge_r" then s=sx end

		
		window.hx=window.win_fbo.hx*s
		window.hy=window.win_fbo.hy*s

	end

	winclamp(window)

	window:set_dirty()	
	window:layout()
	window:build_m4()

end

function wwindow.drag(widget,x,y)

	local window,screen=window_screen(widget)
	local windock= (window.parent.class=="windock") and window.parent or nil

	if window.flags.nodrag then return end	
	
	if windock and windock.windock~="drag" then -- we are docked
		return
	end

	local master=widget.master


	local rx,ry=screen.windows:mousexy(x,y)
	local x,y=rx-master.active_xy[1],ry-master.active_xy[2]

--	local maxx=screen.hx-widget.hx
--	local maxy=screen.hy-widget.hy

	window.px=x
	window.py=y
	
--[[
	if widget.px<0    then widget.px=0 end
	if widget.px>maxx then widget.px=maxx end
	if widget.py<0    then widget.py=0 end
	if widget.py>maxy then widget.py=maxy end
	
	if screen.snap then
		screen:snap(true)
	end
]]

	local cpx,cpy,chx,chy=winclamp(window)
	if cpx*cpx > cpy*cpy then
		window.active_push={"x",cpx}
	else
		window.active_push={"y",cpy}
	end

	window:call_hook_later("slide")
	
	window:set_dirty()
	
	window:layout()
	window:build_m4()
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
	local window=widget
	local windock= (window.parent.class=="windock") and window.parent or nil

	
--	if windock and windock.windock=="drag" then -- we are dragable
			
	--	if window.hx > window.parent.hx then window.hx = window.parent.hx end
	--	if window.hy > window.parent.hy then window.hy = window.parent.hy end

		if v then
			if window.panel_mode=="scale" then -- maintain aspect

				v.sx=window.hx/v.hx
				v.sy=window.hy/v.hy

				if v.sx<v.sy then v.sy=v.sx else v.sx=v.sy end

				v.px=(window.hx-v.hx*v.sx)/2
				v.py=(window.hy-v.hy*v.sy)/2

			elseif window.panel_mode=="stretch" then -- stretch to fit any area

				v.sx=window.hx/v.hx
				v.sy=window.hy/v.hy

			end
		end
--	end	

-- also layout any other children
	widget.meta.layout(widget)

	local ss=(widget.master.grid_size or 24)
	local bar_height=widget.flags.nobar and 0 or ss

	local hy=widget.win_canvas.hy+bar_height
	if hy~=widget.win_fbo.hy then -- resize widgets
		widget.hy=hy
		widget.win_fbo.hy=hy

		widget.win_edge_l.hy=hy+ss
		widget.win_edge_r.hy=hy+ss
		
		widget.win_edge_b.py=hy-ss/8
		widget.win_edge_bl.py=hy-ss/4
		widget.win_edge_br.py=hy-ss/4
		

				
--		print(widget.win_canvas.hy)

		widget:build_m4()
		return wwindow.layout(widget)
	end

end

wwindow.window_hooks_reset=function(widget)
	widget.hx=widget.win_fbo.hx
	widget.hy=widget.win_fbo.hy
	winclamp(widget)
	widget:layout()
	widget:build_m4()
end

wwindow.move_to_top=function(window)
	local window,screen=window_screen(window)
	screen.windows:insert(window) -- move to top
end

wwindow.is_top=function(window)
	local window,screen=window_screen(window)
	return screen.windows[#screen.windows]==window
end

wwindow.window_hooks=function(_window,act,widget)
--print(act,w.id)

	local window,screen=window_screen(_window or widget)

	if window.flags and window.flags.nodrag then return end	

	if act=="active" then


		local windock= (window.parent.class=="windock") and window.parent or nil

--print(widget.id,widget.drag,wwindow.edge_drag)

		if widget.drag~=wwindow.edge_drag then -- do not undock when clicking on the drag widgets

			if windock and windock.windock~="drag" then -- we are docked so undock us
				window.active_nopush=true
				local master=screen.master
				screen:remove_split(window)
				wwindow.window_hooks_reset(window)
				master.active_xy={window.hx/2,window.hy/2,mx=0,my=0}
			end

		end

		if window.parent==screen.windows then
			screen.windows:insert(window) -- move to top
		end
		
--		print("ACTIVE",window.id)

	elseif act=="inactive" then

--[[		
		if window.active_push and math.abs(window.active_push[2])>16 then

--			print("PUSH",window.id,window.active_push[1],window.active_push[2])

			if not window.active_nopush then
				if window.parent.windock=="drag" then -- only dock if we are a dragable window
				
					local axis,order=window.active_push[1],(window.active_push[2]>=0) and 1 or 2
					local split
					local v=widget
					while v~=v.parent do
						if v.screen_split and v.split_axis==axis and v.split_order==order then split=v break end
						v=v.parent
					end

					if split then
						local dock=(split[1].class=="windock" and split[1]~=screen.windows) and split[1] or split[2] -- pick the dock from the screen_split
						dock:insert(window)
					else
						screen:add_split({
							internal=true, -- add split inside
							window=window,
							split_axis=window.active_push[1],
							split_order=(window.active_push[2]>=0) and 1 or 2
						})
					end
				end
			end

		end
		window.active_nopush=nil

		window.master:layout()
		
--		print("INACTIVE",window.id)
]]

	elseif act=="click" then -- turn a click into another act
	
		act=widget.id

	end

-- these acts can be used by outside code
-- eg
-- window.window_hooks("win_hide")

	if act=="win_hide" then
	
		window.hidden=true -- hide it
		window.master:call_descendents(function(w) w:set_dirty() end)
		window.master:resize_and_layout()
		
	elseif act=="win_show" then
	
		window.hidden=false
		window:move_to_top()
		window.master:call_descendents(function(w) w:set_dirty() end)
		window.master:resize_and_layout()

	elseif act=="win_toggle" then
	
		if window.hidden then -- show it
		
			window.hidden=false
			window:move_to_top()
			window.master:call_descendents(function(w) w:set_dirty() end)
			window.master:resize_and_layout()

		else

			window.hidden=true -- hide it
			window.master:call_descendents(function(w) w:set_dirty() end)
			window.master:resize_and_layout()

		end
		
	elseif act=="win_toggle_other" then
	
		local win=window.master.ids[widget.user]
		if win then
			win.window_hooks("win_toggle")
		end
		
	elseif act=="win_grow" then

		window.hx=window.hx*1.5
		window.hy=window.hy*1.5
		winclamp(window)
		window:layout()
		window:build_m4()

	elseif act=="win_shrink" then

		window.hx=window.hx/1.5
		window.hy=window.hy/1.5
		winclamp(window)
		window:layout()
		window:build_m4()

	elseif act=="win_reset" then

		wwindow.window_hooks_reset(window)

	end

end


function wwindow.setup(widget,def)

	widget.flags=def.flags or {}

	widget.class="window"

	widget.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

--	widget.key=wwindow.key
--	widget.mouse=wwindow.mouse
	widget.drag=wwindow.drag
	widget.update=wwindow.update
	widget.draw=wwindow.draw
	widget.layout=wwindow.layout
	widget.move_to_top=wwindow.move_to_top
	widget.is_top=wwindow.is_top
	
	widget.window_hooks = function(act,w) return wwindow.window_hooks(widget,act,w) end

	widget.window_menu=function()
		local window,screen=wwindow.window_screen(widget)
		return screen:window_menu()
	end
	widget.menu_data=widget.menu_data or {
		{	id="win_hide",		text="Hide Window",		},
		{	id="win_reset",		text="Reset Window Size",	},
		{	id="win_shrink",	text="Shrink Window Size",	},
		{	id="win_grow",		text="Grow Window Size",		},
		{	id="win_windows",	text="Windows...",menu_data=widget.window_menu},
		hooks=widget.window_hooks,
	}
	
	local ss=widget.master.grid_size or 24
	local color=0

	local ss1=ss/24
	local ss_side=ss/8
	local ss_corner=ss/4

	local bar_height=widget.flags.nobar and 0 or ss

--	widget.outline_size=-ss/16
--	widget.outline_color=0xcc000000

	widget.win_scale=def.win_scale or 0


-- add all the trimmings
	widget.win_fbo=widget:add({
				hx=def.hx,
				hy=def.hy+bar_height,
				px=0,
				py=0,
--				class="fill",
				color=color,
				fbo=true,
				style="flat",
				highlight="none",
				smode="topleft",
				outline_size=ss/8,
				outline_color=0x44000000,
				outline_fade_color=0x00000000,
			})

	widget.win_canvas=widget.win_fbo:add({
				class="fill",
				px=0,
				py=bar_height,
				hx=def.hx,
				hy=def.hy,
				size="fit",
				color=color,
				highlight="none",
			})

if bar_height>0 then
	widget.win_three=widget.win_fbo:add({
				px=0,
				py=0,
				hx=def.hx,
				hy=ss,
				class="three",
			})

	widget.win_menu=widget.win_three:add({
				class="menuitem",
				px=0,
				py=0,
				hx=ss,
				hy=ss,
				text=">",
				color=color,
				solid=true,
				menu_data=widget.menu_data,
				cursor="hand",
			})

	widget.win_title=widget.win_three:add({
				px=0,
				py=0,
				hx=def.hx,
				hy=ss,
				text=def.title or "...",
			})

--[[
	widget.win_shrink=widget.win_fbo:add_indent({
				px=def.hx-ss*2,
				py=0,
				hx=ss,
				hy=ss,
				text="-",
				color=color,
				solid=true,
				hooks=widget.window_hooks,
				id="win_shrink",
				cursor="hand",
			},ss1)

	widget.win_grow=widget.win_fbo:add_indent({
				px=def.hx-ss,
				py=0,
				hx=ss,
				hy=ss,
				text="+",
				color=color,
				solid=true,
				hooks=widget.window_hooks,
				id="win_grow",
				cursor="hand",
			},ss1)
]]
end

	widget.win_edge_l=widget.win_fbo:add({
				px=-ss/8,
				py=0,
				hx=ss/4,
				hy=def.hy+bar_height,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_l",
				cursor="sizewe",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_r=widget.win_fbo:add({
				px=def.hx-ss/8,
				py=0,
				hx=ss/4,
				hy=def.hy+bar_height,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_r",
				cursor="sizewe",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_t=widget.win_fbo:add({
				px=0,
				py=-ss/8,
				hx=def.hx,
				hy=ss/4,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_t",
				cursor="sizens",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_b=widget.win_fbo:add({
				px=0,
				py=def.hy+bar_height-ss/8,
				hx=def.hx,
				hy=ss/4,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_b",
				cursor="sizens",
				drag=wwindow.edge_drag,
			})

	widget.win_edge_tl=widget.win_fbo:add({
				px=-ss/4,
				py=-ss/4,
				hx=ss/2,
				hy=ss/2,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_tl",
				cursor="sizenwse",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_tr=widget.win_fbo:add({
				px=def.hx-ss/4,
				py=-ss/4,
				hx=ss/2,
				hy=ss/2,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_tr",
				cursor="sizenesw",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_bl=widget.win_fbo:add({
				px=-ss/4,
				py=def.hy+bar_height-ss/4,
				hx=ss/2,
				hy=ss/2,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_bl",
				cursor="sizenwse",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_br=widget.win_fbo:add({
				px=def.hx-ss/4,
				py=def.hy+bar_height-ss/4,
				hx=ss/2,
				hy=ss/2,
				solid=true,
				hooks=widget.window_hooks,
				id="win_edge_br",
				cursor="sizenesw",
				drag=wwindow.edge_drag,
			})


	widget.hx=def.hx
	widget.hy=def.hy+bar_height
	
	return widget
end

return wwindow
end
