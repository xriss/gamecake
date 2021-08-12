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
			camera.pos[2]=camera.pos[2]-5

		end

	end

	local up=camera.up and srecaps.ups(camera.up)
	if up then
		
--		display(wstr.dump(up.axis()))
--		display(wstr.dump(up.button()))
		
		if camera.mode=="orbit" then
		
			camera.orbit=camera.orbit or {
				mode="none",
				ox=0,oy=0,oz=0,
				dx=0,dy=0,dz=0,
				mz_min=4,mz_max=40,
			}
			local orbit=camera.orbit
			
			local mouse_right=up.button("mouse_right") or up.button("mouse_middle") or up.button("mouse_left") or false
			local mx=up.axis("mx") or 0
			local my=up.axis("my") or 0
			local mz=up.axis("mz") or 0
			if mz>32768 then mz=mz-65536 end

			orbit.dz=-(mz-orbit.oz) -- need to reverse the mousewheel
			if orbit.dz < orbit.mz_min then orbit.oz=mz+orbit.mz_min orbit.dz=orbit.mz_min end -- limits
			if orbit.dz > orbit.mz_max then orbit.oz=mz+orbit.mz_max orbit.dz=orbit.mz_max end

			if orbit.mode=="mouse_right" then
			
				if mouse_right then
					orbit.dx=(mx-orbit.ox)
					orbit.dy=(my-orbit.oy)
				else
					orbit.mode="none"
--[[
					local d=gui.datas.get("camangle")
					if d.num~=0 then
						gui.datas.get("angle"):value( -camera.rot[2] )
						orbit.dx=0
					end
]]
				end
			
			else			

				if mouse_right then
					orbit.mode="mouse_right"
					orbit.ox=mx-orbit.dx
					orbit.oy=my-orbit.dy
				end

			end
			
			local rotfix=function(n)
				local d=(n+180)/360
				d=d-math.floor(d)
				return (math.floor(d*3600)-1800)/10
			end
			
			display(orbit.mode,mouse_right)
			display(orbit.dx,orbit.dy,orbit.dz)

			camera.dolly=camera.base.dolly + orbit.dz

			camera.rot[1]=rotfix( camera.base.rot[1]+( (orbit.dy/1024)* 180 ) )
			camera.rot[2]=rotfix( camera.base.rot[2]+( (orbit.dx/1024)*-180 ) )
			camera.rot[3]=rotfix( camera.base.rot[3] )


		end


--[[
	if msg.class=="mouse" then

		local m=input.mouse
		m.ox=msg.ox
		m.oy=msg.oy

		if msg.keyname=="wheel_add" then
			m.dz=m.dz-1
		elseif msg.keyname=="wheel_sub" then
			m.dz=m.dz+1
		end

		if m.dz<0 then m.dz=0 end

		if msg.action==1 then -- mouse down, remember where
			m.action=1
			m.sx=m.ox
			m.sy=m.oy
		end
		if msg.action==-1 then -- mouse up
			m.action=-1
		end

		if m.action==1 then -- mouse is held down now so adjust output value
			m.dx=m.dx+(m.ox-m.sx)
			m.dy=m.dy+(m.oy-m.sy)
			m.sx=m.ox
			m.sy=m.oy
		end

		if m.action==-1 then -- set rotation on mouse up
		
			local d=gui.datas.get("camangle")

			if d.num~=0 then
				local camera=input.scene.get("camera")
				gui.datas.get("angle"):value( -camera.rot[2] )
			end

		end

	end
--		print(cameras.mouse.dx,cameras.mouse.dy)
]]



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
