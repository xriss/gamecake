-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- widget class master
-- the master widget



local gl=require('gles').gles1

module("wetgenes.gamecake.widget.master")

--
-- add meta functions
--
function setup(widget,def)

	local master=widget
	local meta=widget.meta
--	local win=def.win

	local cake=def.state.cake
	local canvas=def.state.canvas

	master.throb=0
--	master.fbo=_G.win.fbo(0,0,0) -- use an fbo

-- the master gets some special overloaded functions to do a few more things
	function master.update(widget)
	
		local throb=(widget.throb<128)
		
		widget.throb=widget.throb-4
		if widget.throb<0 then widget.throb=255 end
		
		if throb ~= (widget.throb<128) then -- dirty throb...
			if widget.focus then
				if widget.focus.class=="textedit" then
					widget.focus:set_dirty()
				end
			end
		end

		meta.update(widget)
	end
	
	function master.layout(widget)
		meta.layout(widget)
		master.remouse(widget)
	end

--[[
	local dirty_fbos={}
	local find_dirty_fbos
	find_dirty_fbos=function(widget)
		if widget.fbo and widget.dirty then
			dirty_fbos[ #dirty_fbos+1 ]=widget
		end
		for i,v in ipairs(widget) do
			find_dirty_fbos(v)
		end
	end
]]	
	function master.draw(widget)
--[[
		dirty_fbos={}
		find_dirty_fbos(widget)
]]
	


		gl.Disable(gl.CULL_FACE)
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.DEPTH_TEST)
		gl.PushMatrix()
		
--[[
		if #dirty_fbos>0 then
			for i=#dirty_fbos,1,-1 do -- call in reverse so sub fbos can work
				meta.draw(dirty_fbos[i]) -- dirty, so this only builds the fbo
			end
		end
]]		
		meta.draw(widget)
		
		gl.PopMatrix()
--		gl.Enable("DEPTH_TEST")
--		gl.Enable("LIGHTING")
	end
	
	function master.msg(widget,m)
		if m[1]=="key" then
			widget:key(m[2],m[3],m[4])
		elseif m[1]=="mouse" then
			widget:mouse(m[3],m[4],m[5],m[2])
		end
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
print(act,x,y,key)	
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
				
				w:set_dirty()

			end
			
			local old_active=master.active
			local old_over=master.over
			for i,v in ipairs(widget) do
				meta.mouse(v,act,x,y,key)
			end
			
			if act==-1 then
				master.active=nil
			end
			
--mark as dirty
			if master.active~=old_active then
				if master.active then master.active:set_dirty() end
				if old_active then old_active:set_dirty() end
			end
			if master.over~=old_over then
				if master.over then master.over:set_dirty() end
				if old_over then old_over:set_dirty() end
			end
			
--		end
	end
--
end
