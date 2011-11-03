-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- widget class master
-- the master widget



local gl=require('gl')

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
		
			if master.active and (master.active.parent.class=="slide" or master.active.parent.class=="oldslide") then -- slide :)
			
				local w=master.active
				local p=master.active.parent
				
				local minx=p.pxd
				local miny=p.pyd-p.hy+w.hy
				local maxx=p.pxd+p.hx-w.hx
				local maxy=p.pyd
				
				w.pxd=x-master.active_x
				w.pyd=y-master.active_y
				
				if w.pxd<minx then w.pxd=minx end
				if w.pxd>maxx then w.pxd=maxx end
				if w.pyd<miny then w.pyd=miny end
				if w.pyd>maxy then w.pyd=maxy end
				
				w.px=w.pxd-p.pxd
				w.py=p.pyd-w.pyd
			
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
