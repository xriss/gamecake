--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- scale or scroll the *SINGLE* child to fit within this panels size

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wwindow)

local font=oven.cake.canvas.font

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

wwindow=wwindow or {}

	local winclamp=function(it)
		local window,screen=it:window_screen()
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
		elseif window.panel_mode=="stretch" then -- stretch aspect
			local sx=window.hx/window.win_fbo.hx
			local sy=window.hy/window.win_fbo.hy
			if sx<1/8 then sx=1/8 end
			if sy<1/8 then sy=1/8 end
			window.hx=window.win_fbo.hx*sx
			window.hy=window.win_fbo.hy*sy
		elseif window.panel_mode=="fill" then
			local ss=(it.master.theme.grid_size)
			if window.hx<ss*2 then window.hx=ss*2 end
			if window.hy<ss*2 then window.hy=ss*2 end
		end
		if window.px<0 then window.px=0 end
		if window.py<0 then window.py=0 end
		if window.px+window.hx>screen.windows.hx then window.px=screen.windows.hx-window.hx end
		if window.py+window.hy>screen.windows.hy then window.py=screen.windows.hy-window.hy end
		return window.px-lpx,window.py-lpy,window.hx-lhx,window.hy-lhy
	end


function wwindow.edge_drag(widget,x,y)

	local window,screen=widget:window_screen()

	if window.flags.nodrag then return end

	local master=widget.master
	local active_xy=master.active_xy

	local windows= (window.parent.class=="windows") and window.parent or nil

	if windows and windows.winmode~="drag" then -- we are docked
		return
	end

	window:move_to_top() -- move to top

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
	window.master.request_layout=true

end

function wwindow.drag(widget,x,y)

	local window,screen=widget:window_screen()
	local windows= (window.parent.class=="windows") and window.parent or nil

	if window.flags.nodrag then return end

	if windows and windows.winmode~="drag" then -- we are docked
		return
	end

--	window:move_to_top()

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
	window.master.request_layout=true

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
	local windows= (window.parent.class=="windows") and window.parent or nil

	local ss=widget.master.theme.grid_size
	local bar_height=widget.flags.nobar and 0 or ss

	if v then
		if window.panel_mode=="scale" then -- scale but maintain aspect of content

			v.sx=window.hx/v.hx
			v.sy=window.hy/v.hy

--				if v.sx~=v.sy then print(widget.hx,v.hx,"-",widget.hy,v.hy) end

			if v.sx<v.sy then v.sy=v.sx else v.sx=v.sy end

			v.px=(window.hx-v.hx*v.sx)/2
			v.py=(window.hy-v.hy*v.sy)/2

		elseif window.panel_mode=="stretch" then -- stretch to fit any area

			v.sx=window.hx/v.hx
			v.sy=window.hy/v.hy

		elseif window.panel_mode=="fill" then -- fill any area, no scale

			v.sx=1
			v.sy=1

		end
	end

-- also layout any other children
	widget.meta.layout(widget)

end

wwindow.window_hooks_reset=function(widget)
	widget:resize()
--	widget:layout()
--	widget:resize()
--	widget:layout()
--print("window reset",widget.id,widget.hx,widget.hy,"...",widget.win_fbo.hx,widget.win_fbo.hy)
	widget.hx=widget.reset_layout.hx
	widget.hy=widget.reset_layout.hy
	winclamp(widget)
	widget:layout()
	widget:build_m4()
end

wwindow.move_to_top=function(window)
	local window,screen=window:window_screen()
	if window.parent==screen.windows then -- must be in windows list
		if not window.flags.nosort then
			screen.windows:insert(window) -- move to top
		end
	end
end

wwindow.is_top=function(window)
	local window,screen=window:window_screen()
	return screen.windows[#screen.windows]==window
end

wwindow.window_hooks=function(_window,act,widget)
--print(act,widget and widget.id)

	local window,screen=(_window or widget):window_screen()
	local old_act="click"

	if ( act=="click" or act=="release" ) and window~=widget then -- turn a click into another act
		old_act=act
		act=widget.id
	end

if window then -- only if message is bound to a window

	if window.flags and window.flags.nodrag then return end

	if act=="active" then


--[[
		local windows= (window.parent.class=="windows") and window.parent or nil

--print(widget.id,widget.drag,wwindow.edge_drag)

		if widget.drag~=wwindow.edge_drag then -- do not undock when clicking on the drag widgets

			if windows and windows.windows~="drag" then -- we are docked so undock us
				window.active_nopush=true
				local master=screen.master
				screen:remove_split(window)
				wwindow.window_hooks_reset(window)
				master.active_xy={window.hx/2,window.hy/2,mx=0,my=0}
			end

		end
]]

		window:move_to_top()

--		print("ACTIVE",window.id)

	elseif act=="inactive" then

--[[
		if window.active_push and math.abs(window.active_push[2])>16 then

--			print("PUSH",window.id,window.active_push[1],window.active_push[2])

			if not window.active_nopush then
				if window.parent.windows=="drag" then -- only dock if we are a dragable window

					local axis,order=window.active_push[1],(window.active_push[2]>=0) and 1 or 2
					local split
					local v=widget
					while v~=v.parent do
						if v.screen_split and v.split_axis==axis and v.split_order==order then split=v break end
						v=v.parent
					end

					if split then
						local dock=(split[1].class=="windows" and split[1]~=screen.windows) and split[1] or split[2] -- pick the dock from the screen_split
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

	end
end

-- these acts can be used by outside code
-- eg
-- window.window_hooks("win_hide")

	if act=="win_hide" or act=="win_menu" then -- clicking the win_menu button will also close the window

		if act=="win_menu" and old_act=="release" then
			-- do not hide
		else
			window.hidden=true -- hide it
			window.master:call_descendents(function(w) w:set_dirty() end)
			window.master.request_layout=true
		end

	elseif act=="win_show" then

		window.hidden=false
		window:move_to_top()
		window.master:call_descendents(function(w) w:set_dirty() end)
		window.master.request_layout=true

	elseif act=="win_toggle" then

		if window.hidden then -- show it

			window.hidden=false
			window:move_to_top()
			window.master:call_descendents(function(w) w:set_dirty() end)
			window.master.request_layout=true

		else

			window.hidden=true -- hide it
			window.master:call_descendents(function(w) w:set_dirty() end)
			window.master.request_layout=true

		end

	elseif act=="win_toggle_other" then

		local win=widget.master.ids[widget.user]
		if win then
			win.window_hooks("win_toggle")
		end

	elseif act=="win_grow" then

		window.hx=window.hx*1.5
		window.hy=window.hy*1.5
		winclamp(window)
		window.master.request_layout=true

	elseif act=="win_shrink" then

		window.hx=window.hx/1.5
		window.hy=window.hy/1.5
		winclamp(window)
		window.master.request_layout=true

	elseif act=="win_reset" then

		wwindow.window_hooks_reset(window)

	end

end


function wwindow.setup(window,def)

--print(def.id,def.px,def.py,def.hx,def.hy)

	window.flags=def.flags or {}

	window.class="window"

	if type(def.solid=="nil") then window.solid=true end -- default to clickable and dragable

	window.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

--	window.key=wwindow.key
--	window.mouse=wwindow.mouse
	window.drag=wwindow.drag
	window.update=wwindow.update
	window.draw=wwindow.draw
	window.layout=wwindow.layout
	window.move_to_top=wwindow.move_to_top
	window.is_top=wwindow.is_top

	window.window_hooks = function(act,w) return wwindow.window_hooks(window,act,w) end
	window.hooks=window.window_hooks

	window.window_menu=function()
		local window,screen=window:window_screen()
		return screen:window_menu()
	end
	window.menu_data=window.menu_data or {
--		{	id="win_actions",	text="Actions...",menu_data={hooks=window.window_hooks,
			{	id="win_hide",		text="Hide Window",		},
			{	id="win_reset",		text="Reset Window Size",	},
			{	id="win_shrink",	text="Shrink Window Size",	},
			{	id="win_grow",		text="Grow Window Size",		},
--		}},
		{	id="win_windows",	text="Windows...",menu_data=window.window_menu},
		hooks=window.window_hooks,
	}

	local ss=window.master.theme.grid_size
	local color=0

	local ss_side=ss/8
	local ss_corner=ss/4

	local bar_height=window.flags.nobar and 0 or ss

--	window.outline_size=-ss/16
--	window.outline_color=0xcc000000

	window.win_scale=def.win_scale or 0

-- add all the trimmings
	window.win_fbo=window:add({
		hook_resize=function(it)
			if window.win_canvas.size=="fit" then -- does the window fit the canvas?

				local bar_height=(window.flags.nobar and 0 or window.master.theme.grid_size)
				it.hx=window.win_canvas.hx
				it.hy=window.win_canvas.hy+bar_height
			else -- or does the canvas fit the window
				local bar_height=(window.flags.nobar and 0 or window.master.theme.grid_size)
				it.hx=it.parent.hx
				it.hy=it.parent.hy

			end
		end,
		hook_layout=function(it)
			it.px=0
			it.py=0
		end,
--				hx=def.hx,
--				hy=def.hy+bar_height,
--				px=0,
--				py=0,
--				class="fill",
		sx=1,
		sy=1,
		color=color,
		fbo=true,
		size="none",
		style="flat",
		highlight="none",
		smode="topleft",
		outline_size=ss/8,
		outline_color=0x44000000,
		outline_fade_color=0x00000000,
	})

	window.win_canvas=window.win_fbo:add({
		hook_resize=function(it)
			if window.win_canvas.size=="fit" then -- does the window fit the canvas?

			else -- or does the canvas fit the window

				local bar_height=(window.flags.nobar and 0 or window.master.theme.grid_size)
				it.hx=it.parent.hx
				it.hy=it.parent.hy-bar_height

			end
		end,
		hook_layout=function(it)
			local bar_height=(window.flags.nobar and 0 or window.master.theme.grid_size)
			it.px=0
			it.py=bar_height
		end,
		class="fill",
--				px=0,
--				py=bar_height,
		hx=def.hx,
		hy=def.hy,
		size= (def.panel_mode~="fill") and "fit",
		color=color,
		highlight="none",
	})


	-- use this to recreate the magic top left windows icon/menu anywhere you want
	window.win_menu_def={
			class="menudrop",
			drop="active",
			topmenu=true,
			px=0,
			py=0,
			hx=ss,
			hy=ss,
			text="~",
			color=color,
			solid=true,
			menu_data=window.menu_data,
			cursor="hand",
			id="win_menu",
			hooks=window.window_hooks,
		}

	if bar_height>0 then -- add a bar

		window.win_three=window.win_fbo:add({
			hook_resize=function(it)
				it.hx=it.parent.hx
			end,
			px=0,
			py=0,
			hx=def.hx,
			hy=ss,
			class="three",
		})

		window.win_menu=window.win_three:add(window.win_menu_def)

		window.win_title=window.win_three:add({
			px=0,
			py=0,
			hx=def.hx,
			hy=ss,
			text=def.title or "...",
			draw_text=function(widget,opts) -- draw the text propperly centered in the entire window
				if opts.size then
					return font.width(widget.text) , widget.grid_size or font.size*1.5
				end
				local tx=(widget.hx+ss-font.width(widget.text))/2-ss
				local ty=widget.hy/2
				if tx<0 then tx=0 end
				oven.gl.Color( unpack(widget.master.get_color(nil,widget.text_color)) )
				font.set_xy(tx+(opts.txp or 0),ty-(opts.fx or 0)+(opts.typ or 0))
				font.draw(widget.text)
			end,
		})

	end

	window.win_edge_l=window.win_fbo:add({
		hook_resize=function(it)
			it.hx=(ss/4)
			it.hy=it.parent.hy
		end,
		hook_layout=function(it)
			it.px=-(ss/8)
			it.py=0
		end,
--				px=-ss/8,
--				py=0,
--				hx=ss/4,
--				hy=def.hy+bar_height,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_l",
		cursor="sizewe",
		drag=wwindow.edge_drag,
	})
	window.win_edge_r=window.win_fbo:add({
		hook_resize=function(it)
			it.hx=(ss/4)
			it.hy=it.parent.hy
		end,
		hook_layout=function(it)
			it.px=it.parent.hx-(ss/8)
			it.py=0
		end,
--				px=def.hx-ss/8,
--				py=0,
--				hx=ss/4,
--				hy=def.hy+bar_height,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_r",
		cursor="sizewe",
		drag=wwindow.edge_drag,
	})
	window.win_edge_t=window.win_fbo:add({
		hook_resize=function(it)
			it.hx=it.parent.hx
			it.hy=(ss/4)
		end,
		hook_layout=function(it)
			it.px=0
			it.py=-(ss/8)
		end,
--				px=0,
--				py=-ss/8,
--				hx=def.hx,
--				hy=ss/4,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_t",
		cursor="sizens",
		drag=wwindow.edge_drag,
	})
	window.win_edge_b=window.win_fbo:add({
		hook_resize=function(it)
			it.hx=it.parent.hx
			it.hy=(ss/4)
		end,
		hook_layout=function(it)
			it.px=0
			it.py=it.parent.hy-(ss/8)
		end,
--				px=0,
--				py=def.hy+bar_height-ss/8,
--				hx=def.hx,
--				hy=ss/4,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_b",
		cursor="sizens",
		drag=wwindow.edge_drag,
	})

	window.win_edge_tl=window.win_fbo:add({
		hook_layout=function(it)
			it.px=0-(ss/4)
			it.py=0-(ss/4)
		end,
--				px=-ss/4,
--				py=-ss/4,
		hx=ss/2,
		hy=ss/2,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_tl",
		cursor="sizenwse",
		drag=wwindow.edge_drag,
	})
	window.win_edge_tr=window.win_fbo:add({
		hook_layout=function(it)
			it.px=it.parent.hx-(ss/4)
			it.py=0-(ss/4)
		end,
--				px=def.hx-ss/4,
--				py=-ss/4,
		hx=ss/2,
		hy=ss/2,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_tr",
		cursor="sizenesw",
		drag=wwindow.edge_drag,
	})
	window.win_edge_bl=window.win_fbo:add({
		hook_layout=function(it)
			it.px=0-(ss/4)
			it.py=it.parent.hy-(ss/4)
		end,
--				px=-ss/4,
--				py=def.hy+bar_height-ss/4,
		hx=ss/2,
		hy=ss/2,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_bl",
		cursor="sizenwse",
		drag=wwindow.edge_drag,
	})
	window.win_edge_br=window.win_fbo:add({
		hook_layout=function(it)
			it.px=it.parent.hx-(ss/4)
			it.py=it.parent.hy-(ss/4)
		end,
--				px=def.hx-ss/4,
--				py=def.hy+bar_height-ss/4,
		hx=ss/2,
		hy=ss/2,
		solid=true,
		hooks=window.window_hooks,
		id="win_edge_br",
		cursor="sizenesw",
		drag=wwindow.edge_drag,
	})


	window.hx=def.hx
	window.hy=def.hy+bar_height

-- you should put your widgets here
	window.children=window.win_canvas

	window.reset_layout={}
	for _,n in ipairs{"hidden","px","py","hx","hy"} do
		window.reset_layout[n]=window[n]
	end
	window.reset_layout_resize=function()
		window:resize()
		window:layout()
		window:resize()
--		window.reset_layout.hx=window.win_fbo.hx
--		window.reset_layout.hy=window.win_fbo.hy
	end

--local widget=window
--print("window start",widget.id,widget.hx,widget.hy,"...",widget.win_fbo.hx,widget.win_fbo.hy)

	return window
end

return wwindow
end
