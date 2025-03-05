--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generic default widget functions

local tardis=require("wetgenes.tardis")

local wstr=require("wetgenes.string")
local ls=function(...) print(wstr.dump(...)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmeta)
wmeta=wmeta or {}

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

-- available widget classes
wmeta.classes={

-- base classes

	["master"]=oven.rebake("wetgenes.gamecake.widgets.master"),
	["fill"]=oven.rebake("wetgenes.gamecake.widgets.fill"),
	["center"]=oven.rebake("wetgenes.gamecake.widgets.center"),
	["paragraph"]=oven.rebake("wetgenes.gamecake.widgets.paragraph"),
	["split"]=oven.rebake("wetgenes.gamecake.widgets.split"),
	["three"]=oven.rebake("wetgenes.gamecake.widgets.three"),
	["quad"]=oven.rebake("wetgenes.gamecake.widgets.quad"),
	["panel"]=oven.rebake("wetgenes.gamecake.widgets.panel"),
	["screen"]=oven.rebake("wetgenes.gamecake.widgets.screen"),
	["window"]=oven.rebake("wetgenes.gamecake.widgets.window"),
	["windows"]=oven.rebake("wetgenes.gamecake.widgets.windows"),
	["dialogs"]=oven.rebake("wetgenes.gamecake.widgets.dialogs"),
	["button"]=oven.rebake("wetgenes.gamecake.widgets.button"),
	["checkbox"]=oven.rebake("wetgenes.gamecake.widgets.checkbox"),
	["drag"]=oven.rebake("wetgenes.gamecake.widgets.drag"),
	["text"]=oven.rebake("wetgenes.gamecake.widgets.text"),
	["tiles"]=oven.rebake("wetgenes.gamecake.widgets.tiles"),
	["textedit"]=oven.rebake("wetgenes.gamecake.widgets.textedit"),
	["menu"]=oven.rebake("wetgenes.gamecake.widgets.menu"),
	["menubar"]=oven.rebake("wetgenes.gamecake.widgets.menubar"),
	["menuitem"]=oven.rebake("wetgenes.gamecake.widgets.menuitem"),
	["pages"]=oven.rebake("wetgenes.gamecake.widgets.pages"),
	["tabs"]=oven.rebake("wetgenes.gamecake.widgets.tabs"),

--classes built out of the base classes

	["tabpages"]=oven.rebake("wetgenes.gamecake.widgets.tabpages"),

	["split_drag"]=oven.rebake("wetgenes.gamecake.widgets.split_drag"),

	["pan"]=oven.rebake("wetgenes.gamecake.widgets.pan"),
	["slide"]=oven.rebake("wetgenes.gamecake.widgets.slide"),

	["scroll"]=oven.rebake("wetgenes.gamecake.widgets.scroll"),


	["tree"]=oven.rebake("wetgenes.gamecake.widgets.tree"),
	["treefile"]=oven.rebake("wetgenes.gamecake.widgets.treefile"),

	["menudrop"]=oven.rebake("wetgenes.gamecake.widgets.menudrop"),

	["file"]=oven.rebake("wetgenes.gamecake.widgets.file"),
	["texteditor"]=oven.rebake("wetgenes.gamecake.widgets.texteditor"),
}

--
-- add meta functions
--
function wmeta.setup(def)

--	local master=def.master
	local meta=def.meta
	local win=def.win

-- set a dirty flag on this and all parents, this has a smart break, as if a child is dirty
-- then its parent must also be
-- the dirty flag is cleared on draw
	function meta.set_dirty(widget)
		widget.dirty=true
		while (widget.parent ~= widget) and widget.parent and not widget.parent.dirty do
			widget=widget.parent
			widget.dirty=true
		end
	end

-- call hook but do it later so we avoid race conditions
	function meta.call_hook_later(widget,hook,dat)
		widget.master.later_append(meta.call_hook,widget,hook,dat)
	end

	function meta.call_hook(widget,hook,dat)

--[[
		if hook=="click" and widget.id then
			local action=widget.master.actions[widget.id]
			if action and widget.user then
				action=action[widget.user]
			end
			if action then -- add an action message
				oven.win:push_msg({
					class="action",
					action=1,
					time=os.time(),
					id=action.id,
					user=action.user,
				})
			end
		end
]]
		if widget.class_hooks then
			for _,ch in ipairs(widget.class_hooks) do
				if ch(hook,widget,dat) then return end -- and it can eat the event if it returns true
			end
		end
		local hooks=widget.hooks or widget.master.hooks -- can use master hooks
		local type_hooks=type(hooks)
		if type_hooks=="function" then -- master function
			hooks(hook,widget,dat)
		elseif type_hooks=="table" and hooks[hook] then -- or table of functions
			hooks[hook](hook,widget,dat)
		end
	end

	meta.add_class_hook=function(widget,fn)
		widget.class_hooks=widget.class_hooks or {}
		widget.class_hooks[ #widget.class_hooks+1 ]=fn
	end
	meta.del_class_hook=function(widget,fn)
		local hooks=widget.class_hooks or {}
		for i=#hooks,1,-1 do
			if hooks[i]==fn then
				table.remove(hooks,i)
			end
		end
	end

--
-- add a new widget as a child to this one
--
	function meta.add(parent,...) -- could supply multiple, but probably just one at a time
		if parent.children then parent=parent.children end -- always put children here
		local ret
		for i,def in pairs{...} do
			local widget={}
			setmetatable(widget,meta)
			table.insert(parent,widget)
			widget.parent=parent
			widget.master=parent.master
			widget:setup(def)
			widget.meta=meta
			ret=ret or widget
			for i,v in ipairs(def) do -- sub add
				(ret.children or ret):add(v)
			end
		end
		return ret -- return the first widget added
	end

	function meta.add_border(parent,c)
		local n=parent:add({hx=c.hx,hy=c.hy}) -- full size
		c.hx=c.hx-((c.px or 0)*2)
		c.hy=c.hy-((c.py or 0)*2)
		return n:add(c) -- smaller and centered
	end

	function meta.add_indent(parent,c,t)
		t=t or 1
		local n=parent:add({px=c.px,py=c.py,hx=c.hx,hy=c.hy}) -- full size
		c.px=t*1
		c.py=t*1
		c.hx=c.hx-t*2
		c.hy=c.hy-t*2
		return n:add(c) -- smaller
	end

--
-- remove from parent
--
	function meta.remove(widget)

		if widget.parent then
			for i,v in ipairs(widget.parent) do
				if v==widget then
					table.remove(widget.parent,i)
				end
			end
			widget.parent=nil
		end

	end
--
-- add a previosuly created widget as a child to this widget
-- the widget will be forcibly removed...
--
	function meta.insert(parent,widget,top)

		meta.remove(widget) -- make sure we dont end up in two parents

		if top then
			table.insert(parent,top,widget)
		else
			table.insert(parent,widget)
		end

		widget.parent=parent
		widget.master=parent.master

		return widget
	end

	function meta.parent_index(widget)
		for i,v in ipairs(widget.parent) do if v==widget then return i end end
	end

--
-- initial setup
--def
	function meta.setup(widget,def)

		for a,b in pairs(def) do if type(a)=="string" then widget[a]=b end end -- shallow copy every string value

		for _,n in ipairs({"data","datx","daty"}) do
			if type(widget[n])=="string" then
				local it=n
				while widget[it] do it=widget[it] end -- allow lookup
				if type(it)=="string" then
					if widget.master.datas then -- auto lookup data by name
						widget[n]=widget.master.datas.get(it)
					end
				else
					widget[n]=it
				end
			end
		end

		if type(widget.class)=="function" then widget.class(widget,def) end -- allow callback to fill in more values

		widget.state=widget.state or "none"

		widget.meta=meta

		widget.smode=widget.smode or "center"
		widget.sx=widget.sx or 1 -- display scale (of children)
		widget.sy=widget.sy or 1
		widget.pa=widget.pa or 0 -- display rotation angle (of children)

		widget.px=widget.px or 0 -- relative to parent, pixel position
		widget.py=widget.py or 0

		widget.hx=widget.hx or 0 -- absolute pixel size of widget (in parents space)
		widget.hy=widget.hy or 0
		widget.hz=widget.hz or 0 -- used to signal an fbo with a depth buffer

		widget.font=widget.font or widget.parent.font --  use this font if set or inherit value from parent

		widget.text_color=widget.text_color or widget.parent.text_color -- black text

		if widget.class and wmeta.classes[widget.class] then -- got a class, call its setup, its setup can override other functions
			wmeta.classes[widget.class].setup(widget,def)
		end

		if widget.master.ids and widget.id then widget.master.ids[widget.id]=widget end -- lookup by id

		if def.fbo then -- an fbo buffer has been requested (can speed rendering up)
			widget.fbo=framebuffers.create(0,0,0)
		end

		widget:set_dirty()

		return widget
	end
--
-- and final cleanup
--
	function meta.clean(widget)
		widget:set_dirty()
		if widget.master.ids and widget.id then widget.master.ids[widget.id]=nil end -- remove id lookup
		if widget.fbo then widget.fbo:clean() end
		if widget.master.focus==widget then widget.master.set_focus(nil) end
		if widget.master.edit ==widget then widget.master.set_focus_edit(nil) end
		return widget
	end


-- get a member from this or parents... widget
	function meta.bubble(widget,name)
		local w=widget
		repeat
			if w[name] then return w[name] end
			if w==w.parent then break end -- loop sanity
			w=w.parent
		until not w
	end

--
-- live adjustment
--
	function meta.get(widget,val,...)

		if val=="slide" then

			local x=(widget.px) / (widget.parent.hx-widget.hx)
			local y=(widget.py) / (widget.parent.hy-widget.hy)


			return x,y
		end

	end

	function meta.set(widget,val,...)
	local t={...}

		if val=="slide" then
			for i,v in ipairs(widget) do

				local pxf=0
				local pyf=0
				if type(t[1])=="table" then
					pxf=t[1][1] or v.pxf or 0
					pyf=t[1][2] or v.pyf or 0
				else
					pxf=t[1] or v.pxf or 0
					pyf=t[2] or v.pyf or 0
				end


--print("SET",v.pxf,v.pyf)

				v.px=(widget.hx-v.hx)*pxf -- local position relative to parents size
				v.py=(widget.hy-v.hy)*pyf

			end
		end
	end

--
-- handle key input
--
	function meta.key(widget,ascii,key,act)
	end

--
-- handle mouse input
--

-- get local x,y from master coordinates
	function meta.get_local_xy(w,_x,_y)
		if not w.m4 then return -1,-1 end
		local v4=tardis.v4.new(_x,_y,0,1)
		v4:product(w.m4)
		return v4[1],v4[2]
	end
	meta.mousexy=meta.get_local_xy

-- get master xy from local coordinates
	function meta.get_master_xy(w,x,y)
		if not w.m4 then return -1,-1 end
		local m=tardis.m4.new(w.m4):inverse()
--		w.m4:inverse(m)
		local v=tardis.v4.new(x or 0,y or 0,0,1)
		v:product(m)
		return v[1],v[2]
	end

	function meta.mouse(widget,act,_x,_y,keyname)
		if widget.hidden then return end
		local old_over=widget.master.over
		for i=#widget,1,-1 do -- children must be within parent bounds to catch clicks
			local v=widget[i]
			if not v.hidden then
				local x,y=v:mousexy(_x,_y)
				local tx=(x-(v.pan_px or 0))
				local ty=(y-(v.pan_py or 0))
				if tx>=0 and tx<v.hx and ty>=0 and ty<v.hy then
--print(math.floor(tx),math.floor(ty),math.floor(v.hx),math.floor(v.hy),v.class,v.id,v.user,i)
					if v.solid then
						widget.master.over=v
					end
					v:mouse(act,_x,_y,keyname)
					if widget.master.over~=old_over then break end -- we are over something new
				end
			end
		end
	end

	function meta.parent_active(widget)
		if not widget.master.active then return true end

		local w=widget
		repeat
			if w==widget.master.active then return true end
			w=w.parent
			if w==widget.master.active then return true end
		until w.parent==w -- reached top

		return false
	end
--
-- update this widget and its sub widgets
--
	function meta.update(widget)

		if widget.anim then
			widget.anim:update()
		end

		for i,v in ipairs(widget) do
			if not v.hidden then v:update() end -- hidden widgets are ignored
		end
	end

--
-- remove all children of this widget
--
	function meta.remove_all(widget)

		local len=#widget
		for i=1,len do
			widget[i]=nil
		end
	end

--
-- clean and remove all children of this widget
--
	function meta.clean_all(widget,start)
		start=start or 1
		local len=#widget
		for i=len,start,-1 do
			if widget[i] then
				widget[i]:clean_all()
				widget[i]:clean()
				widget[i]=nil
			end
		end
	end

--
-- Call this function for all descendents, recursivly
--
	function meta.call_descendents(widget,func)
		for i,v in ipairs(widget) do
			meta.call_descendents(v,func)
			func(v)
		end
	end

	meta.isover=function(widget,mode)
		local o=widget.master.over
		if o then
			while o~=o.parent do -- need to check all parents
				if o==widget then return true end
				if widget.also_over then -- these widgets also count as over
					for i,v in pairs(widget.also_over) do
						if o==v then return true end -- check if any parent is in the also over group
					end
				end
				o=o.parent
			end
		end
		return false
	end


-- search upwards and return window,screen (so this could be a widget in the window)
-- screen should always be the widgets parent
	meta.window_screen=function(it)
		local window=it.window
		local screen=it.screen or ( window and window.screen )
		while it.parent ~= it and it.parent do -- go searching
			window=it.window or window
			screen=it.screen or screen
			if it.class=="window" then window=it end -- found window
			it=it.parent -- search upwards
			if it.class=="screen" then screen=it end -- check for screen which should always be the parent of a window
			if window and screen then return window,screen end -- return the first valid data
		end
		return window,screen
	end


	function meta.build_m4(widget)

		widget.m4=widget.m4 or tardis.m4.new()

		if widget.parent==widget then
			widget.m4:identity()
		else
--print("parent",widget.parent.m4)
			widget.m4:set(widget.parent.m4)
		end

		local m4=tardis.m4.new()
		m4:identity()

--		local m4=widget.m4


--print("SS",widget.sx,widget.sy)
		if widget.sx==1 and widget.sy==1 then
				m4:translate( tardis.v3.new(-widget.px,-widget.py,0,0) )
--				x= x-widget.px
--				y= y-widget.py
--print("noscale",sx,sy)
		else
			if widget.smode=="center" then
--print("CCCscale",widget.sx,widget.sy)
				m4:translate( tardis.v3.new(widget.hx*0.5,widget.hy*0.5,0) )
				m4:scale_v3(  tardis.v3.new(1/widget.sx,1/widget.sy,1) )
				m4:translate( tardis.v3.new(-widget.px-widget.hx*0.5,-widget.py-widget.hy*0.5,0) )
--				x= ((x-widget.px-widget.hx*0.5)/widget.parent.sx)+widget.hx*0.5
--				y= ((y-widget.py-widget.hy*0.5)/widget.parent.sy)+widget.hy*0.5
			else
--print("scale",widget.sx,widget.sy)
				m4:scale_v3(  tardis.v3.new(1/widget.sx,1/widget.sy,1) )
				m4:translate( tardis.v3.new(-widget.px,-widget.py,0) )
--				x=((x-widget.px)/widget.sx)
--				y=((y-widget.py)/widget.sy)
			end
		end

		if widget.pan_px and widget.pan_py then -- fidle everything
--print("build",widget.pan_px,widget.pan_py)
			m4:translate({widget.pan_px,widget.pan_py,0})
		end

		m4:product(widget.m4,widget.m4)


		for i,v in ipairs(widget) do
			meta.build_m4(v)
		end

	end

-- resize is performed recursively before layout so that layout can position its children
	function meta.resize(widget,mini)
		mini=mini or 0
		for i,v in ipairs(widget) do if i>mini and v.size then


			for token in string.gmatch(v.size, "[^%s]+") do -- can contain multiple tokens

				if token=="full" then -- force full size

					v.px=0
					v.py=0
					v.hx=widget.hx/widget.sx
					v.hy=widget.hy/widget.sy

	--print("full",v.px,v.py,v.hx,v.hy)
	--print("parent class",widget.parent.class)

				elseif token=="fullx" then -- force full size X only

					v.px=0
					v.hx=widget.hx/widget.sx

				elseif token=="fully" then -- force full size Y only

					v.py=0
					v.hy=widget.hy/widget.sy

				elseif token=="border" then -- force a fixed border size

					v.hx=(widget.hx/widget.sx)-(v.px*2)
					v.hy=(widget.hy/widget.sy)-(v.py*2)

				elseif token=="minmax" then -- force a minimum width maximum height with scale on parent

					if     v.hx_min and v.hy_max then
--						v.px=0
--						v.py=0
						v.hx=widget.hx
						v.hy=v.hy_max
						if v.hx < v.hx_min then
							v.sx=widget.hx/v.hx_min
							v.sy=v.sx
							v.hx=v.hx_min
							v.hy=v.hy_max--*v.sy
						else
							v.sx=1
							v.sy=1
						end
					elseif v.hy_min and v.hx_max then
						v.px=0
						v.py=0
						v.hx=v.hx_max
						v.hy=widget.hy
						if v.hy < v.hy_min then
							v.sx=widget.hy/v.hy_min
							v.sy=v.sx
							v.hy=v.hy_min
							v.hx=v.hx_max--*v.sx
						else
							v.sx=1
							v.sy=1
						end
					end

				end
			end

		end end
		if widget.hook_resize then -- let the widget do some magic before we recurse
			widget.hook_resize(widget)
		end
		for i,v in ipairs(widget) do
--			if not v.hidden then v:resize() end
			v:resize()
		end
		for i,v in ipairs(widget) do if i>mini and v.size then
			for token in string.gmatch(v.size, "[^%s]+") do -- can contain multiple tokens

				if token=="fitx" then  -- set hx to maximum of children

					v.hx=0
					for i,w in ipairs(v) do
						if not w.hidden then
							local x=(w.hx+w.px)*w.sx
							if x>v.hx then v.hx=x end
						end
					end
					v.hx=math.floor(v.hx)

				elseif token=="fity" then -- set hy to maximum of children

					v.hy=0
					for i,w in ipairs(v) do
						if not w.hidden then
--print(w.sy)
							local y=(w.hy+w.py)*w.sy
							if y>v.hy then v.hy=y end
						end
					end
					v.hy=math.floor(v.hy)
				end

			end
		end end


	end

	function meta.layout(widget,mini)
--		mini=mini or 0
--		for i,v in ipairs(widget) do if i>mini then
--		end end

		if widget.hook_layout then -- let the widget do some magic before we recurse
			widget.hook_layout(widget)
		end
		for i,v in ipairs(widget) do
--			if not v.hidden then v:layout() end
			v:layout()
		end
	end

	function meta.resize_and_layout(widget)
--print("master layout")
		widget:resize()
		widget:layout()
		widget:build_m4()
--exit()
	end

end


return wmeta
end
