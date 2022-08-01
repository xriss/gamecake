--[[--------------------------------------------------------------------

(C) Kriss@XIXs.com 2021 and released under the MIT license.

See https://github.com/xriss/gamecake for full notice.

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")

--[[#lua.wetgenes.bullet

	local bullet=require("wetgenes.bullet")

We use bullet as the local name of this library.

A lua binding to the [Bullet 
Physics](https://github.com/bulletphysics/bullet3) library

]]

local bullet={}

local core=require("wetgenes.bullet.core")
bullet.core=core

-- meta methods bound to the various objects

bullet.world_functions={is="world"}
bullet.world_metatable={__index=bullet.world_functions}

bullet.mesh_functions={is="mesh"}
bullet.mesh_metatable={__index=bullet.mesh_functions}

bullet.shape_functions={is="shape"}
bullet.shape_metatable={__index=bullet.shape_functions}

bullet.body_functions={is="body"}
bullet.body_metatable={__index=bullet.body_functions}

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world

	world=bullet.world()

Create the world you will be simulating physics in.

]]
bullet.world=function(...)
	local world={}
	world.meshes={}
	world.shapes={}
	world.bodies={}
	world.names={}
	setmetatable(world,bullet.world_metatable)
	world[0]=core.world_create(...)

	world.maxsteps=1
	world.fixedstep=1/60
	
--	core.world_register(world[0],world)
	
	return world
end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.destroy

	world:destroy()

Destroy the world and all associated data.

]]
bullet.world_functions.destroy=function(world)

	for i,body  in pairs(world.bodys)   do body:destroy() end
	for i,shape in pairs(world.shapes)  do shape:destroy() end

	core.world_destroy( world[0] )

end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.get

	world:get(name)
	
Get a named mesh/body/shape

]]
bullet.world_functions.get=function(world,name)

	return world.names[name]

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.set

	world:set(name,it)
	
Set a named mesh/body/shape

]]
bullet.world_functions.set=function(world,name,it)

	if it then
		it.name=name
		world.names[name]=it
		return it
	else
		world.names[name]=nil
	end

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.gravity

	world:gravity(x,y,z)
	
	x,y,z = world:gravity()

Set or get world gravity vector. Recommended gravity is 0,-10,0

]]
bullet.world_functions.gravity=function(world,vx,vy,vz)

	local rx,ry,rz = core.world_gravity( world[0] , vx , vy , vz )

	return rx,ry,rz

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.step

	world:step(seconds,maxsteps,fixedstep)

	world:step(seconds,0,seconds)

world.maxsteps and world.fixedstep will be used as defaults if the second and
third values are not provided.

Move the physics forward in time by the given amount in seconds.

maxsteps is maximum amount of steps to take during this call and defaults to 1.

fixedstep is how many seconds to step forward at a time for stable simulation
and defaults to 1/60

To force a step forward of a given amount of time use a maxsteps of 0.


]]
bullet.world_functions.step=function(world,seconds,maxsteps,fixedstep)

	core.world_step( world[0] , seconds , maxsteps or world.maxsteps , fixedstep or world.fixedstep )

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.ray_test

	local test={
		mode="closest",
		ray={{0,0,0},{1,1,1}},
	}
	test=world:ray_test(test)
	
	if world:ray_test(opts).hit then ... end

Perform a ray test between the two ray vectors provided in the test table. 
fills in hit={...} if we hit something with details of the hit.

Always returns the test table that was passed in, with modifications that 
provide the result. Be carefull about reusing this table it is safest to create 
a new one for each ray_test call.

]]
bullet.world_functions.ray_test=function(world,test)

	core.world_ray_test( world[0] , test )

	return test
end



------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.mesh

	mesh = world:mesh()

Create a mesh.

]]
bullet.world_functions.mesh=function(world,cp,cv)
	local mesh={}
	setmetatable(mesh,bullet.mesh_metatable)

	mesh.sp=pack.save_array(cp,"s32")
	mesh.sv=pack.save_array(cv,"f64")

	mesh[0]=core.mesh_create( #cp/3,mesh.sp,4*3 , #cv/3,mesh.sv,8*3 )
	mesh.world=world

	world.meshes[ mesh[0] ]=mesh

	return mesh
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.mesh.destroy

	mesh:destroy()

Destroy mesh.

]]
bullet.mesh_functions.destroy=function(mesh)
	local world=mesh.world
	if mesh.name then world:set(mesh.name) end
	core.mesh_destroy( mesh[0] )
	world.meshes[ mesh[0] ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape

	shape = world:shape()

Create a shape.

]]
bullet.world_functions.shape=function(world,name,a,...)
	local shape={}
	setmetatable(shape,bullet.shape_metatable)

	if name=="mesh" then a=a[0] end

	shape[0]=core.shape_create(name,a,...)
	shape.world=world
	
	world.shapes[ core.shape_ptr(shape[0]) ]=shape

	return shape
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape.destroy

	shape:destroy()

Destroy shape.

]]
bullet.shape_functions.destroy=function(shape)
	local world=shape.world
	if shape.name then world:set(shape.name) end
	core.shape_destroy( shape[0] )
	world.shapes[ core.shape_ptr(shape[0]) ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape.margin

	r = body:margin( radius )

get/set the shapes margin size

]]
bullet.shape_functions.margin=function(shape,n)
	return core.shape_margin( shape[0] , n )
end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body

	body = world:body("rigid",shape,mass,x,y,z,cgroup,cmask)

Create a body.

]]
bullet.world_functions.body=function(world,name,shape,mass,x,y,z,cgroup,cmask)
	local body={}
	setmetatable(body,bullet.body_metatable)
	body[0]=core.body_create(name,shape[0],mass,x,y,z)
	body.world=world
	
	world.bodies[ core.body_ptr(body[0]) ]=body

	core.world_add_body( world[0] , body[0] , cgroup , cmask or -1 )

	return body
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.destroy

	body:destroy()

Destroy body.

]]
bullet.body_functions.destroy=function(body)
	local world=body.world
	if body.name then world:set(body.name) end
	core.body_destroy( body[0] )
	world.bodies[ core.body_ptr(body[0]) ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.transform

	px,py,pz,qx,qy,qz,qw = body:transform()
	px,py,pz,qx,qy,qz,qw = body:transform(px,py,pz)
	px,py,pz,qx,qy,qz,qw = body:transform(px,py,pz,qx,qy,qz,qw)

get/set the body transform. Position and Rotation Quaternion.

]]
bullet.body_functions.transform=function(body,...)
	return core.body_transform( body[0] , ... )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.restitution

	r = body:transform( r )
	r = body:transform()

get/set the body restitution

]]
bullet.body_functions.restitution=function(body,r)
	return core.body_restitution( body[0] , r )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.friction

	l,a,s = body:friction( linear , angular , spinning )
	l,a,s = body:friction()

get/set the body friction

]]
bullet.body_functions.friction=function(body,l,a,s)
	return core.body_friction( body[0] , l , a , s or a)
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.damping

	l,a = body:damping( linear , angular )
	l,a = body:damping()

get/set the body damping

]]
bullet.body_functions.damping=function(body,l,a)
	return core.body_damping( body[0] , l , a  )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.velocity

	x,y,z = body:velocity( x,y,z )
	x,y,z = body:velocity()

get/set the body velocity

]]
bullet.body_functions.velocity=function(body,x,y,z)
	return core.body_velocity( body[0] , x,y,z )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.ccd

	r,t = body:ccd( radius,threshold )
	r,t = body:ccd()

get/set the continuos collision detection radius,threshold values

]]
bullet.body_functions.ccd=function(body,radius,threshold)
	return core.body_ccd( body[0] , radius,threshold )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.active

	b = body:active( true )
	b = body:active( false )
	b = body:active()

get/set the active state of an object

]]
bullet.body_functions.active=function(body,b)
	return core.body_active( body[0] , b )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.factor

	x,y,z = body:factor( x , y , z )
	x,y,z = body:factor( r )
	r = ( body:factor( r ) )

get/set the linear factor of an object (which disables movement when zero)

]]
bullet.body_functions.factor=function(body,x,y,z)
	return core.body_factor( body[0] , x , y or x , z or x)
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.angular_velocity

	x,y,z = body:angular_velocity( x,y,z )
	x,y,z = body:angular_velocity()

get/set the body angular velocity

]]
bullet.body_functions.angular_velocity=function(body,x,y,z)
	return core.body_angular_velocity( body[0] , x,y,z )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.angular_factor

	x,y,z = body:angular_factor( x , y , z )
	x,y,z = body:angular_factor( r )
	r = ( body:angular_factor( r ) )

get/set the angular factor of an object (which disables rotation when zero)

]]
bullet.body_functions.angular_factor=function(body,x,y,z)
	return core.body_angular_factor( body[0] , x , y or x , z or x)
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.custom_material_callback

	b = body:custom_material_callback( b )
	b = body:custom_material_callback()

get/set the body custom_material_callback flag

When set we run a custom callback to try and smooth mesh collisions.

]]
bullet.body_functions.custom_material_callback=function(body,b)
	return core.body_custom_material_callback( body[0] , b )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.gravity

	body:gravity(x,y,z)
	x,y,z = body:gravity()

Set or get body gravity vector. Fidling with this may be the easiest 
way for a player to move an object around, it certainly makes it easier 
to create "magnetic fields" to hover objects above the ground.

]]
bullet.body_functions.gravity=function(body,vx,vy,vz)

	local rx,ry,rz = core.body_gravity( body[0] , vx , vy , vz )

	return rx,ry,rz

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.cgroup

	body:cgroup(bits)
	bits=body:cgroup()

Set or get body cgroup bits. You have 31 bits, so use 0x7fffffff to set 
all. These provide simple yes/no collision control between bodies.

	cgroup bits are all the groups this body belongs to.
	cmask bits are all the groups this body colides with.

]]
bullet.body_functions.cgroup=function(body,bits)

	return core.body_cgroup( body[0] , bits )

end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.cmask

	body:cmask(bits)
	bits=body:cmask()
	body:cmask(0x7fffffff)

Set or get body cmask bits. You have 31 bits, so use 0x7fffffff to set 
all. This body will only colide with another body if the other bodys 
cgroup has a bit set that is also set in our cmask.

	cgroup bits are all the groups this body belongs to.
	cmask bits are all the groups this body colides with.

]]
bullet.body_functions.cmask=function(body,bits)

	return core.body_cmask( body[0] , bits )

end

return bullet

