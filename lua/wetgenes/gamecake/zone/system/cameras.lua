--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local deepcopy=require("wetgenes"):export("deepcopy")

local wstr=require("wetgenes.string")
local ls=function(...)print(wstr.dump(...))end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc


	local gui=oven.rebake(oven.modname..".gui")


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

	if camera.up then -- key/mouse input
		local up=camera.up
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
