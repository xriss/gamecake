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

function wwindow.edge_drag(widget,x,y)

	local parent=widget.parent
	local master=widget.master
	local window=parent.parent
	local active_xy=master.active_xy
	
	master:insert(window) -- move to top

	if not active_xy.edge then -- fill in starting edge on the first call

		active_xy.edge=widget.id
		
		active_xy.wx,active_xy.wy=window.parent:mousexy(active_xy.mx,active_xy.my)

		active_xy.px=window.px
		active_xy.py=window.py
		active_xy.hx=window.hx
		active_xy.hy=window.hy

	end

	local mx,my=window.parent:mousexy(x,y)

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
	
	if window.hx > window.parent.hx then window.hx=window.parent.hx end
	if window.hy > window.parent.hy then window.hy=window.parent.hy end
	
	if window.panel_mode=="scale" then -- keep aspect when scaling

		local sx=window.hx/window.win_fbo.hx
		local sy=window.hy/window.win_fbo.hy

		if window.win_fbo.hx*sx > window.parent.hx then sx=window.parent.hx/window.win_fbo.hx end
		if window.win_fbo.hy*sx > window.parent.hy then sx=window.parent.hy/window.win_fbo.hy end
		if window.win_fbo.hx*sy > window.parent.hx then sy=window.parent.hx/window.win_fbo.hx end
		if window.win_fbo.hy*sy > window.parent.hy then sy=window.parent.hy/window.win_fbo.hy end

		local s=sx<sy and sx or sy
		if	master.active_xy.edge=="win_edge_t" or master.active_xy.edge=="win_edge_b" then s=sy end
		if	master.active_xy.edge=="win_edge_l" or master.active_xy.edge=="win_edge_r" then s=sx end

		
		window.hx=window.win_fbo.hx*s
		window.hy=window.win_fbo.hy*s

	end

	window:set_dirty()	
	window:layout()
	window:build_m4()

end

function wwindow.drag(widget,x,y)

	local parent=widget.parent
	local master=widget.master

	parent:insert(widget) -- move to top

	local rx,ry=parent:mousexy(x,y)
	local x,y=rx-master.active_xy[1],ry-master.active_xy[2]

--	local maxx=parent.hx-widget.hx
--	local maxy=parent.hy-widget.hy

	widget.px=x
	widget.py=y
	
--[[
	if widget.px<0    then widget.px=0 end
	if widget.px>maxx then widget.px=maxx end
	if widget.py<0    then widget.py=0 end
	if widget.py>maxy then widget.py=maxy end
	
	if parent.snap then
		parent:snap(true)
	end
]]

	widget:call_hook_later("slide")
	
	widget:set_dirty()
	
	widget:layout()
	widget:build_m4()
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

-- also layout any other children
	widget.meta.layout(widget)

	local ss=(widget.master.grid_size or 24)
	local hy=widget.win_canvas.hy+ss
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

wwindow.win_hooks=function(widget,act,w)
--print(act,w.id)

	local winclamp=function(window)
--		if window.hx > window.parent.hx then window.hx = window.parent.hx end
--		if window.hy > window.parent.hy then window.hy = window.parent.hy end
		if window.panel_mode=="scale" then -- maintain aspect
			local sx=window.hx/window.win_fbo.hx
			local sy=window.hy/window.win_fbo.hy
			local s=sx<sy and sx or sy
			window.hx=window.win_fbo.hx*s
			window.hy=window.win_fbo.hy*s
		end
		if window.px<0 then window.px=0 end
		if window.py<0 then window.py=0 end
		if window.px+window.hx>window.parent.hx then window.px=window.parent.hx-window.hx end
		if window.py+window.hy>window.parent.hy then window.py=window.parent.hy-window.hy end
	end

	if act=="click" then
		if w.id=="win_hide" then
		
			widget.hidden=true
			
		elseif w.id=="win_grow" then
			widget.hx=widget.hx*1.5
			widget.hy=widget.hy*1.5
			winclamp(widget)
			widget:layout()
			widget:build_m4()

		elseif w.id=="win_shrink" then
			widget.hx=widget.hx/1.5
			widget.hy=widget.hy/1.5
			winclamp(widget)
			widget:layout()
			widget:build_m4()

		elseif w.id=="win_reset" then

			widget.hx=widget.win_fbo.hx
			widget.hy=widget.win_fbo.hy
			winclamp(widget)
			widget:layout()
			widget:build_m4()

		end
	end
end

function wwindow.setup(widget,def)

	widget.class="window"

	widget.panel_mode=def.panel_mode or "scale" 	-- scale the child to fit

--	widget.key=wwindow.key
--	widget.mouse=wwindow.mouse
	widget.drag=wwindow.drag
	widget.update=wwindow.update
	widget.draw=wwindow.draw
	widget.layout=wwindow.layout
	
	widget.win_hooks = function(act,w) return wwindow.win_hooks(widget,act,w) end

	widget.menu_data=widget.menu_data or {
		{	id="win_hide",		text="Hide Window",		},
		{	id="win_reset",		text="Reset Window",	},
		{	id="win_shrink",	text="Shrink Window",	},
		{	id="win_grow",		text="Grow Window",		},
		hooks=widget.win_hooks,
	}
	
	local ss=widget.master.grid_size or 24
	local color=0

	local ss1=ss/24
	local ss_side=ss/8
	local ss_corner=ss/4


	widget.outline_size=ss_side
--	widget.outline_color=0xcc000000

	widget.win_scale=def.win_scale or 0


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
				highlight="none",
				smode="topleft",
			})

	widget.win_canvas=widget.win_fbo:add({
				class="fill",
				px=0,
				py=ss,
				hx=def.hx,
				hy=def.hy,
				size="fit",
				color=color,
				highlight="none",
			})

	widget.win_menu=widget.win_fbo:add_indent({
				class="menuitem",
				px=0,
				py=0,
				hx=ss,
				hy=ss,
				text=".",
				color=color,
				solid=true,
				menu_data=widget.menu_data,
				cursor="hand",
			},ss1)

	widget.win_title=widget.win_fbo:add_indent({
				px=ss,
				py=0,
				hx=def.hx-ss*3,
				hy=ss,
				text=def.title or "...",
--				color=color,
			},ss1)

	widget.win_shrink=widget.win_fbo:add_indent({
				px=def.hx-ss*2,
				py=0,
				hx=ss,
				hy=ss,
				text="-",
				color=color,
				solid=true,
				hooks=widget.win_hooks,
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
				hooks=widget.win_hooks,
				id="win_grow",
				cursor="hand",
			},ss1)

	widget.win_edge_l=widget.win_fbo:add({
				px=0,
				py=0,
				hx=ss/8,
				hy=def.hy+ss,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_l",
				cursor="sizewe",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_r=widget.win_fbo:add({
				px=def.hx-ss/8,
				py=0,
				hx=ss/8,
				hy=def.hy+ss,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_r",
				cursor="sizewe",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_t=widget.win_fbo:add({
				px=0,
				py=0,
				hx=def.hx,
				hy=ss/8,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_t",
				cursor="sizens",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_b=widget.win_fbo:add({
				px=0,
				py=def.hy+ss-ss/8,
				hx=def.hx,
				hy=ss/8,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_b",
				cursor="sizens",
				drag=wwindow.edge_drag,
			})

	widget.win_edge_tl=widget.win_fbo:add({
				px=0,
				py=0,
				hx=ss/4,
				hy=ss/4,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_tl",
				cursor="sizenwse",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_tr=widget.win_fbo:add({
				px=def.hx-ss/4,
				py=0,
				hx=ss/4,
				hy=ss/4,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_tr",
				cursor="sizenesw",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_bl=widget.win_fbo:add({
				px=0,
				py=def.hy+ss-ss/4,
				hx=ss/4,
				hy=ss/4,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_bl",
				cursor="sizenwse",
				drag=wwindow.edge_drag,
			})
	widget.win_edge_br=widget.win_fbo:add({
				px=def.hx-ss/4,
				py=def.hy+ss-ss/4,
				hx=ss/4,
				hy=ss/4,
				solid=true,
				hooks=widget.win_hooks,
				id="win_edge_br",
				cursor="sizenesw",
				drag=wwindow.edge_drag,
			})


	widget.hx=def.hx
	widget.hy=def.hy+ss
	
	return widget
end

return wwindow
end
