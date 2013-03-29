-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generic default widget functions


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmeta)
wmeta=wmeta or {}

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")


-- available widget classes
wmeta.classes={

-- base classes

	["master"]=oven.rebake("wetgenes.gamecake.widgets.master"),
	["button"]=oven.rebake("wetgenes.gamecake.widgets.button"),
	["drag"]=oven.rebake("wetgenes.gamecake.widgets.drag"),
	["text"]=oven.rebake("wetgenes.gamecake.widgets.text"),
	["textedit"]=oven.rebake("wetgenes.gamecake.widgets.textedit"),

--classes built out of the base classes

	["pan"]=oven.rebake("wetgenes.gamecake.widgets.pan"),
	["slide"]=oven.rebake("wetgenes.gamecake.widgets.slide"),

	["scroll"]=oven.rebake("wetgenes.gamecake.widgets.scroll"),

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
		while (widget.parent ~= widget) and not widget.parent.dirty do
			widget=widget.parent
			widget.dirty=true
		end
	end

	function meta.call_hook(widget,hook,dat)
		local hooks=widget.hooks or widget.master.hooks
		local type_hooks=type(hooks)
		if type_hooks=="function" then -- master function
			hooks(hook,widget,dat)
		elseif type_hooks=="table" and hooks[hook] then -- or table of functions
			hooks[hook](widget,dat)
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
	function meta.insert(parent,widget)
	
		meta.remove(widget) -- make sure we dont end up in two parents
		
		table.insert(parent,widget)
		widget.parent=parent
		widget.master=parent.master
		
		return widget
	end

--
-- initial setup
--def
	function meta.setup(widget,def)
	
		widget.state="none"
		
		widget.meta=meta
		
		widget.data=def.data -- this widget is synced with this data
		
		widget.class=def.class
		widget.highlight=def.highlight
		
		widget.id=def.id
		widget.user=def.user -- any user data you wish to associate with this widget (we will ignore it)
		widget.hooks=def.hooks

		widget.clip=def.clip -- use viewport clipping to prevent drawing outside of the area

		widget.sx=def.sx or 1 -- display scale (of children)
		widget.sy=def.sy or 1
		widget.pa=def.pa or 0 -- display rotation angle (of children)
		
		
		widget.px=def.px or 0 -- relative to parent, pixel position
		widget.py=def.py or 0

		widget.hx=def.hx or 0 -- absolute pixel size of widget
		widget.hy=def.hy or 0
		

		-- turn this into a matrix? so we can rotate and stuff
		widget.pxd=def.pxd or 0 -- INTERNAL absolute pixel display position ( generated from px,py )
		widget.pyd=def.pyd or 0


		widget.color=def.color
		
		
		widget.font=def.font or widget.parent.font --  use this font if set or inherit value from parent
		
		widget.text_color_shadow=def.text_color_shadow  -- may need a shadow
		widget.text_color_over=def.text_color_over -- if set, switch text color on hover
		widget.text_color=def.text_color or widget.parent.text_color or 0xff000000 -- black text
		widget.text_size=def.text_size
		widget.text_align=def.text_align -- default is "center", and "wrap" will wrap the text
		

		widget.text=def.text -- display this text on the button
		widget.sheet=def.sheet -- display this sheet (by name) on the button
		widget.style=def.style -- style the button this way


		
		if widget.color or widget.text or widget.sheet then widget.solid=true end
		widget.solid=widget.solid or def.solid
		
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
		return widget
	end


-- get a member from this or parents... widget
	function meta.bubble(widget,name)
		local w=widget
		repeat
			if w[name] then return w[name] end
			w=w.parent
		until not w
	end

--
-- live adjustment
--
	function meta.get(widget,val,...)
	
		if val=="slide" then
		
--			local x=(widget.pxd-widget.parent.pxd) / (widget.parent.hx-widget.hx)
--			local y=(widget.pyd-widget.parent.pyd) / (widget.parent.hy-widget.hy)
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
				
				v.pxd=widget.pxd+v.px -- absolute
				v.pyd=widget.pyd+v.py
				
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
	function meta.mouse(widget,act,x,y,key)
	
--print(x..","..y.." : "..widget.px..","..widget.py)

		if widget.pan_px then x=x+widget.pan_px end
		if widget.pan_py then y=y+widget.pan_py end

		if widget.solid and x>=widget.pxd and x<widget.pxd+widget.hx and y>=widget.pyd and y<widget.pyd+widget.hy then
		
			if act==1 then
-- only set if null or our parent...
--print(widget,widget.class)
--print("active",widget,widget and widget.class,
--widget and widget.parent,widget and widget.parent.class)
				if not widget.master.active or widget.master.active==widget.parent then
					widget.master.active=widget
					widget.master.active_x=x-widget.pxd
					widget.master.active_y=y-widget.pyd
				end
			end
			if act==-1 then
				if not widget.master.dragging() or widget.master.active==widget then
--				if widget.master.active and widget.master.active==widget then -- widget clicked
					widget:call_hook("click")
				end
			end

			if not widget.master.dragging() or widget.master.active==widget then
--			if not widget.master.active or widget.master.active==widget then -- over widget
				widget.master.over=widget
			end
		else
		
			if widget.master.over==widget then
				widget.master.over=nil
			end
		end
	
		for i,v in ipairs(widget) do
			v:mouse(act,x,y,key)
		end
	end
	
--
-- update this widget and its sub widgets
--
	function meta.update(widget)
	
		if widget.anim then
			widget.anim:update()
		end
	
		for i,v in ipairs(widget) do
			v:update()
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
			widget[i]:clean()
			widget[i]=nil
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

-- more setup, moved to other files
	oven.rebake("wetgenes.gamecake.widgets.meta_layout").setup(def)

end


return wmeta
end
