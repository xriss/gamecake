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


	local gui=oven.rebake("wetgenes.gamecake.zone.gui")
--	local ok,gui=pcall(function() return oven.rebake(oven.modname..".gui") end)

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

	gl.PushMatrix()

	local camera=cameras.scene.get("camera")

	cameras.active=camera

	if camera then

		gl.MultMatrix(camera.inv)

		gl.uniforms.camera=function(u)
			gl.UniformMatrix4f( u , camera.mtx )
		end

	end




end

B.cameras.draw_tail=function(cameras)

	gl.PopMatrix()

end


B.cameras.create=function(cameras,boot)
	local camera={}
	camera.scene=cameras.scene
	camera.boot=boot
	camera.caste=cameras.caste
	camera.cameras=cameras
	setmetatable(camera,B.camera_metatable)

	camera.up=1

	camera.mode="orbit"

	camera.rot=V3( boot.rot or {0,0,0} )
	camera.pos=V3( boot.pos or {0,-5,0} )

	camera.dolly=5 -- minimum dolly value

--	camera.floor=0 -- keep camera above this

--	camera.base={ rot=deepcopy(camera.rot) ,  pos=deepcopy(camera.pos) , dolly=camera.dolly }

	camera.mtx=M4()
	camera.inv=M4()

-- world movement vectors for player in camera space
	camera.playerz=V3( { 0 , -1 , 0 } )
	camera.playery=V3( { 0 ,  0 , 1 } ) -- player xy are in stick axis space so +z is jump
	camera.playerx=V3( { 1 ,  0 , 0 } )

	camera.orbit=camera.orbit or {
		mode="none",
		mx=0,my=20,mz=20,
		my_min=-90,my_max=90,
		mz_min=8,mz_max=50,
		dolly=1,
	}
	
	camera.move_and_rotate=true


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
		
		if camera.mode=="orbit" then
		
			local orbit=camera.orbit
			
			local sensitivity=2

			local mouse_middle=up.button("mouse_middle") or false
			local mouse_button=up.button("mouse_right") or up.button("mouse_left") or false
			local lx=up.axisfixed("lx")
			local ly=up.axisfixed("ly")
			local rx=up.axisfixed("rx")
			local ry=up.axisfixed("ry")
			local r3=up.button("r3") or false
			local mx=up.axis("mx") or 0
			local my=up.axis("my") or 0
			local mz=up.axis("mz") or 0 ; if mz>32768 then mz=mz-65536 end

			local rotfix=function(n)
				local d=(n+180)/360
				d=d-math.floor(d)
				return (d*360)-180
			end

			if r3 or mouse_middle then -- hold r3 or middle_mouse in to dolly camera and allow parallel to camera movement
				orbit.mz=  orbit.mz + ( 0.25 * (ry-ly) * sensitivity )
				orbit.mx=rotfix( orbit.mx - (  (rx   ) * sensitivity ) )		-- right stick only gives no auto camera rotate
			else
				orbit.my=rotfix( orbit.my + (      ry  * sensitivity ) )
				if camera.move_and_rotate then
					orbit.mx=rotfix( orbit.mx - (  (rx+lx) * sensitivity ) )		-- left + right stick gives auto camera rotate
				else
					orbit.mx=rotfix( orbit.mx - (  (rx   ) * sensitivity ) )		-- right stick only gives no auto camera rotate
				end
			end
			
			local do_orbit=mouse_button
			local do_zoom=true

			if gui and gui.master and gui.master.hidden then -- no gui displayed

				do_orbit=true

			elseif gui and gui.master and gui.master.over then -- ignore clicks on gui

				do_orbit=false
				do_zoom=false

				orbit.lx=mx -- ignore movement
				orbit.ly=my
				orbit.lz=mz

			end
			
			if do_zoom and orbit.lz then -- perform zoom

				orbit.mz=orbit.mz-((mz-orbit.lz)*1.0) -- need to reverse the mousewheel

			end
			
			if do_orbit and orbit.lx and orbit.ly then -- perform orbits
			
				orbit.my=rotfix( orbit.my+(((my-orbit.ly)/1024)* 180 ) )
				orbit.mx=rotfix( orbit.mx+(((mx-orbit.lx)/1024)*-180 ) )

			end

			if orbit.mx_set then orbit.mx=rotfix(orbit.mx_set) end
			if orbit.my_set then orbit.my=rotfix(orbit.my_set) end
			if orbit.mz_set then orbit.mz=rotfix(orbit.mz_set) end

			if orbit.my < orbit.my_min then orbit.my=orbit.my_min end -- limits
			if orbit.my > orbit.my_max then orbit.my=orbit.my_max end

			if orbit.mz < orbit.mz_min then orbit.mz=orbit.mz_min end -- limits
			if orbit.mz > orbit.mz_max then orbit.mz=orbit.mz_max end
			
			camera.dolly=orbit.mz

			camera.rot[1]=rotfix( orbit.my )
			camera.rot[2]=rotfix( orbit.mx )
			camera.rot[3]=rotfix( 0 )


			orbit.lx=mx -- we have applied this movement
			orbit.ly=my
			orbit.lz=mz
		end

	end


-- ray cast the camera distance to control the dolly
	local physics=camera.scene.systems.physics
	if physics then
		local q=Q4()
		q:rotate( camera.rot[1] ,  {1,0,0} )
		q:rotate( camera.rot[2] ,  {0,1,0} )
		q:rotate( camera.rot[3] ,  {0,0,1} )
		local d=V3(0,0,1):product(q)

		local test=physics.world:ray_test({
			ray={
				camera.pos+V3(0,0,0),
				camera.pos+(d*camera.dolly)+V3(0,0,0),
			},
		})
		if test.hit then
			camera.orbit.dolly = ( camera.orbit.dolly*3 + test.hit.fraction ) /4
		else
			camera.orbit.dolly = ( camera.orbit.dolly*3 + 1 ) /4
		end
	end




-- This is a camera so we are applying reverse transforms...
	camera.mtx:identity()
	camera.mtx:translate( camera.pos[1] , camera.pos[2] , camera.pos[3] )
	camera.mtx:rotate( camera.rot[3] ,  0, 0, 1 )
	camera.mtx:rotate( camera.rot[2] ,  0, 1, 0 )
	camera.mtx:rotate( camera.rot[1] ,  1, 0, 0 )
	camera.mtx:translate( 0,0, 0.0 + camera.dolly*camera.orbit.dolly )

--	if camera.floor then
--		if camera.mtx[14] > camera.floor then camera.mtx[14]=camera.floor end -- keep above ground
--	end
	
	local c=math.cos(math.pi*camera.rot[2]/180)
	local s=math.sin(math.pi*camera.rot[2]/180)
	camera.playerz=V3( { 0 , -1 , 0 } )
	camera.playery=V3( { s ,  0 , c } ) -- player xy are in stick axis space so +z is jump
	camera.playerx=V3( { c ,  0 ,-s } )

-- build inverse camera transform to
	camera.mtx:inverse( camera.inv )

end

return B
end
