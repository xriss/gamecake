

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.bullet


	local bullet=require("wetgenes.bullet")

We use bullet as the local name of this library.

A lua binding to the [Bullet 
Physics](https://github.com/bulletphysics/bullet3) library



## lua.wetgenes.bullet.body.cgroup


	body:cgroup(bits)
	bits=body:cgroup()

Set or get body cgroup bits. You have 31 bits, so use 0x7fffffff to set 
all. These provide simple yes/no collision control between bodies.

	cgroup bits are all the groups this body belongs to.
	cmask bits are all the groups this body colides with.



## lua.wetgenes.bullet.body.change_shape


	body:change_shape(shape)
	body:change_shape(shape,mass)

Change the shape assoctiated with this body and optionally change the 
mass.



## lua.wetgenes.bullet.body.cmask


	body:cmask(bits)
	bits=body:cmask()
	body:cmask(0x7fffffff)

Set or get body cmask bits. You have 31 bits, so use 0x7fffffff to set 
all. This body will only colide with another body if the other bodys 
cgroup has a bit set that is also set in our cmask.

	cgroup bits are all the groups this body belongs to.
	cmask bits are all the groups this body colides with.



## lua.wetgenes.bullet.body.force


	body:force(fx,fy,fz)
	body:force(fx,fy,fz,lx,ly,lz)

Apply force fx,fy,fz at world relative location (subtract origin of 
object) lx,ly,lz which will default to 0,0,0 if not given.



## lua.wetgenes.bullet.body.gravity


	body:gravity(x,y,z)
	x,y,z = body:gravity()

Set or get body gravity vector. Fidling with this may be the easiest 
way for a player to move an object around, it certainly makes it easier 
to create "magnetic fields" to hover objects above the ground.



## lua.wetgenes.bullet.body.impulse


	body:impulse(fx,fy,fz)
	body:impulse(fx,fy,fz,lx,ly,lz)

Apply impulse fx,fy,fz at world relative location (subtract origin of 
object) lx,ly,lz which will default to 0,0,0 if not given.



## lua.wetgenes.bullet.body.overlaps


	body:overlaps()

Get list of bodys that overlap with a ghost body object.



## lua.wetgenes.bullet.body.support


	x,y,z=body:support(nx,ny,nz)

Get world location of support point in given direction.

EG the world location that is touching the floor when the 
direction is up.



## lua.wetgenes.bullet.world


	world=bullet.world()

Create the world you will be simulating physics in.



## lua.wetgenes.bullet.world.body


	body = world:body("rigid",shape,mass,x,y,z,cgroup,cmask)
	body = world:body({name="rigid",shape=shape,mass=mass.pos=V3(0)})

Create a body.



## lua.wetgenes.bullet.world.body.active


	b = body:active( true )
	b = body:active( false )
	b = body:active()

get/set the active state of an object



## lua.wetgenes.bullet.world.body.angular_factor


	x,y,z = body:angular_factor( x , y , z )
	x,y,z = body:angular_factor( r )
	r = ( body:angular_factor( r ) )

get/set the angular factor of an object (which disables rotation when zero)



## lua.wetgenes.bullet.world.body.angular_velocity


	x,y,z = body:angular_velocity( x,y,z )
	x,y,z = body:angular_velocity()

get/set the body angular velocity



## lua.wetgenes.bullet.world.body.ccd


	r,t = body:ccd( radius,threshold )
	r,t = body:ccd()

get/set the continuos collision detection radius,threshold values



## lua.wetgenes.bullet.world.body.custom_material_callback


	b = body:custom_material_callback( b )
	b = body:custom_material_callback()

get/set the body custom_material_callback flag

When set we run a custom callback to try and smooth mesh collisions.



## lua.wetgenes.bullet.world.body.damping


	l,a = body:damping( linear , angular )
	l,a = body:damping()

get/set the body damping



## lua.wetgenes.bullet.world.body.destroy


	body:destroy()

Destroy body.



## lua.wetgenes.bullet.world.body.factor


	x,y,z = body:factor( x , y , z )
	x,y,z = body:factor( r )
	r = ( body:factor( r ) )

get/set the linear factor of an object (which disables movement when zero)



## lua.wetgenes.bullet.world.body.friction


	l,a,s = body:friction( linear , angular , spinning )
	l,a,s = body:friction()

get/set the body friction



## lua.wetgenes.bullet.world.body.restitution


	r = body:transform( r )
	r = body:transform()

get/set the body restitution



## lua.wetgenes.bullet.world.body.transform


	px,py,pz,qx,qy,qz,qw = body:transform()
	px,py,pz,qx,qy,qz,qw = body:transform(px,py,pz)
	px,py,pz,qx,qy,qz,qw = body:transform(px,py,pz,qx,qy,qz,qw)

get/set the body transform. Position and Rotation Quaternion.



## lua.wetgenes.bullet.world.body.velocity


	x,y,z = body:velocity( x,y,z )
	x,y,z = body:velocity()

get/set the body velocity



## lua.wetgenes.bullet.world.contacts


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
	


## lua.wetgenes.bullet.world.destroy


	world:destroy()

Destroy the world and all associated data.



## lua.wetgenes.bullet.world.get


	world:get(name)
	
Get a named mesh/body/shape



## lua.wetgenes.bullet.world.gravity


	world:gravity(x,y,z)
	
	x,y,z = world:gravity()

Set or get world gravity vector. Recommended gravity is 0,-10,0



## lua.wetgenes.bullet.world.mesh


	mesh = world:mesh()

Create a mesh.



## lua.wetgenes.bullet.world.mesh.destroy


	mesh:destroy()

Destroy mesh.



## lua.wetgenes.bullet.world.ray_test


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



## lua.wetgenes.bullet.world.set


	world:set(name,it)
	
Set a named mesh/body/shape



## lua.wetgenes.bullet.world.shape


	shape = world:shape()

Create a shape.



## lua.wetgenes.bullet.world.shape.destroy


	shape:destroy()

Destroy shape.



## lua.wetgenes.bullet.world.shape.margin


	r = body:margin( radius )

get/set the shapes margin size



## lua.wetgenes.bullet.world.status


	print( world:status() )

Return a debug string about allocated objects in this world.



## lua.wetgenes.bullet.world.step


	world:step(seconds,maxsteps,fixedstep)

	world:step(seconds,0,seconds)

world.maxsteps and world.fixedstep will be used as defaults if the second and
third values are not provided.

Move the physics forward in time by the given amount in seconds.

maxsteps is maximum amount of steps to take during this call and defaults to 1.

fixedstep is how many seconds to step forward at a time for stable simulation
and defaults to 1/60

To force a step forward of a given amount of time use a maxsteps of 0.

