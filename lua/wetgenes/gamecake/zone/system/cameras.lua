--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc


	local gui=oven.rebake(oven.modname..".gui")

	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")


B.cameras={}
B.cameras_metatable={__index=B.cameras}

B.camera={}
B.camera_metatable={__index=B.camera}

local gl=oven.gl


B.system=function(cameras)

	setmetatable(cameras,B.cameras_metatable)

	cameras.caste="camera"

	return cameras
end


B.cameras.draw_head=function(cameras)

	gl.state.push(gl.state_defaults)
	gl.state.set({
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.TRUE,
	})

	gl.PushMatrix()

	local camera=cameras.scene.get("camera")

	cameras.active=camera

	if camera then

--		print( M4(gl.matrix(gl.MODELVIEW)) )

		gl.MultMatrix(camera.inv)

		gl.uniforms.camera=function(u)
			gl.UniformMatrix4f( u , camera.mtx )
		end

	end




end

B.cameras.draw_tail=function(cameras)

	gl.PopMatrix()

	gl.state.pop()

end


B.cameras.create=function(cameras,boot)
	local camera={}
	camera.boot=boot
	camera.caste=cameras.caste
	camera.cameras=cameras
	setmetatable(camera,B.camera_metatable)

	camera.up=1

	camera.mode="orbit"

	camera.rot=V3( boot.rot or {20,0,0} )
	camera.pos=V3( boot.pos or {0,-5,0} )

	camera.dolly=5 -- minimum dolly value

	camera.floor=0 -- keep camera above this

	camera.base={ rot=deepcopy(camera.rot) ,  pos=deepcopy(camera.pos) , dolly=camera.dolly }

	camera.mtx=M4()
	camera.inv=M4()

	return camera
end

B.camera.update=function(camera)
	
	if camera.focus then

		if camera.focus.focus_camera then

			camera.focus:focus_camera(camera)

		else

			camera.pos:set( camera.focus.pos )

		end

	end

	local up=camera.up and srecaps.ups(camera.up)
	if up then
		
		display(wstr.dump(up.axis()))
--		display(wstr.dump(up.button()))
		
		if camera.mode=="orbit" then
		
			camera.orbit=camera.orbit or {
				mode="none",
				mx=0,my=0,mz=20,
				mz_min=4,mz_max=40,
			}
			local orbit=camera.orbit
						
			local minzone=4095
			local maxzone=32767
			local fixaxis=function(n)
				local fix=function(n)
					local n=(n-minzone)/(maxzone-minzone)
					if n<0 then return 0 end
					if n>1 then return 1 end
					return n
				end
				n=n or 0
				if n < 0 then
					return -fix(-n)
				else
					return fix(n)
				end
			end
			
			local sensitivity=2


			local mouse_button=up.button("mouse_right") or up.button("mouse_middle") or up.button("mouse_left") or false
			local lx=fixaxis( up.axis("lx") )
			local ly=fixaxis( up.axis("ly") )
			local rx=fixaxis( up.axis("rx") )
			local ry=fixaxis( up.axis("ry") )
			local mx=up.axis("mx") or 0
			local my=up.axis("my") or 0
			local mz=up.axis("mz") or 0 ; if mz>32768 then mz=mz-65536 end

			local rotfix=function(n)
				local d=(n+180)/360
				d=d-math.floor(d)
--				return (math.floor(d*3600)-1800)/10
				return (d*360)-180
			end
			
			if ( lx*lx + ly*ly ) > 0.25*0.25 then -- slowly rotate camera in direction of movement
				local r = 180*math.atan2(lx,-ly)/math.pi
				if r> 90 then r= 180-r end
				if r<-90 then r=-180-r end
--				print(math.floor(r))
				orbit.mx=rotfix( orbit.mx - (r/256) )
			end
			

			orbit.my=rotfix( orbit.my + (ry*sensitivity) )
			orbit.mx=rotfix( orbit.mx - (rx*sensitivity) )


			local do_orbit=mouse_button
			local do_zoom=true

			if gui.master.hidden then -- no gui displayed

				do_orbit=true

			elseif gui.master.over ~=  gui.master then -- ignore clicks on gui

				do_orbit=false
				do_zoom=false

				orbit.lx=mx -- ignore movement
				orbit.ly=my
				orbit.lz=mz

			end
			
			if do_zoom and orbit.lz then -- perform zoom

				orbit.mz=orbit.mz-((mz-orbit.lz)*1.0) -- need to reverse the mousewheel

				if orbit.mz < orbit.mz_min then orbit.mz=orbit.mz_min end -- limits
				if orbit.mz > orbit.mz_max then orbit.mz=orbit.mz_max end

			end
			
			if do_orbit and orbit.lx and orbit.ly then -- perform orbits
			
				orbit.my=rotfix( orbit.my+(((my-orbit.ly)/1024)* 180 ) )
				orbit.mx=rotfix( orbit.mx+(((mx-orbit.lx)/1024)*-180 ) )

			end
			

--[[
			orbit.dz=-(mz-orbit.oz) -- need to reverse the mousewheel
			if orbit.dz < orbit.mz_min then orbit.oz=mz+orbit.mz_min orbit.dz=orbit.mz_min end -- limits
			if orbit.dz > orbit.mz_max then orbit.oz=mz+orbit.mz_max orbit.dz=orbit.mz_max end

			if orbit.mode=="mouse_right" then
			
				if mouse_right then

					orbit.dx=(mx-orbit.ox)
					orbit.dy=(my-orbit.oy)
					
				else
					orbit.mode="none"
				end
			
			else			

				if mouse_right then
					orbit.mode="mouse_right"
					orbit.ox=mx-orbit.dx
					orbit.oy=my-orbit.dy
				end

			end
]]
						
			display(orbit.mode,mouse_right)
			display(orbit.rx,orbit.ry)
			display(orbit.mx,orbit.my,orbit.mz)

			camera.dolly=camera.base.dolly + orbit.mz

			camera.rot[1]=rotfix( camera.base.rot[1] + orbit.my )
			camera.rot[2]=rotfix( camera.base.rot[2] + orbit.mx )
			camera.rot[3]=rotfix( camera.base.rot[3]            )


			orbit.lx=mx -- we have applied this movement
			orbit.ly=my
			orbit.lz=mz
		end

	end

-- This is a camera so we are applying reverse transforms...
	camera.mtx:identity()
	camera.mtx:translate( camera.pos[1] , camera.pos[2] , camera.pos[3] )
	camera.mtx:rotate( camera.rot[3] ,  0, 0, 1 )
	camera.mtx:rotate( camera.rot[2] ,  0, 1, 0 )
	camera.mtx:rotate( camera.rot[1] ,  1, 0, 0 )
	camera.mtx:translate( 0,0, camera.dolly )

	if camera.mtx[14] > camera.floor then camera.mtx[14]=camera.floor end -- keep above ground

-- build inverse camera transform to
	camera.mtx:inverse( camera.inv )

end

return B
end
