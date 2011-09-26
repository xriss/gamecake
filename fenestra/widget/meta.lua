-- generic default widget functions


local math=math
local table=table

local ipairs=ipairs
local setmetatable=setmetatable
local type=type

local function print(...) _G.print(...) end


local require=require


module("fenestra.widget.meta")

-- available widget classes
classes={
	["master"]=require("fenestra.widget.master"),
	["string"]=require("fenestra.widget.string"),
	["textedit"]=require("fenestra.widget.textedit"),
}

--
-- add meta functions
--
function setup(def)

	local master=def.master
	local meta=def.meta
	local win=def.win

	function meta.call_hook(widget,hook)
		if widget.hooks and widget.hooks[hook] then -- either local hooks
			widget.hooks[hook](widget)
		elseif widget.master.hooks and widget.master.hooks[hook] then -- or a master hook
			widget.master.hooks[hook](widget)
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
		
		widget.class=def.class
		widget.highlight=def.highlight
		
		widget.id=def.id
		widget.user=def.user
		widget.hooks=def.hooks
		
		widget.sx=def.sx or def.hx or 1 -- (ratio)size for layout code
		widget.sy=def.sy or def.hy or 1 -- use hxy if its provided
		
		widget.mx=def.mx or 0 -- max (ratio)size for layout code
		widget.my=def.my or 0
		

		
		-- if set these will generate rx,ry
		widget.pxf=def.pxf      -- local position, for sliders etc, goes from 0-1 
		widget.pyf=def.pyf      -- fractional position within container

		widget.pxr=def.pxr or 0 -- relative pixel position (may auto generate)
		widget.pyr=def.pyr or 0
		
		widget.px=def.px or 0 -- pixel position (probably auto generated)
		widget.py=def.py or 0
		
		widget.pa=def.pa or 0 -- display rotation angle, possibly

		
		-- if set these will generate hx,hy
		widget.hxf=def.hxf	  -- optional relative local size of container, possibly best not to use
		widget.hyf=def.hyf	  -- it does not have a default so may not be set
		
		widget.hx=def.hx or 0 -- absolute pixel size (may auto generate)
		widget.hy=def.hy or 0
		
		widget.hx_max=def.hx_max -- clip maximum layout size
		widget.hy_max=def.hy_max
		
		
		
		widget.color=def.color
		widget.text_color=def.text_color or master.text_color or 0xff000000 -- black text
		widget.text_size=def.text_size or master.text_size or 16 -- quite chunky text by default
		
		widget.text_color_over=def.text_color_over -- if set, switch text color on hover
		widget.text_align=def.text_align -- default is center
		
		widget.text=def.text -- display this text on the button
		
		if widget.color or widget.text then widget.solid=true end
		widget.solid=widget.solid or def.solid
		
		if widget.class and classes[widget.class] then -- got a class, call its setup, its setup can override other functions
			classes[widget.class].setup(widget,def)
		end
		
		return widget
	end
--
-- and final cleanup
--
	function meta.clean(widget)
		return widget
	end
--
-- live adjustment
--
	function meta.get(widget,val,...)
	
		if val=="slide" then
		
			local x=(widget.px-widget.parent.px) / (widget.parent.hx-widget.hx)
			local y=(widget.py-widget.parent.py) / (widget.parent.hy-widget.hy)
			
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
				
				v.px=widget.px+(widget.hx-v.hx)*v.pxf -- local position relative to parents size
				v.py=widget.py-(widget.hy-v.hy)*v.pyf
				
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
		elseif widget.class=="fill" or widget.class=="fbo" then
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
			
			v.px=widget.px+(widget.hx-v.hx)*v.pxf -- local position relative to parents size
			v.py=widget.py-(widget.hy-v.hy)*v.pyf
			
		end
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	

	function meta.layout_base(widget)
		for i,v in ipairs(widget) do
		
			if v.hxf then v.hx=widget.hx*v.hxf end -- generate size as a fraction of parent
			if v.hyf then v.hy=widget.hy*v.hyf end
			
			if v.pxf then v.pxr=(widget.hx)*v.pxf end -- local position relative to parents size
			if v.pyf then v.pyr=(widget.hy)*v.pyf end

			v.px=widget.px+v.pxr -- local absolute position
			v.py=widget.py-v.pyr
			
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
			w.px=widget.px+hx
			w.py=widget.py-hy -- top align by default
			hx=hx+w.hx
			if hx > mhx then mhx=hx end -- max x total size
			if w.hy > my then my=w.hy end -- max y size for this line
		end
		
		function endoflines()
			widget.hx=mhx
			widget.hy=mhy
		end
		
		function endofline()
			hx=0
			hy=hy+my
			my=0
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
		
-- layout sub sub widgets	
		for i,v in ipairs(widget) do
			v:layout()
		end
	end
	
-- this is the magical layout that works kind of like text
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
				v.py=widget.py-y
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
				v.px=widget.px+x
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

		if widget.solid and x>=widget.px and x<widget.px+widget.hx and y<=widget.py and y>widget.py-widget.hy then
		
			if act=="down" then
-- only set if null or our parent...
				if not master.active or master.active==widget.parent then
					master.active=widget
					master.active_x=x-widget.px
					master.active_y=y-widget.py
				end
			end
			if act=="up" then
				if master.active and master.active==widget then -- widget clicked
					widget:call_hook("click")
				end
			end

			if not master.active or master.active==widget then -- over widget
				master.over=widget
			end
		else
		
			if master.over==widget then master.over=nil end
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

end
