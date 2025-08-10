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
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local cameras=M

cameras.caste="camera"

cameras.uidmap={
	focus=1,
	render=2,
	sky=3,
	length=3,
}

cameras.values={
	mode="orbit",
	pos=V3( 0,0,0 ),
	focus=V3(0,0,0),
	direction=0,
	tilt=10,
	dolly=10,
	dolly_part=1,
}

cameras.types={
	direction="twrap",
	pos="tween",
	tilt="tween",
	dolly="tween",
	dolly_part="tween",
}

cameras.twraps={
	direction=360,
}

-- methods added to system
cameras.system={}
-- methods added to each item
cameras.item={}

cameras.item.set_values=function(camera)

	camera:set_auto_values()

end

cameras.item.get_values=function(camera)

	camera:get_auto_values()
--	camera.direction=camera:get("direction") -- 360

	if not camera.focus_pos then camera.focus_pos=V3() end
	local focus=camera:depend("focus") -- probably focused on player
	if focus then
		camera.focus_pos:set( focus:get("pos") )
		if focus.focus_camera then focus:focus_camera(camera) end
	end

-- This is a camera so we are applying reverse transforms...
	camera.mtx=M4()
	camera.mtx:translate( camera.focus_pos[1] , camera.focus_pos[2] , camera.focus_pos[3] )
	camera.mtx:rotate( camera.direction ,  0, 1, 0 )
	camera.mtx:rotate( camera.tilt      ,  1, 0, 0 )
	camera.mtx:translate( 0,0, 0.0 + camera.dolly*camera.dolly_part )

	camera.pos=camera.mtx:get_translation_v3()
--	camera:set("pos",camera.pos)


	local c=math.cos(math.pi*camera.direction/180)
	local s=math.sin(math.pi*camera.direction/180)
	camera.playerz=V3( { 0 , -1 , 0 } )
	camera.playery=V3( { s ,  0 , c } ) -- player xy are in stick axis space so +z is jump
	camera.playerx=V3( { c ,  0 ,-s } )

-- build inverse camera transform to
	camera.inv=M4()
	camera.mtx:inverse( camera.inv )

end


cameras.item.set_active=function(camera)
	local sys=camera.sys
	local gl=camera.gl

	camera:get_values()

	if camera then -- can be called with nil to unset

		gl.uniforms.camera=function(u)
			gl.UniformMatrix4f( u , camera.mtx ) -- so we can undo the camera from the view
		end

		gl.uniforms.incamera=function(u)
			gl.UniformMatrix4f( u , camera.inv ) -- apply this one in a shader
		end

	else

		gl.uniforms.camera=nil
		gl.uniforms.incamera=nil

	end


end

cameras.item.setup=function(camera)

	if not camera.sys.singular then camera.sys.singular=kinetic end

	camera:get_values()

	camera.tilt_min=-90
	camera.tilt_max=90
	camera.dolly_min=8
	camera.dolly_max=50

	camera.move_and_rotate=true

	return camera
end

cameras.item.update=function(camera)

	camera:get_values()

	local player=camera:depend("focus") -- probably focused on player
	local up=player and player.get_up and player:get_up()
	if up then

		if camera.mode=="orbit" then

			local orbit=camera.orbit

			local sensitivity=256*camera.scene.ticks.length

			local mouse_middle=up:get("mouse_middle") or false
			local mouse_button=( up:get("mouse_right") or up:get("mouse_left") ) or false
			local lx=up:axis("lx") or 0
			local ly=up:axis("ly") or 0
			local rx=up:axis("rx") or 0
			local ry=up:axis("ry") or 0
			local r3=up:get("r3") or false
			local mx=up:get("mx") or 0
			local my=up:get("my") or 0
			local mz=up:get("mz") or 0

			if r3 or mouse_middle then -- hold r3 or middle_mouse in to dolly camera and allow parallel to camera movement
				camera.dolly=  camera.dolly + ( 0.25 * (ry-ly) * sensitivity )
				camera.direction=( camera.direction - (  (rx   ) * sensitivity ) )		-- right stick only gives no auto camera rotate
			else
				camera.tilt=( camera.tilt + (      ry  * sensitivity ) )
				if camera.move_and_rotate then
					camera.direction=( camera.direction - (  ((rx*4+lx)/5) * sensitivity ) )		-- left + right stick gives auto camera rotate
				else
					camera.direction=( camera.direction - (  (rx   ) * sensitivity ) )		-- right stick only gives no auto camera rotate
				end
			end

			local do_orbit=mouse_button
			local do_zoom=true

			do_orbit=true

			if do_zoom then -- perform zoom

				camera.dolly=camera.dolly-((mz)*1.0) -- need to reverse the mousewheel

			end

			if do_orbit then -- perform orbits

				camera.tilt=camera.tilt+(((my)/1024)* 180 )
				camera.direction=( camera.direction+(((mx)/1024)*-180 ) )

			end

			-- wrap direction
			if camera.direction<0 then
				camera.direction=360-((-camera.direction)%360)
			else
				camera.direction=camera.direction%360
			end

			-- clamp tilt
			if camera.tilt < camera.tilt_min then camera.tilt=camera.tilt_min end -- limits
			if camera.tilt > camera.tilt_max then camera.tilt=camera.tilt_max end

			-- clamp dolly
			if camera.dolly < camera.dolly_min then camera.dolly=camera.dolly_min end -- limits
			if camera.dolly > camera.dolly_max then camera.dolly=camera.dolly_max end

		end

	end

-- ray cast the camera distance to control the dolly
--[[
	local physics=camera.scene.systems.physics
	if physics then
		local q=Q4()
		q:rotate( camera.rot[1] ,  {1,0,0} )
		q:rotate( camera.rot[2] ,  {0,1,0} )
		q:rotate( camera.rot[3] ,  {0,0,1} )
		local d=V3(0,0,1):product(q)


		local frac=1

		for yp=-3,3,3 do
			local test=physics.world:ray_test({
				ray={
					camera.focus+V3(0,0,0),
					camera.focus+(d*camera.dolly)+V3(0,0+yp,0),
				},
				cmask=0x00ff,
			})
			if test.hit then
--				test.hit.body=physics.world.bodies[test.hit.body_ptr]
				if test.hit.fraction < frac then
					frac=test.hit.fraction
				end
			end
		end
--		camera.orbit.dolly = ( camera.orbit.dolly*3 + frac ) /4
		camera.orbit.dolly = frac
	end
]]

	camera:set_values()

end

cameras.item.draw=function(camera)

end
