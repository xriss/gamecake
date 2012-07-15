-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- generic default widget functions


module("wetgenes.gamecake.widget.meta")

-- available widget classes
classes={

-- base classes

	["master"]=require("wetgenes.gamecake.widget.master"),
	["button"]=require("wetgenes.gamecake.widget.button"),
	["drag"]=require("wetgenes.gamecake.widget.drag"),
	["text"]=require("wetgenes.gamecake.widget.text"),
	["textedit"]=require("wetgenes.gamecake.widget.textedit"),

--classes built out of the base classes

	["pan"]=require("wetgenes.gamecake.widget.pan"),
	["slide"]=require("wetgenes.gamecake.widget.slide"),

	["scroll"]=require("wetgenes.gamecake.widget.scroll"),

}

--
-- add meta functions
--
function setup(def)

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
	function meta.add(parent,def)
		
		local widget={}
		setmetatable(widget,meta)
		table.insert(parent,widget)
		widget.parent=parent
		widget.master=parent.master
		widget:setup(def)
		widget.meta=meta		
		return widget
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
		widget.user=def.user
		widget.hooks=def.hooks
		
		widget.sx=def.sx or def.hx or 1 -- (ratio)size for layout code
		widget.sy=def.sy or def.hy or 1 -- use hx and hy if its provided
		
		widget.mx=def.mx or 0 -- max (ratio)size for layout code
		widget.my=def.my or 0
		

		
		-- if set these will generate rx,ry
		widget.pxf=def.pxf      -- local position, for sliders etc, goes from 0-1 
		widget.pyf=def.pyf      -- fractional position within container

		widget.px=def.px or 0 -- relative pixel position (may generate)
		widget.py=def.py or 0
		
		widget.pxd=def.pxd or 0 -- absolute pixel position (very probably generated)
		widget.pyd=def.pyd or 0
		
		widget.pa=def.pa or 0 -- display rotation angle, possibly

		
		-- if set these will generate hx,hy
		widget.hxf=def.hxf	  -- optional relative local size of container, possibly best not to use
		widget.hyf=def.hyf	  -- it does not have a default so may not be set
		
		widget.hx=def.hx or 0 -- absolute pixel size (may generate)
		widget.hy=def.hy or 0
		
		widget.hx_max=def.hx_max -- clip maximum layout size
		widget.hy_max=def.hy_max		
				
		widget.hx_fill=def.hx_fill -- if we wish to stretch this layout then this widget can fill up
		widget.hy_fill=def.hy_fill -- this much extra space where 1 is all of the avilable extra space

		widget.color=def.color
		widget.text_color=def.text_color or widget.master.text_color or 0xff000000 -- black text
		widget.text_size=def.text_size or widget.master.text_size or 16 -- quite chunky text by default
		
		widget.text_color_over=def.text_color_over -- if set, switch text color on hover
		widget.text_align=def.text_align -- default is center
		
		widget.text=def.text -- display this text on the button
		
		if widget.color or widget.text then widget.solid=true end
		widget.solid=widget.solid or def.solid
		
		if widget.class and classes[widget.class] then -- got a class, call its setup, its setup can override other functions
			classes[widget.class].setup(widget,def)
		end
		
		if widget.master.ids and widget.id then widget.master.ids[widget.id]=widget end -- lookup by id
		
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
--
-- live adjustment
--
	function meta.get(widget,val,...)
	
		if val=="slide" then
		
			local x=(widget.pxd-widget.parent.pxd) / (widget.parent.hx-widget.hx)
			local y=(widget.pyd-widget.parent.pyd) / (widget.parent.hy-widget.hy)
			
			
			return x,y
		end
		
	end
	
	function meta.set(widget,val,...)
	local t={...}
	
		if val=="slide" then
			for i,v in ipairs(widget) do
			
				if type(t[1])=="table" then
					v.pxf=t[1][1] or v.pxf or 0
					v.pyf=t[1][2] or v.pyf or 0
				else
					v.pxf=t[1] or v.pxf or 0
					v.pyf=t[2] or v.pyf or 0
				end
				
--print("SET",v.pxf,v.pyf)

				v.px=(widget.hx-v.hx)*v.pxf -- local position relative to parents size
				v.py=(widget.hy-v.hy)*v.pyf
				
				v.pxd=widget.pxd+v.px -- absolute
				v.pyd=widget.pyd-v.py
				
			end
		end
	end
	
--
-- initial layout of widgets, to put them into reasonable positions
--
	function meta.layout(widget)
--print(widget.class)
		if widget.class=="flow" or widget.class=="hx" then -- hx will be removed
			meta.layout_flow(widget)
		elseif widget.class=="fill" or widget.class=="pan" then
			meta.layout_fill(widget)
		elseif widget.class=="slide" or widget.class=="pad" then
			meta.layout_padding(widget)
		elseif widget.class=="master" or widget.class=="abs" then
			meta.layout_base(widget)
		else
			meta.layout_base(widget)
		end
	end
	
	function meta.layout_none(widget)
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	
	function meta.layout_padding(widget)
		for i,v in ipairs(widget) do

			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end
			
			v.px=(widget.hx-v.hx)*v.pxf -- local position relative to parents size
			v.py=(widget.hy-v.hy)*v.pyf

			v.pxd=widget.pxd+v.px -- local absolute position
			v.pyd=widget.pyd-v.py

		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	

	function meta.layout_base(widget)
		for i,v in ipairs(widget) do
		
			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end
			
			if v.pxf then v.px=(widget.hx)*v.pxf end -- local position relative to parents size
			if v.pyf then v.py=(widget.hy)*v.pyf end

			v.pxd=widget.pxd+v.px -- absolute position
			v.pyd=widget.pyd-v.py
			
		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end

-- this is a fixed layout that works kind of like text
-- we do not adjust the hx,hy size of sub widgets
-- we just place them left to right top to bottom
-- finally we resize this widget to fit its content
-- the widgets sx,sy is used as default hx,hy for layout
	function meta.layout_fill(widget)
		
		local hx,hy=0,0
		local my=0
		local mhx,mhy=0,0
		function addone(w)
			w.px=hx
			w.py=hy
			hx=hx+w.hx
			if hx > mhx then mhx=hx end -- max x total size
			if w.hy > my then my=w.hy end -- max y size for this line
--print(w.id or "?",w.px,w.py,w.hx,w.hy)
		end
		
		function endoflines()
			widget.hx=mhx
			widget.hy=mhy
		end
		
		function endofline()
			hx=0
			hy=hy+my
			my=0
			mhy=hy
		end
		
		if #widget>0 then
		
			widget.hx=widget.sx -- use sx,sy as the base fill size
			widget.hy=widget.sy
		
			for i,w in ipairs(widget) do
			
				if hx+w.hx>widget.hx then
					if hx==0 then -- need one item per line so add it anyway
						addone(w)
						endofline()
					else -- skip this one, push it onto nextline
						endofline()
						addone(w)
					end
				else -- it fits so just add
					addone(w)
				end
			end

			if hx>0 then endofline() end -- final end of line
			
			endoflines()
			
		end
		
		for i,v in ipairs(widget) do
			v.pxd=widget.pxd+v.px
			v.pyd=widget.pyd-v.py
		end

-- layout sub sub widgets	
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	
-- this is the magical layout that works like text
-- except things expand to fit the area
-- use sx,sy and mx,my to control what ends up where
	function meta.layout_flow(widget)
		local sx,sy=0,0
		local my=0
		local line=1
		
		function endoflines()
			local y=0
			for i,v in ipairs(widget) do
				v.hy=v.sy*widget.hy/sy
				if v.hy_max and v.hy > v.hy_max then v.hy = v.hy_max end
				v.py=y
				if v.endofline then y=y+v.hy end
--print(v.px..","..v.py.." - "..v.hx..","..v.hy)
			end
		end
		
		function endofline(i)
			local x=0
			for i=line,i do -- final line layout
				local v=widget[i]
				v.sy=my
				v.hx=v.sx*widget.hx/sx
				if v.hx_max and v.hx > v.hx_max then v.hx = v.hx_max end
				v.px=x
				x=x+v.hx
			end
			widget[i].endofline=true
			sx=0
			sy=sy+my
			my=0
			line=i+1
		end
		
		if #widget>0 then
			for i,v in ipairs(widget) do
			
				v.endofline=false
				if sx+v.sx>widget.mx then
					if sx==0 then -- only one on line
						if v.sy>my then my=v.sy end
						sx=sx+v.sx				
						endofline(i)
					else -- skip this one, push onto nextline
						endofline(i-1)
						if v.sy>my then my=v.sy end
						sx=sx+v.sx				
					end
				else
					if v.sy>my then my=v.sy end
					sx=sx+v.sx				
				end
			end

			endofline(#widget)
			endoflines()
		end
		
		for i,v in ipairs(widget) do
			v.pxd=widget.pxd+v.px
			v.pyd=widget.pyd-v.py
		end

		for i,v in ipairs(widget) do
			v:layout()
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

		if widget.pan_px then x=x-widget.pan_px end
		if widget.pan_py then y=y-widget.pan_py end

		if widget.solid and x>=widget.pxd and x<widget.pxd+widget.hx and y<=widget.pyd and y>widget.pyd-widget.hy then
		
			if act=="down" then
-- only set if null or our parent...
				if not widget.master.active or widget.master.active==widget.parent then
					widget.master.active=widget
					widget.master.active_x=x-widget.pxd
					widget.master.active_y=y-widget.pyd
				end
			end
			if act=="up" then
				if widget.master.active and widget.master.active==widget then -- widget clicked
					widget:call_hook("click")
				end
			end

			if not widget.master.active or widget.master.active==widget then -- over widget
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

end
