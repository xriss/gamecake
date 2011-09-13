-- widget class master
-- the master widget



local gl=require('gl')


local math=math
local table=table

local ipairs=ipairs
local setmetatable=setmetatable
local type=type

local function print(...) _G.print(...) end



module("fenestra.widget.master")

--
-- add meta functions
--
function setup(widget,def)

	local master=widget
	local meta=widget.meta
--	local win=def.win

-- the master gets some special overloaded functions to do a few more things
	function master.update(widget)
		meta.update(widget)
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
	
	function master.mouse(widget,act,x,y,key)
	
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
