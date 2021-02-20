--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require("wetgenes.grd")
local pack=require("wetgenes.pack")
local core=require("wetgenes.gamecake.core")

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

-- Layout replacement...
-- Maintain a hierarchical system of views so we can render into sub parts of the main screen.
-- Handle with switching to and from FBO as render targets.


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,views)
		
	local gl=oven.gl
	local cake=oven.cake
	local win=oven.win
	local canvas=cake.canvas

-- this stack is for master views, so push/pop when entering a subsystem that needs to be resized
-- the oven creates a master win related view at startup

	views.stack={}
	views.get=function() return assert(views.stack[#views.stack]) end
	views.push=function(view)
		views.stack[#views.stack+1]=view
	end
	views.pop=function()
		local view=views.get()
		views.stack[#views.stack]=nil
		return view
	end
	views.apply=function() local view=views.get() if view then view.apply() end end

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
			vz=fbo.h*4,
			fov=1/4,
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

		view.vz=opts.vz or (view.vy and view.vy*4)-- depth range of the zbuffer

		view.fov=opts.fov or 0 -- field of view, a tan like value, so 1 would be 90deg, 0.5 would be 45deg and so on

--		view.ss=opts.overscan or 1 -- scale vx,vy by this value, to allow simple overscan
		
--		view.aspect=opts.aspect or 1 -- pixel aspect fix, normally 1/1 can also set to 0 for scale to total available area

-- these values are updated when you call update()
		view.pmtx=M4{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} 	-- projection matrix
		view.port=V4{0,0,0,0}								-- view port x,y,w,h

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

			elseif view.mode=="fbo" then

				view.px=0
				view.py=0
				view.hx=view.fbo.w
				view.hy=view.fbo.h

			elseif view.mode=="full" then

				view.px=view.parent.px
				view.py=view.parent.py
				view.hx=view.parent.hx
				view.hy=view.parent.hy

			elseif view.mode=="clip" then

				view.px=view.parent.px
				view.py=view.parent.py
				view.hx=view.parent.hx
				view.hy=view.parent.hy
				
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
				view.vz=view.vz or view.parent.hy*4

			end

			view.pz=view.pz or -view.vz/2

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
		
			local m=view.pmtx:identity()
			
			local f=view.vz
			local n=1

			local va=view.vy/view.vx
			local pa=view.hy/view.hx
			
			if view.fov==0 then
			
				if (pa > (va) ) 	then 	-- fit width to screen
				
					view.pmtx[1] = 2/view.hx
					view.pmtx[6] = -2/view.hy
					
					view.sx=1
					view.sy=pa/va

				else									-- fit height to screen
				
					view.pmtx[1] = 2/view.hx
					view.pmtx[6] = -2/view.hy
					
					view.sx=va/pa
					view.sy=1

				end

			else
				
				if view.master.stretch then

					view.pmtx[1] = va/view.fov
					view.pmtx[6] = -1/view.fov
					
					view.sx=1
					view.sy=1

				
				elseif (pa > (va) ) 	then 	-- fit width to screen
				
					view.pmtx[1] = ((va)*1)/view.fov
					view.pmtx[6] = -((va)/pa)/view.fov
					
					view.sx=1
					view.sy=pa/va

				else									-- fit height to screen
				
					view.pmtx[1] = pa/view.fov
					view.pmtx[6] = -1/view.fov
					
					view.sx=va/pa
					view.sy=1

				end
			end

-- depth buffer fix an fov of 0 is a uniform projection
			if view.fov==0 then

				view.pmtx[11] = -2/(f+n)
				view.pmtx[12] = 0
				view.pmtx[15] = -(f+n)/(f-n)
				view.pmtx[16] = 1

			else

				view.pmtx[11] = -(f+n)/(f-n)
				view.pmtx[12] = -1
				view.pmtx[15] = -2*f*n/(f-n)
				view.pmtx[16] = 0

			end

			view.pmtx:translate(-view.vx/2,-view.vy/2,view.pz) -- top left corner is origin

			return view
		end

		
		view.apply=function()		-- apply current setting to opengl, update the viewport and projection/modelview matrix
		
			view.update()
		
			gl.Viewport(view.port[1],view.port[2],view.port[3],view.port[4])

			gl.MatrixMode(gl.PROJECTION)
			gl.LoadMatrix( view.pmtx )

			gl.MatrixMode(gl.MODELVIEW)
			gl.LoadIdentity()

			return view
		end

-- convert rawx,rawy to x,y in msg		
		view.msg=function(msg)

			if msg.xraw and msg.yraw then	-- we need to fix raw x,y mouse numbers
			
				msg.x=( view.vx * ( (msg.xraw-(view.hx*0.5+view.px)) * view.sx ) / view.hx ) + view.vx*0.5
				msg.y=( view.vy * ( (msg.yraw-(view.hy*0.5+view.py)) * view.sy ) / view.hy ) + view.vy*0.5

				return true
			end

		end
		
		view.update()
		
		return view
	end

	return views
end
