--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

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

	cameras.mouse={dx=0,dy=0,dz=10,ox=0,oy=0,sx=0,sy=0,action=0}

	return cameras
end



B.cameras.draw_head=function(cameras)

	gl.state.push(gl.state_defaults)
	gl.state.set({
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.TRUE,
	})

	gl.PushMatrix()

	local camera=cameras.scene.caste("camera")[1]

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


B.cameras.msg=function(cameras,msg)
	if msg.class=="mouse" then

		local m=cameras.mouse
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

			local camera=cameras.scene.get("camera")
			gui.datas.get("angle"):value( -camera.rot[2] )

		end

	end
--		print(cameras.mouse.dx,cameras.mouse.dy)
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

B.camera.setroty=function(camera,n)

	camera.base.rot[2]=n

	camera.cameras.mouse.dx=0

end

B.camera.update=function(camera)

	local oldrot=V3( camera.rot )

	local rotfix=function(n)
		local d=(n+180)/360
		d=d-math.floor(d)
		return (math.floor(d*3600)-1800)/10
	end

	if camera.focus then

		if camera.focus.focus_camera then

			camera.focus:focus_camera(camera)

		else

			camera.pos:set( camera.focus.pos )
			camera.pos[2]=camera.pos[2]-5

		end

	end

	camera.dolly=camera.base.dolly + camera.cameras.mouse.dz

	camera.rot[1]=rotfix( camera.base.rot[1]+( (camera.cameras.mouse.dy)* 180 ) )
	camera.rot[2]=rotfix( camera.base.rot[2]+( (camera.cameras.mouse.dx)*-180 ) )
	camera.rot[3]=rotfix( camera.base.rot[3] )

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
