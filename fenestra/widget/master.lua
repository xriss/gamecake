-- widget class master
-- the master widget



local gl=require('gl')


local math=math
local table=table

local ipairs=ipairs
local setmetatable=setmetatable
local type=type
local tostring=tostring

local function print(...) _G.print(...) end



module("fenestra.widget.master")

--
-- add meta functions
--
function setup(widget,def)

	local master=widget
	local meta=widget.meta
--	local win=def.win

	master.throb=0

-- the master gets some special overloaded functions to do a few more things
	function master.update(widget)
		widget.throb=widget.throb-4
		if widget.throb<0 then widget.throb=255 end

		meta.update(widget)
	end
	
	function master.layout(widget)
		meta.layout(widget)
		master.remouse(widget)
	end

	function master.draw(widget)
	
		gl.Disable(gl.CULL_FACE)
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.DEPTH_TEST)
		gl.PushMatrix()
		meta.draw(widget)
		gl.PopMatrix()
--		gl.Enable("DEPTH_TEST")
--		gl.Enable("LIGHTING")
	end
	
--
-- handle key input
--
	function master.key(widget,ascii,key,act)

		if master.focus then -- key focus
		
			master.focus:key(ascii,key,act)
		end

	end

--
-- set the mouse position to its last position
-- call this after adding/removing widgets to make sure they highlight properly
--	
	function master.remouse(widget)
		local p=widget.last_mouse_position or {0,0}
		widget.mouse(widget,nil,p[1],p[2],nil)
	end
--
-- handle mouse input
--	
	function master.mouse(widget,act,x,y,key)
	
		master.last_mouse_position={x,y}
	
--		if widget.state=="ready" then
		
			if master.active and master.active.parent.class=="slide" then -- slide :)
			
				local w=master.active
				local p=master.active.parent
				
				local minx=p.px
				local miny=p.py-p.hy+w.hy
				local maxx=p.px+p.hx-w.hx
				local maxy=p.py
				
				w.px=x-master.active_x
				w.py=y-master.active_y
				
				if w.px<minx then w.px=minx end
				if w.px>maxx then w.px=maxx end
				if w.py<miny then w.py=miny end
				if w.py>maxy then w.py=maxy end
			
				w:call_hook("slide")

			end
		
			for i,v in ipairs(widget) do
				meta.mouse(v,act,x,y,key)
			end
			
			if act=="up" then
				master.active=nil
			end
			
--		end
	end
--
end
