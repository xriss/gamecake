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
	["split"]=oven.rebake("wetgenes.gamecake.widgets.split"),
	["three"]=oven.rebake("wetgenes.gamecake.widgets.three"),
	["panel"]=oven.rebake("wetgenes.gamecake.widgets.panel"),
	["screen"]=oven.rebake("wetgenes.gamecake.widgets.screen"),
	["window"]=oven.rebake("wetgenes.gamecake.widgets.window"),
	["windock"]=oven.rebake("wetgenes.gamecake.widgets.windock"),
	["button"]=oven.rebake("wetgenes.gamecake.widgets.button"),
	["drag"]=oven.rebake("wetgenes.gamecake.widgets.drag"),
	["text"]=oven.rebake("wetgenes.gamecake.widgets.text"),
	["textedit"]=oven.rebake("wetgenes.gamecake.widgets.textedit"),
	["menu"]=oven.rebake("wetgenes.gamecake.widgets.menu"),
	["menubar"]=oven.rebake("wetgenes.gamecake.widgets.menubar"),
	["menuitem"]=oven.rebake("wetgenes.gamecake.widgets.menuitem"),
	["pages"]=oven.rebake("wetgenes.gamecake.widgets.pages"),

--classes built out of the base classes

	["pan"]=oven.rebake("wetgenes.gamecake.widgets.pan"),
	["slide"]=oven.rebake("wetgenes.gamecake.widgets.slide"),

	["scroll"]=oven.rebake("wetgenes.gamecake.widgets.scroll"),

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
		if type(widget.class_hooks)=="function" then -- the widget class wants to see this hook
			if widget.class_hooks(hook,widget,dat) then return end -- and it can eat the event if it returns true
		end
		local hooks=widget.hooks or widget.master.hooks -- can use master hooks
		local type_hooks=type(hooks)
		if type_hooks=="function" then -- master function
			hooks(hook,widget,dat)
		elseif type_hooks=="table" and hooks[hook] then -- or table of functions
			hooks[hook](hook,widget,dat)
		end
	end

--
-- add a new widget as a child to this one
--
	function meta.add(parent,...)		
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
	
		for a,b in pairs(def) do widget[a]=b end -- auto copy everything, then change below
	
		widget.state=widget.state or "none"
		
		widget.meta=meta
		
--		widget.draw=def.draw -- custom render, probably best to wrap with a widget:draw_base(function)
		
--		widget.data=def.data -- this widget is synced with this data
		
--		widget.class=def.class
--		widget.highlight=def.highlight
		
--		widget.id=def.id
--		widget.user=def.user -- any user data you wish to associate with this widget (we will ignore it)
--		widget.hooks=def.hooks

--		widget.clip=def.clip -- use viewport clipping to prevent drawing outside of the area

		widget.smode=widget.smode or "center"
		widget.sx=widget.sx or 1 -- display scale (of children)
		widget.sy=widget.sy or 1
		widget.pa=widget.pa or 0 -- display rotation angle (of children)
		
--		widget.size=def.size 	-- special layout action flag
								--	"full" 	==	Expand to fullsize of widget
								-- "minmax" fit using hx_min hy_max or hy_min hx_max
--		widget.hx_min=def.hx_min
--		widget.hy_min=def.hy_min
--		widget.hx_max=def.hx_max
--		widget.hy_max=def.hy_max
		
		widget.px=widget.px or 0 -- relative to parent, pixel position
		widget.py=widget.py or 0

		widget.hx=widget.hx or 0 -- absolute pixel size of widget (in parents space)
		widget.hy=widget.hy or 0
		widget.hz=widget.hz or 0 -- used to signal an fbo with a depth buffer
		

--		widget.outline_size=def.outline_size
--		widget.outline_color=def.outline_color
--		widget.outline_fade_color=def.outline_fade_color
		
--		widget.transparent=def.transparent -- transparent color tint

--		widget.color=def.color
		
--		widget.cursor=def.cursor
--		widget.drag=def.drag
		
		widget.font=widget.font or widget.parent.font --  use this font if set or inherit value from parent
		
		widget.text_color=widget.text_color or widget.parent.text_color -- black text
--		widget.text_color_over=def.text_color_over -- if set, switch text color on hover
--		widget.text_color_shadow=def.text_color_shadow  -- may need a shadow
--		widget.text_size=def.text_size
--		widget.text_align=def.text_align -- default is "center", and "wrap" will wrap the text
		
--		widget.grid_size=def.grid_size

--		widget.sheet=def.sheet -- display this sheet (by name) on the button
--		widget.sheet_id=def.sheet_id
--		widget.sheet_px=def.sheet_px
--		widget.sheet_py=def.sheet_py
--		widget.sheet_hx=def.sheet_hx
--		widget.sheet_hy=def.sheet_hy

--		widget.draw_text=def.draw_text -- special text draw function probably nil
		
--		widget.text=def.text -- display this text on the button
--		widget.style=def.style -- style the button this way
--		widget.skin=def.skin -- skin the button this way
		
--		widget.hidden=def.hidden -- start off hidden?


-- remove auto solid, need to make sure that all buttons now have a class of button.
--		if widget.hooks then widget.solid=true end
--		widget.solid=widget.solid or def.solid
		
		if widget.class and wmeta.classes[widget.class] then -- got a class, call its setup, its setup can override other functions
			wmeta.classes[widget.class].setup(widget,def)
		end
		
		if widget.master.ids and widget.id then widget.master.ids[widget.id]=widget end -- lookup by id
		
		if def.fbo then -- an fbo buffer has been requested (can speed rendering up)
			widget.fbo=framebuffers.create(0,0,0)
--			widget.fbo_fov=def.fbo_fov
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
		local m=tardis.m4.new()
		w.m4:inverse(m)
		local v=tardis.v4.new(x or 0,y or 0,0,1)
		v:product(m)
		return v[1],v[2]
	end
	
	function meta.mouse(widget,act,_x,_y,keyname)
		
		local x,y=widget:mousexy(_x,_y)

		local nudge=0--widget.outline_size or 0 -- allow clicking outside
		local tx=(x-(widget.pan_px or 0))
		local ty=(y-(widget.pan_py or 0))
		if widget==widget.master or ( tx>=0-nudge and tx<widget.hx+nudge and ty>=0-nudge and ty<widget.hy+nudge ) then

			if widget.solid then
				if (not widget.master.dragging()) --[[or widget.master.active==widget]] then
					widget.master.over=widget
				end
			end

			for i,v in ipairs(widget) do -- children must be within parent bounds to catch clicks
				if not v.hidden then v:mouse(act,_x,_y,keyname) end
			end

		else
		
			if (not widget.master.dragging()) and widget.master.over==widget then
				widget.master.over=nil
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
	function meta.clean_all(widget)
	
		for i,v in ipairs(widget) do
			v:clean_all()
		end
		
		local len=#widget
		for i=1,len do
			if widget[i] then
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

-- more setup, moved to other files
	oven.rebake("wetgenes.gamecake.widgets.meta_layout").setup(def)

end


return wmeta
end
