--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")


-- This is all hideously complex, should be replaced with something "simple" and a bit more stack based as the stack was retro fitted here.
-- we also need with switching to and from fbo as render targets, again this has just been hacked in.
-- so stop using this and start again, lets call the new one views...


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,layouts)
		
	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local canvas=cake.canvas

-- manage a simple stack of layouts
	local stack
	layouts.reset=function()
		stack={}
		layouts.push( layouts.create{} )
		return layouts.get()
	end
	layouts.pop=function()
		local l=assert(stack[#stack])
		stack[#stack]=nil
		return l
	end
	layouts.push=function(l)
		stack[#stack+1]=l
		return l
	end
	layouts.get=function()
		return assert(stack[#stack])
	end
	layouts.push_child=function(opts)
		opts=opts or {}
		opts.parent=layouts.get()
		return layouts.push( layouts.create(opts) )
	end
	layouts.project23d=function(...) return layouts.get().project23d(...) end
	layouts.xyscale   =function(...) return layouts.get().xyscale(...)    end
	layouts.build     =function(...) return layouts.get().build(...)      end
	layouts.restore   =function(...) return layouts.get().restore(...)    end
	layouts.viewport  =function(...) return layouts.get().viewport(...)   end
	layouts.apply     =function(...) return layouts.get().apply(...)      end

-- generate functions locked to the canvas
	layouts.create = function(opts)
	local layout={}
	
	layout.parent=opts.parent
	
	if layout.parent then
		layout.x=opts.x or layout.parent.x
		layout.y=opts.y or layout.parent.y
		layout.w=opts.w or layout.parent.w
		layout.h=opts.h or layout.parent.h
		layout.overscale=opts.overscale or 1
	else
		layout.x=opts.x or 0
		layout.y=opts.y or 0
		layout.w=opts.w or win.width
		layout.h=opts.h or win.height
		layout.overscale=opts.overscale or win.overscale or 1
	end

-- set all vars to 1 so anycode using them before they get set will not crash
	layout.view_width=1
	layout.view_height=1
	layout.view_fov=1
	layout.view_depth=1
	layout.x_scale=1
	layout.y_scale=1
	layout.x_origin=1
	layout.y_origin=1
	layout.x_size=1
	layout.y_size=1
	
--
-- build a simple field of view projection matrix designed to work in 2d or 3d and keep the numbers
-- easy for 2d positioning.
--
-- setting aspect to 640,480 and fov of 1 would mean at a z depth of 240 (which is y/2) then your view area would be
-- -320 to +320 in the x and -240 to +240 in the y.
--
-- fov is a tan like value (a view size inverse scalar) so 1 would be 90deg, 0.5 would be 45deg and so on
--
-- the depth parameter is only used to limit the range of the zbuffer so it covers 0 to depth
--
-- The following would be a reasonable default for a 640x480 canvas.
--
-- build_project23d(640,480,0.5,1024)
--
-- then at z=((480/2)/0.5)=480 we would have one to one pixel scale...
-- the total view area volume from there would be -320 +320 , -240 +240 , -480 +(1024-480)
--
-- canvas needs to contain width and height of the display which we use to work
-- out where to place our view such that it is always visible and keeps its aspect.
--
	layout.project23d = function(width,height,fov,depth)
		
		local l=layout -- we always have layout
		
		local aspect=height/width
		
		layout.view_width=width
		layout.view_height=height
		layout.view_fov=fov
		layout.view_depth=depth

		local m={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- defaults
		layout.pmtx=m
		
		local f=depth
		local n=1

		local layout_aspect=(l.h/l.w)
		
		if fov==0 then
		
			if (layout_aspect > (aspect) ) 	then 	-- fit width to screen
			
				m[1] = 2/l.w
				m[6] = -2/l.h
				
				layout.x_scale=1
				layout.y_scale=layout_aspect/aspect
			else									-- fit height to screen
			
				m[1] = 2/l.w
				m[6] = -2/l.h
				
				layout.x_scale=aspect/layout_aspect
				layout.y_scale=1
			end

		else
			
			if (layout_aspect > (aspect) ) 	then 	-- fit width to screen
				m[1] = ((aspect)*1)/fov
				m[6] = -((aspect)/layout_aspect)/fov
				
				layout.x_scale=1
				layout.y_scale=layout_aspect/aspect
			else									-- fit height to screen
			
				m[1] = layout_aspect/fov
				m[6] = -1/fov
				
				layout.x_scale=aspect/layout_aspect
				layout.y_scale=1
			end
		end

		layout.x_origin=l.x+l.w/2
		layout.y_origin=l.y+l.h/2
		layout.x_size=l.w
		layout.y_size=l.h

	-- we reposition with the viewport, so only need to fix the size in the matrix when using a layout.	
		
		if fov==0 then
			m[11] = -2/(f+n)
			m[15] = -(f+n)/(f-n)
			m[16] = 1
		else
			m[11] = -(f+n)/(f-n)
			m[12] = -1
			m[15] = -2*f*n/(f-n)
		end

		
-- apply overscale?

		if layout.overscale and layout.overscale~=1 then
		
			m[1] = m[1] * layout.overscale
			m[6] = m[6] * layout.overscale
			
--			layout.x_size = layout.x_size * layout.overscale
--			layout.y_size = layout.y_size * layout.overscale
			
			layout.x_scale = layout.x_scale / layout.overscale
			layout.y_scale = layout.y_scale / layout.overscale
		
		end
		
		return m -- return the matrix but we also updated the layout size/scale for later use
	end
	

-- convert raw xy coords (IE mouse win width and height) into local coords (view width and height) centered on origin
-- basically do whatever transform we came up with in project23d
	layout.xyscale=function(x,y)

		-- centered and scaled
		x=layout.view_width  * ( (x-layout.x_origin) * layout.x_scale ) / layout.x_size
		y=layout.view_height * ( (y-layout.y_origin) * layout.y_scale ) / layout.y_size

		return x,y
	end

	layout.xyunscale=function(x,y)

		x=((x * layout.x_size/layout.view_width )/layout.x_scale)+layout.x_origin
		y=((y * layout.y_size/layout.view_height)/layout.y_scale)+layout.y_origin
		
		return x,y
	end

-- a viewport clip, resize an area of this aspect ratio to fit into the current layout
-- use this before layout.project23d and you will not have any pixels escaping out of
-- your draw area
	layout.build=function(width,height,x,y)

		local l=layout.parent or {x=0,y=0,w=win.width,h=win.height}
		
--[[
		if not l then -- screen , apply scale here?
			l={x=0,y=0,w=0,h=0}
			l.w=math.floor(win.width*win.overscale)
			l.h=math.floor(win.height*win.overscale)
			l.x=math.floor((win.width-l.w)/2)
			l.y=math.floor((win.height-l.h)/2)
		end
]]
		if not width or not height then -- full screen

			layout.x=l.x
			layout.y=l.y
			layout.w=l.w
			layout.h=l.h

			return
			
		elseif x and y then -- force size and position
		
			layout.x=x
			layout.y=y
			layout.w=width
			layout.h=height

			return
			
		else
			layout.x=l.x
			layout.y=l.y
			layout.w=l.w
			layout.h=l.h
			
			local layout_aspect=(l.h/l.w)
			local aspect=height/width		
				
			if layout_aspect > aspect then 			-- fit width to screen
			
				layout.h=l.w*aspect -- our new display height				
				layout.y=l.y+((l.h-layout.h)*0.5) -- centered
				
				return

			else											-- fit height to screen

				layout.w=l.h/aspect -- our new display width
				layout.x=l.x+((l.w-layout.w)*0.5) -- centered
				
				return
			end
		end

	end

	layout.restore=function()

		canvas.layout=layout -- remember current layout in canvas

		if layout.parent then
			gl.Viewport( layout.x , (layout.parent.h-(layout.y+layout.h)) , layout.w , layout.h )
		else
			gl.Viewport( layout.x , win.height-(layout.y+layout.h) , layout.w , layout.h )
		end

	end
	
	layout.viewport=function(width,height,x,y)
	
		-- adjust the fake root layout
		win:info()
		stack[1].x=0
		stack[1].y=0
		stack[1].w=win.width
		stack[1].h=win.height
		
		layout.build(width,height,x,y)
		
		layout.restore()
		
	end

-- this applys a full viewport and adjusts opengls projection and model view stacks
-- push and pop these if you wish to preserve old values
-- returns last applied layout, so you can restore it to undo these changes
	layout.apply=function(w,h,fov,d,clip)
		layout.revert=function() return layout.apply(w,h,fov,d,clip) end
	
		local flag if type(w)=="boolean" then flag=w w=nil end
	
		local ret=canvas.layout
	
		w=w or layout.w
		h=h or layout.h
		d=d or layout.h*4
		fov=fov or 0.25
		
		if clip then

			layout.viewport(w,h)
			
		elseif flag then -- use preset viewport?
		
			layout.viewport(w,h,layout.x,layout.y)

		else

			layout.viewport()

		end
		
		layout.project23d(w,h,fov,d)
	
		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( layout.pmtx )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		if fov==0 then
			gl.Translate(-w/2,-h/2,(-d/2)) -- top left corner is origin
		else
			gl.Translate(-w/2,-h/2,(-h/2)/fov) -- top left corner is origin
		end
		
		return ret
	end

-- does nothing?
--	layout.setup=layout.apply
--	layout.clean=function()
--	end
	
	return layout
end

-- init with a default layout
	layouts.reset()
	layouts.apply()

	return layouts
end
