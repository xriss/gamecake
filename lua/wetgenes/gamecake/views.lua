--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local ls=function(t) print( require("wetgenes.string").dump(t) ) end

-- Layout replacement...
-- Maintain a hierarchical system of views so we can render into sub parts of the main screen.
-- Handle with switching to and from FBO as render targets.


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,views)
		
	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local canvas=cake and cake.canvas

-- this stack is for master views, so push/pop when entering a subsystem that needs to be resized
-- the oven creates a master win related view at startup

	views.stack={}
	views.peek=function() return assert(views.stack[#views.stack]) end
	views.get=views.peek
	views.push=function(view)
		views.stack[#views.stack+1]=view
	end
	views.pop=function()
		local view=views.peek()
		views.stack[#views.stack]=nil
		return view
	end
	views.apply=function() local view=views.peek() if view then view.apply() end end

	views.push_and_apply=function(view)
		views.push(view)
		views.apply()
		return view
	end

	views.pop_and_apply=function()
		views.pop()
		views.apply()
	end

	views.push_and_apply_fbo=function(fbo)
		local view=views.create({
			mode="fbo",
			fbo=fbo,
			vx=fbo.w,
			vy=fbo.h,
			vz=fbo.h*2,
			fov=0,
		})
		views.push(view)
		views.apply()
 		return view
	end

	
-- create a view, for main screen or FBO.
	views.create=function(opts)
		local view={}
				
		view.parent=opts.parent	-- parent link
		if view.parent then view.master=view.parent.master else view.master=view end -- master link

		view.mode=opts.mode -- should we build px,py,hx,hy

		view.win=opts.win -- size can be linked to this win
		view.fbo=opts.fbo -- or this fbo

		view.px=opts.px	-- or set explicitly
		view.py=opts.py
		view.hx=opts.hx
		view.hy=opts.hy

		view.cx=opts.cx or 0.0	-- set starting point on screen 0,0,0 topleft 0.5,0.5,0.0 center of screen and 1.0,1.0,0.0 is bottom right
		view.cy=opts.cy or 0.0
		view.cz=opts.cz or 0.0

-- the projection view size, mostly aspect, that we will be aiming for
		if view.win then
			view.vx=opts.vx or view.win.width
			view.vy=opts.vy or view.win.height
		elseif view.fbo then
			view.vx=opts.vx or view.fbo.w
			view.vy=opts.vy or view.fbo.h
		else
			view.vx=opts.vx -- width
			view.vy=opts.vy -- height
		end

		view.vx_auto=not opts.vx
		view.vy_auto=not opts.vy
		view.vz_auto=not opts.vz
		view.scale_auto=not opts.scale

		view.vz=opts.vz or (view.vy and view.vy*2)-- depth range of the zbuffer

		view.fov=opts.fov or 0	-- field of view, a tan like value, so 1 would be 90deg, 0.5 would be 45deg and so on
								-- with 0 triggering a no perspective mode
								-- this is used directly as the Z multiplier in the matrix for Z and W
								
		view.fov_lock=opts.fov_lock or 0	-- set to 1 to lock to X or 2 to lock to Y otherwise we auto pick to fit

		view.scale=opts.scale or ( view.vy and 2/view.vy ) -- camera scale fix, should probably be 2/vy for 2d screens

-- these values are updated when you call update()
		view.pmtx=M4() 	-- projection matrix
		view.pinv=M4() 	-- inverse projection matrix

		view.cmtx=M4() 	-- 2D camera matrix 
		view.cinv=M4() 	-- 2D inverse camera matrix

		view.port=V4()	-- view port x,y,w,h

		view.sx=1	-- view scale
		view.sy=1

		view.resize=function()	-- update px,py,hx,hy

			if view.parent then view.parent.resize() end -- make sure parent is correct size
		
			if type(view.mode)=="function" then -- special

				view.mode(view)

			elseif view.mode=="win" then
			
				view.win:info()
				view.px=0
				view.py=0
				view.hx=view.win.width
				view.hy=view.win.height

				if view.vx_auto then view.vx=view.hx end
				if view.vy_auto then view.vy=view.hy end
				if view.vz_auto then view.vz=2*view.vy end
				if view.scale_auto then view.scale=2/view.vy end

			elseif view.mode=="fbo" then

				view.px=0
				view.py=0
				view.hx=view.fbo and view.fbo.w or 1
				view.hy=view.fbo and view.fbo.h or 1

				if view.vx_auto then view.vx=view.hx end
				if view.vy_auto then view.vy=view.hy end
				if view.vz_auto then view.vz=2*view.vy end
				if view.scale_auto then view.scale=2/view.vy end

			elseif view.mode=="full" then

				view.px=view.parent.px
				view.py=view.parent.py
				view.hx=view.parent.hx
				view.hy=view.parent.hy

				if view.vx_auto then view.vx=view.hx end
				if view.vy_auto then view.vy=view.hy end
				if view.vz_auto then view.vz=2*view.vy end
				if view.scale_auto then view.scale=2/view.vy end

			elseif view.mode=="clip" then

				view.px=view.parent.px
				view.py=view.parent.py
				view.hx=view.parent.hx
				view.hy=view.parent.hy
				
				if view.vx_auto then view.vx=view.hx end
				if view.vy_auto then view.vy=view.hy end
				if view.vz_auto then view.vz=2*view.vy end
				if view.scale_auto then view.scale=2/view.vy end

				if view.vx and view.vy then
					if view.hx/view.hy > view.vx/view.vy then -- fit y
						local hx=view.hy * view.vx/view.vy
						view.px=view.px+math.floor((view.hx-hx)/2)
						view.hx=hx
					else
						local hy=view.hx * view.vy/view.vx
						view.py=view.py+math.floor((view.hy-hy)/2)
						view.hy=hy
					end
				end

			elseif view.mode=="raw" then -- use the parents raw size

				view.px=view.parent.px
				view.py=view.parent.py
				view.hx=view.parent.hx
				view.hy=view.parent.hy
				view.vx=view.parent.hx
				view.vy=view.parent.hy

				if view.vx_auto then view.vx=view.hx end
				if view.vy_auto then view.vy=view.hy end
				if view.vz_auto then view.vz=2*view.vy end
				if view.scale_auto then view.scale=2/view.vy end

			end


			return view
		end
		
		view.update=function()	-- update pmtx and port

			view.resize() -- possibly check parents size as well


-- calculate opengl viewport
			view.port[1]=view.px
			view.port[2]=view.master.hy-view.hy-view.py		-- we are reversing y so need to use the height of the master view
			view.port[3]=view.hx
			view.port[4]=view.hy
			

--	layout.project23d = function(width,height,fov,depth)
		
			view.pmtx:identity()
			
			local va=view.vy/view.vx
			local ha=view.hy/view.hx
			
--PRINT(view.mode,view.px,view.py,view.vx,view.vy,view.hx,view.hy,va,ha,va/ha,view.scale)

-- after dealing with fov here you can assume a y of +-1 will get you to the edge of your view space (fited to the real screen)
-- and x will depend on your aspect so +-1.7777... for a standard 1920x1080 screen to give square pixels

-- sadly we can not reliably use a reversed Z buffer or even a 0-1 Z buffer as glClipControl is missing in GLES so we waste a lot of precision.
-- could possibly still reverse using glDepthRangef but that seems a bad idea without glClipControl

			if view.master.stretch then

				view.fov_axis=0 -- we are not keeping an aspect ratio

				view.sx=1
				view.sy=1

				view.pmtx[1] = va			-- X = X mul
				view.pmtx[6] = -1			-- Y = Y mul
								
			elseif ( view.fov_lock==1 ) or ( ( view.fov_lock~=2 ) and ( ha > va ) ) then 	-- fit width to screen
			
				view.fov_axis=1
				
				view.sx=1
				view.sy=ha/va

				view.pmtx[1] =   va    		-- X = X mul
				view.pmtx[6] = -(va/ha)		-- Y = Y mul
				
			else	--  view.fov_lock==2	-- fit height to screen
			
				view.fov_axis=2

				view.sx=va/ha
				view.sy=1

				view.pmtx[1] = ha			-- X = X mul
				view.pmtx[6] = -1			-- Y = Y mul
				
			end

-- depth buffer fix an fov of 0 is a uniform projection

--[[

only use half the Z buffer and infinite perspective depth (no clip far and 
virtually no clip near) orthogonal uses the view.z as max depth and -view.z as 
min depth to behave like perspective

z/w is in the 0.0 to 1.0 range so the depth buffer is in 0.5 to 1.0 we actually 
bleed out of this range to near 0 since we cant clip at 0.5

Ah mybad we can not use glDepthRange as WebGL hates the depth buffer 
(in mmany ways), so back to the icky default way it is.

]]

			if view.fov==0 then

				view.pmtx[11] = -1/view.vz		-- Z = Z mul  -- clip to a max of +-vz
				view.pmtx[15] = 0				-- Z = Z add
				
				view.pmtx[12] = 0				-- W = Z mul
				view.pmtx[16] = 1				-- W = W add

			else

				view.pmtx[11] = -view.fov		-- Z = Z mul
				view.pmtx[15] = 0				-- Z = Z add

				view.pmtx[12] = -view.fov		-- W = Z mul
				view.pmtx[16] = 1				-- W = W add
				
			end


			view.pmtx:inverse( view.pinv )

			view.cmtx:identity() -- a default camera matrix for 2d views
			
			if view.scale then
				view.cmtx:scale( view.scale )
			end

			view.cmtx:translate( view.vx*(view.cx-0.5) , view.vy*(view.cy-0.5) , 0 )

			view.cmtx:inverse( view.cinv )

			return view
		end

		
		view.apply=function()		-- apply current setting to opengl, update the viewport and projection/modelview matrix
		
			view.update()
		
			gl.Viewport(view.port[1],view.port[2],view.port[3],view.port[4])

			gl.MatrixMode(gl.PROJECTION)
			gl.LoadMatrix( view.pmtx )

			gl.MatrixMode(gl.MODELVIEW)
			gl.LoadMatrix( view.cmtx )

			return view
		end

-- convert rawx,rawy to x,y in msg		
		view.msg=function(msg)

			if msg.xraw and msg.yraw then	-- we need to fix raw x,y mouse numbers
			
				local hx=view.win and view.win.width  or view.hx -- we should use win size if we are allowed
				local hy=view.win and view.win.height or view.hy
			
				local va=V4( 2*(msg.xraw-hx*0.5)/hx , -2*(msg.yraw-hy*0.5)/hy , 0 , 1 ) -- convert to -1 +1 range
				local vb=va*view.pinv
				local vc=vb*view.cinv

				msg.x=vc[1]
				msg.y=vc[2]

--	print( msg.xraw , msg.yraw , va , vb , vc )
--				msg.x=( view.vx * ( (msg.xraw-(view.hx*0.5+view.px)) * view.sx ) / view.hx ) + view.vx*(0.5-view.cx)
--				msg.y=( view.vy * ( (msg.yraw-(view.hy*0.5+view.py)) * view.sy ) / view.hy ) + view.vy*(0.5-view.cy)

				return true
			end

		end
		
		view.update()
		
		return view
	end

	return views
end
