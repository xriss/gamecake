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

	world.time=0
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
	for i,mesh  in pairs(world.meshes)  do mesh:destroy() end

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

	fixedstep=fixedstep or world.fixedstep
	maxsteps=maxsteps or world.maxsteps
	seconds=seconds or world.fixedstep

	core.world_step( world[0] , seconds , maxsteps , fixedstep )
	
	-- snap slightly squiffy step times
	local steps=math.ceil(world.time/fixedstep)
	local add=math.ceil(seconds/fixedstep)
	if add>maxsteps then add=maxsteps end
	if add<1 then add=1 end
	world.time=(steps+add)*fixedstep

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
	if test.hit and test.hit.body_ptr then
		test.hit.body=world.bodies[test.hit.body_ptr]
	end
	return test
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.contacts

	local contacts,csiz=world:contacts()
	local contacts,csiz=world:contacts(min_dist)
	local contacts,csiz=world:contacts(min_dist,min_impulse)

Fetch all contacts in the world that are the same or closer than 
min_dist which defaults to 0 and hit the same or harder than 
min_impulse which also defaults to 0. This helps filter out 
uninteresting collisions before we process them.

csiz, the second return allows us to put more info into each chunk in 
the future, it will probably be 10 but may grow if it turns out that 
more contact info for each point would help.

This returns a list of contacts. Each contact is an array that 
consists of.

	a_body,
	b_body,

and then 1 or more chunks of csiz (which is currently 10) numbers 
representing

	ax,ay,az,	-- world position on a_body
	bx,by,bz,	-- world position on b_body
	nx,ny,nz,	-- world normal on b_body
	impulse,	-- impulse applied by collision

So you can find the two bodys in contact[1] and contact[2] but are then 
expected to loop over the rest of the array as chunks of csiz like so.

	for idx=3,#contact,csiz do
		local pos_a={ contact[idx+0] , contact[idx+1] , contact[idx+2] }
		local pos_b={ contact[idx+3] , contact[idx+4] , contact[idx+5] }
		local nrm_b={ contact[idx+6] , contact[idx+7] , contact[idx+8] }
		local impulse=contact[idx+9]
		...
	end
	
This is intended to be processed and interesting collisions handled or 
saved for later.
	
]]
bullet.world_functions.contacts=function(world,min_dist,min_impulse)
	local collisions,csiz=core.world_contacts( world[0] , min_dist or 0 , min_impulse or 0 )
	for i,collision in ipairs(collisions) do
		collision[1]=world.bodies[ collision[1] ]
		collision[2]=world.bodies[ collision[2] ]
	end
	return collisions,csiz
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
--	if mesh.name then world:set(mesh.name) end
	local ptr=mesh[0]
	core.mesh_destroy( mesh[0] )
	world.meshes[ ptr ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape

	shape = world:shape()

Create a shape.

]]
bullet.world_functions.shape=function(world,name,a,...)
	local shape={}
	setmetatable(shape,bullet.shape_metatable)

	if name=="mesh" then shape.mesh=a a=a[0] end
	if name=="compound" then -- multiple sub shapes?
		shape.shapes={}
		for i,v in ipairs(a) do
			shape.shapes[i]=v[1] -- remember children for later freeing
		end
	end

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
--	if shape.name then world:set(shape.name) end
	local ptr=core.shape_ptr(shape[0])
	core.shape_destroy( shape[0] )
	world.shapes[ ptr ]=nil
	if shape.mesh then
		shape.mesh:destroy()
	end
	for i,v in ipairs(shape.shapes or {}) do -- sub shapes should also be destroyed
		v:destroy() 
	end
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
	body = world:body({name="rigid",shape=shape,mass=mass.pos=V3(0)})

Create a body.

]]
bullet.world_functions.body=function(world,name,shape,mass,x,y,z,cgroup,cmask)

	local opts={}
	if type(name)=="table" then
		opts=name
		name=nil
	end
	
	opts.name=opts.name or name
	opts.shape=opts.shape or shape
	opts.mass=opts.mass or mass
	opts.pos=opts.pos or {x or 0,y or 0,z or 0}
	opts.cgroup=opts.cgroup or cgroup or -1
	opts.cmask=opts.cmask or cmask or -1

	local body={}
	setmetatable(body,bullet.body_metatable)
	body[0]=core.body_create(opts.name,opts.shape[0],opts.mass,opts.pos[1],opts.pos[2],opts.pos[3])
	body.world=world
	body.name=name -- probably rigid or ghost
	body.shape=shape
	body.mass=opts.mass -- remember
	body.cgroup=opts.cgroup
	body.cmask=opts.cmask
	
	world.bodies[ core.body_ptr(body[0]) ]=body

	core.world_add_body( world[0] , body.name , body[0] , body.cgroup , body.cmask )

	return body
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.destroy

	body:destroy()

Destroy body.

]]
bullet.body_functions.destroy=function(body)
	local world=body.world
	core.world_remove_body(world[0],body[0])
--	if body.name then world:set(body.name) end
	local ptr=core.body_ptr(body[0])
	core.body_destroy( body[0] )
	world.bodies[ ptr ]=nil
	body.shape:destroy()
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


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.overlaps

	body:overlaps()

Get list of bodys that overlap with a ghost body object.

]]
bullet.body_functions.overlaps=function(body)

	local world=body.world

	local overlaps=core.body_overlaps( body[0] )

	for i=1,#overlaps do
		overlaps[i]=world.bodies[ overlaps[i] ]
	end

	return overlaps
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.support

	x,y,z=body:support(nx,ny,nz)

Get world location of support point in given direction.

EG the world location that is touching the floor when the 
direction is up.

]]
bullet.body_functions.support=function(body,nx,ny,nz)
	return core.body_support( body[0] , nx , ny , nz )
end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.force

	body:force(fx,fy,fz)
	body:force(fx,fy,fz,lx,ly,lz)

Apply force fx,fy,fz at world relative location (subtract origin of 
object) lx,ly,lz which will default to 0,0,0 if not given.

]]
bullet.body_functions.force=function(body,fx,fy,fz,lx,ly,lz)
	return core.body_force( body[0] , fx,fy,fz , lx,ly,lz )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.impulse

	body:impulse(fx,fy,fz)
	body:impulse(fx,fy,fz,lx,ly,lz)

Apply impulse fx,fy,fz at world relative location (subtract origin of 
object) lx,ly,lz which will default to 0,0,0 if not given.

]]
bullet.body_functions.impulse=function(body,fx,fy,fz,lx,ly,lz)
	return core.body_impulse( body[0] , fx,fy,fz , lx,ly,lz )
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.body.change_shape

	body:change_shape(shape)
	body:change_shape(shape,mass)

Change the shape assoctiated with this body and optionally change the 
mass.

]]
bullet.body_functions.change_shape=function(body,shape,mass)
	local world=body.world
	body.mass=mass or body.mass

	if world then
		core.world_remove_body( world[0] , body[0] )
	end
	
	local old_ptr=core.body_shape( body[0] , shape[0] , body.mass )
	body.shape=shape

	if world then
		core.world_add_body( world[0] , body.name , body[0] , body.cgroup , body.cmask )
		return world.shapes[ old_ptr ]
	end

end

--[[#lua.wetgenes.bullet.world.status

	print( world:status() )

Return a debug string about allocated objects in this world.

]]
bullet.world_functions.status=function(world)
	local lines={}
	local count={}
	for i,body  in pairs(world.bodies)  do count.bodies=(count.bodies or 0) +1 end
	for i,shape in pairs(world.shapes)  do count.shapes=(count.shapes or 0) +1 end
	for i,mesh  in pairs(world.meshes)  do count.meshes=(count.meshes or 0) +1 end
	for name,count in pairs(count) do
		lines[#lines+1]=(name).." : "..count
	end
	return table.concat(lines,"\n")
end

return bullet

