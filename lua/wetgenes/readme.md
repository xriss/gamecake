


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes


	local wetgenes=require("wetgenes")

Simple generic functions that are intended to be useful for all 
wetgenes.* modules.

Probably best to cherry pick a few functions you need and export then like so.

	local export,lookup,deepcopy=require("wetgenes"):export("export","lookup","deepcopy")



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




## lua.wetgenes.chipmunk


	local chipmunk=require("wetgenes.chipmunk")

We use chipmunk as the local name of this library.

A lua binding to the Chipmunk2D physics library [chipmunk-physics.net](https://chipmunk-physics.net/)



## lua.wetgenes.chipmunk.arbiter.ignore


	return arbiter:ignore()

Ignore this collision, from now until the shapes separate.



## lua.wetgenes.chipmunk.arbiter.points


	points=arbiter:points()
	points=arbiter:points(points)

Get and/or Set the points data for this arbiter.



## lua.wetgenes.chipmunk.arbiter.surface_velocity


	vx,vy=arbiter:surface_velocity()
	vx,vy=arbiter:surface_velocity(vx,vy)

Get and/or Set the surface velocity for this arbiter.



## lua.wetgenes.chipmunk.body


	body=chipmunk.body(mass,inertia)

Create a dynamic body, with the given mass and inertia.

You will need to add the body to a space before it exists so it is 
normally preferable to use the space:body function which will call this 
function and then automatically add the body into the space.

	body=chipmunk.body("kinematic")

Create a kinematic body, these are bodies that we can move around, by 
setting its velocity, but are not effected by collisions with other 
bodies. EG a moving platform.

	body=chipmunk.body("static")

Create a static body, mostly you can just use space.static as the 
default static body but you may create more if you wish to group your 
static shapes into multiple bodies.




## lua.wetgenes.chipmunk.body.angle


	a=body:angle()
	a=body:angle(a)

Get and/or Set the rotation angle in radians for this body.



## lua.wetgenes.chipmunk.body.angular_velocity


	a=body:angular_velocity()
	a=body:angular_velocity(a)

Get and/or Set the angular velocity in radians for this body.



## lua.wetgenes.chipmunk.body.apply_force


	body:apply_force(fx,fy,px,py)
	body:apply_force(fx,fy,px,py,"world")

Apply a force to this body at a specific point, the point can be in 
world coordinates if you include the "world" flag but defaults to local 
object coordinates.



## lua.wetgenes.chipmunk.body.apply_impulse


	body:apply_impulse(ix,iy,px,py)
	body:apply_impulse(ix,iy,px,py,"world")

Apply a force to this body at a specific point, the point can be in 
world coordinates if you include the "world" flag but defaults to local 
object coordinates.



## lua.wetgenes.chipmunk.body.center_of_gravity


	vx,vy=body:center_of_gravity()
	vx,vy=body:center_of_gravity(vx,vy)

Get and/or Set the center of gravity for this body.



## lua.wetgenes.chipmunk.body.force


	vx,vy=body:force()
	vx,vy=body:force(vx,vy)

Get and/or Set the force for this body. This is reset back to 0 after 
each step.



## lua.wetgenes.chipmunk.body.mass


	m=body:mass()
	m=body:mass(m)

Get and/or Set the mass for this body.



## lua.wetgenes.chipmunk.body.moment


	m=body:moment()
	m=body:moment(m)

Get and/or Set the moment for this body.



## lua.wetgenes.chipmunk.body.position


	vx,vy=body:position()
	vx,vy=body:position(vx,vy)

Get and/or Set the position for this body.



## lua.wetgenes.chipmunk.body.position_func


	body:position_func(position_callback)
	body:position_func()

Set or clear the position callback update function for this body.

	position_callback(body)

	body.delta_time

This callback will be called with the above values set into body, you 
can adjust these and return true to perform a normal position update 
but with these new values.

Alternatively you can update the bodys position directly and return 
false so the normal position update code will not be run.



## lua.wetgenes.chipmunk.body.shape


	shape=body:shape(form,...)

Add a new shape to this body, returns the shape for further 
modification.



## lua.wetgenes.chipmunk.body.torque


	a=body:torque()
	a=body:torque(a)

Get and/or Set the torque for this body.



## lua.wetgenes.chipmunk.body.type


	t=body:type()
	t=body:type(t)

Get and/or Set the type for this body.



## lua.wetgenes.chipmunk.body.velocity


	vx,vy=body:velocity()
	vx,vy=body:velocity(vx,vy)

Get and/or Set the velocity for this body.



## lua.wetgenes.chipmunk.body.velocity_func


	body:velocity_func(velocity_callback)
	body:velocity_func()

Set or clear the velocity callback update function for this body.

	velocity_callback(body)

	body.gravity_x
	body.gravity_y
	body.damping
	body.delta_time

This callback will be called with the above values set into body, you 
can adjust these and return true to perform a normal velocity update 
but with these new values.

IE you can choose a new gravity vector for this body which is the 
simplest change to make.

Alternatively you can update the bodys velocity directly and return 
false so the normal velocity update code will not be run.



## lua.wetgenes.chipmunk.constraint


	constraint=chipmunk.constraint(abody,bbody,form,...)

Create a constraint between two bodies.

You will need to add the constraint to a space before it has any effect 
so it is normally preferable to use the space:constraint function which 
will call this function and then automatically add the constraint into 
the space.

	constraint=chipmunk.constraint(abody,bbody,"pin_join",ax,ay,bx,by)

form of "pin_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"slide_joint",ax,ay,bx,by,fl,fh)

form of "slide_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"pivot_joint",x,y)
	constraint=chipmunk.constraint(abody,bbody,"pivot_joint",ax,ay,bx,by)

form of "pivot_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"groove_joint",ax,ay,bx,by,cx,cy)

form of "groove_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"damped_spring",ax,ay,bx,by,fl,fs,fd)

form of "damped_spring" ...

	constraint=chipmunk.constraint(abody,bbody,"damped_rotary_spring",fa,fs,fd)

form or "damped_rotary_spring" ...

	constraint=chipmunk.constraint(abody,bbody,"rotary_limit_joint",fl,fh)

form of "rotary_limit_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"ratchet_joint",fp,fr)

form of "ratchet_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"gear_joint",fp,fr)

form of "gear_joint" ...

	constraint=chipmunk.constraint(abody,bbody,"simple_motor",fr)

form of "simple_motor" ...



## lua.wetgenes.chipmunk.constraint.collide_bodies


	v=constraint:collide_bodies()
	v=constraint:collide_bodies(v)

Get and/or Set the max collide bodies flag for this constraint.



## lua.wetgenes.chipmunk.constraint.error_bias


	v=constraint:error_bias()
	v=constraint:error_bias(v)

Get and/or Set the error bias for this constraint.



## lua.wetgenes.chipmunk.constraint.impulse


	v=constraint:impulse()

Get the last impulse for this constraint.



## lua.wetgenes.chipmunk.constraint.max_bias


	v=constraint:max_bias()
	v=constraint:max_bias(v)

Get and/or Set the max bias for this constraint.



## lua.wetgenes.chipmunk.constraint.max_force


	v=constraint:max_force()
	v=constraint:max_force(v)

Get and/or Set the max force for this constraint.



## lua.wetgenes.chipmunk.shape


	shape=chipmunk.shape(body,form...)
	
Create a shape, added to the given body. Shapes are always added to a 
body but must be added to a space before they have any effect. So it is 
normally preferable to use the body:shape function which will 
automatically add the shape into the space that the body belongs to.

	shape=chipmunk.shape(space.static,form...)

Create a static shape in world space. We use space.static as the body. 

	shape=chipmunk.shape(body,"circle",radius,x,y)
	
Form of "circle" needs a radius and a centre point.

	shape=chipmunk.shape(body,"segment",ax,ay,bx,by,radius)

Form of "segment" needs two points and a radius.

	shape=chipmunk.shape(body,"poly",{x1,y1,x2,y2,...},radius)

Form of "poly" is a generic polygon defined by a table of points.

	shape=chipmunk.shape(body,"box",minx,miny,maxx,maxy,radius)

Form of "box" needs two points for opposite corners, lowest pair 
followed by highest pair and a radius. The radius should be 0 unless 
you want rounded corners



## lua.wetgenes.chipmunk.shape.bounding_box


	min_x,min_y,max_x,max_y=shape:bounding_box()

Get the current bounding box for this shape.



## lua.wetgenes.chipmunk.shape.collision_type


	f=shape:collision_type()
	f=shape:collision_type(f)

Get and/or Set the collision type for this shape.

The f argument can be a string in which case it will be converted to a 
number via the space:type function.



## lua.wetgenes.chipmunk.shape.elasticity


	f=shape:elasticity()
	f=shape:elasticity(f)

Get and/or Set the elasticity for this shape.



## lua.wetgenes.chipmunk.shape.filter


	f=shape:filter()
	f=shape:filter(f)

Get and/or Set the filter for this shape.



## lua.wetgenes.chipmunk.shape.friction


	f=shape:friction()
	f=shape:friction(f)

Get and/or Set the friction for this shape.



## lua.wetgenes.chipmunk.shape.query_point


	item = shape:query_point(x,y)

Find the nearest point on the shape from the point at x,y.

returns a table with the following info or nil for no hit

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.distance		-- the distance to the point of contact
	it.gradient_x	-- the normalised vector to collision (x)
	it.gradient_y	-- the normalised vector to collision (y)



## lua.wetgenes.chipmunk.shape.query_segment


	it = shape:query_segment(sx,sy,ex,ey,r)

Find the hitpoint along this raycast segment, from (sx,sy) to 
(ex,ey) with a radius of r. 

Returns a table with the following info or nil for no hit

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.normal_x		-- the normal at contact (x)
	it.normal_y		-- the normal at contact (y)
	it.alpha		-- how far along the segment the contact happened (0 to 1)



## lua.wetgenes.chipmunk.shape.radius


	radius=shape:radius()
	radius=shape:radius(radius)

Get and/or Set the radius for this shape. Setting is unsafe and may 
break the physics simulation.



## lua.wetgenes.chipmunk.shape.sensor


	f=shape:sensor()
	f=shape:sensor(f)

Get and/or Set the sensor flag for this shape.



## lua.wetgenes.chipmunk.shape.surface_velocity


	vx,vy=shape:surface_velocity()
	vx,vy=shape:surface_velocity(vx,vy)

Get and/or Set the surface velocity for this shape.



## lua.wetgenes.chipmunk.space


	space=chipmunk.space()

Create the space you will be simulating physics in.



## lua.wetgenes.chipmunk.space.add


	space:add(body)
	space:add(shape)
	space:add(constraint)

Add a body/shape/constraint to the space.



## lua.wetgenes.chipmunk.space.add_handler


	space:add_handler(handler,id1,id2)
	space:add_handler(handler,id1)
	space:add_handler(handler)

Add collision callback handler, for the given collision types.

The handler table will have other values inserted in it and will be 
used as an arbiter table in callbacks. So *always* pass in a new one to 
this function. There does not seem to be a way to free handlers so be 
careful what you add.

id1,id2 can be a string in which case it will be converted to a number 
via the space:type function.



## lua.wetgenes.chipmunk.space.body


	space:body(...)

Create and add this body to the space.



## lua.wetgenes.chipmunk.space.collision_bias


	v=space:collision_bias()
	v=space:collision_bias(v)

Get and/or Set the colision bias for this space.



## lua.wetgenes.chipmunk.space.collision_persistence


	v=space:collision_persistence()
	v=space:collision_persistence(v)

Get and/or Set the collision persistence for this space.



## lua.wetgenes.chipmunk.space.collision_slop


	v=space:collision_slop()
	v=space:collision_slop(v)

Get and/or Set the colision slop for this space.



## lua.wetgenes.chipmunk.space.constraint


	space:constraint(...)

Create and add this constraint to the space.



## lua.wetgenes.chipmunk.space.contains


	space:contains(body)
	space:contains(shape)
	space:contains(constraint)

Does the space contain this body/shape/constraint, possibly superfluous 
as we can check our own records.



## lua.wetgenes.chipmunk.space.current_time_step


	v=space:current_time_step()

Get the current time step for this space.



## lua.wetgenes.chipmunk.space.damping


	v=space:damping()
	v=space:damping(v)

Get and/or Set the damping for this space.



## lua.wetgenes.chipmunk.space.gravity


	vx,vy=space:gravity()
	vx,vy=space:gravity(vx,vy)

Get and/or Set the gravity vector for this space.



## lua.wetgenes.chipmunk.space.idle_speed_threshold


	v=space:idle_speed_threshold()
	v=space:idle_speed_threshold(v)

Get and/or Set the idle speed threshold for this space.



## lua.wetgenes.chipmunk.space.iterations


	v=space:iterations()
	v=space:iterations(v)

Get and/or Set the iterations for this space.



## lua.wetgenes.chipmunk.space.locked


	v=space:locked()

Get the locked state for this space, if true we cannot change shapes.



## lua.wetgenes.chipmunk.space.query_bounding_box


	array = space:query_bounding_box(lx,ly,hx,hy,group,categories,mask)

Find the shapes that are within this bounding box (lx,ly) to (hx,hy).
Use group,categories and mask to filter the results.

Returns an array of shapes.



## lua.wetgenes.chipmunk.space.query_point


	array = space:query_point(x,y,d,group,categories,mask)

Find the shapes that are within d distance from the point at x,y.
Use group,categories and mask to filter the results.

Returns an array of hit data, with each item containing the following.

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.distance		-- the distance to the point of contact
	it.gradient_x	-- the normalised vector to collision (x)
	it.gradient_y	-- the normalised vector to collision (y)



## lua.wetgenes.chipmunk.space.query_point_nearest


	item = space:query_point_nearest(x,y,d,group,categories,mask)

Find the nearest shape that is within d distance from the point at x,y.
Use group,categories and mask to filter the results.

returns a table with the following info or nil for no hit

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.distance		-- the distance to the point of contact
	it.gradient_x	-- the normalised vector to collision (x)
	it.gradient_y	-- the normalised vector to collision (y)



## lua.wetgenes.chipmunk.space.query_segment


	array = space:query_segment(sx,sy,ex,ey,r,group,categories,mask)

Find the shapes that are along this raycast segment, from (sx,sy) to 
(ex,ey) with a radius of r. Use group,categories and mask to filter the 
results.

Returns an array of hit data, with each item containing the following.

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.normal_x		-- the normal at contact (x)
	it.normal_y		-- the normal at contact (y)
	it.alpha		-- how far along the segment the contact happened (0 to 1)



## lua.wetgenes.chipmunk.space.query_segment_first


	it = space:query_segment_first(sx,sy,ex,ey,r,group,categories,mask)

Find the shapes that are along this raycast segment, from (sx,sy) to 
(ex,ey) with a radius of r. Use group,categories and mask to filter the 
results.

Returns a table with the following info or nil for no hit

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.normal_x		-- the normal at contact (x)
	it.normal_y		-- the normal at contact (y)
	it.alpha		-- how far along the segment the contact happened (0 to 1)



## lua.wetgenes.chipmunk.space.query_shape


	array = space:query_shape(shape)

Find the shapes that intersect with the given shape.

Returns an array of hit data, with each item containing the following.

	it.shape		-- the shape
	it.normal_x		-- the normal at contact (x)
	it.normal_y		-- the normal at contact (y)
	it.contacts		-- array of contact points -> {ax,ay,bx,by,distance,etc...}



## lua.wetgenes.chipmunk.space.reindex


	space:reindex(shape)
	space:reindex(body)
	space:reindex()

Reindex the shapes, either a specific shape, all the shapes in a body 
or just all the static shapes.



## lua.wetgenes.chipmunk.space.remove


	space:remove(body)
	space:remove(shape)
	space:remove(constraint)

Remove a body/shape/constraint from this space.



## lua.wetgenes.chipmunk.space.sleep_time_threshold


	v=space:sleep_time_threshold()
	v=space:sleep_time_threshold(v)

Get and/or Set the sleep time threshold for this space.



## lua.wetgenes.chipmunk.space.step


	space:step(time)

Run the simulation for time in seconds. EG 1/60.



## lua.wetgenes.chipmunk.space.type


	number = space:type(name)
	name = space:type(number)

Manage collision types, pass in a string and always get a number out. 
This number is consistent only for this space.

Alternatively pass in a number and get a string or nil as a result.



## lua.wetgenes.csv



	local wcsv = require("wetgenes.csv")

Load and save csv, prefrably using tab sperators. 

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.

and when using commas a , must be placed inside a quoted string with a 
double "" to escape " within this string.

This is intended for "small" csv files that fit in memory so does not 
stream or try and do anything clever to reduce memory overheads.



## lua.wetgenes.csv.build


Build csv data into a string from a simple table of lines where each 
line is a table of cells.

	text = wcsv.build(lines)
	text = wcsv.build(lines,opts)



## lua.wetgenes.csv.doesc


Escape special chars within a csv cell.



## lua.wetgenes.csv.doquote


Wrap a string in quotes and escape any " within that string using csv 
rules.



## lua.wetgenes.csv.map


Use the first line to map all other lines into named keys, an empty 
string will map to nil. This will return an array of items that is 
smaller than the array of lines by at least one as we also trim 
trailing empty objects.

	items = wcsv.map(lines)
	items = wcsv.map(wcsv.parse(text))



## lua.wetgenes.csv.parse


Parse csv data from a chunk of text. Returns a simple table of lines 
where each line is a table of cells. An empty or missing string 
indicates an empty cell. The second return can be ignored or used to 
build a csv in a similar format to the one we read.

	lines,opts = wcsv.parse(text)
	lines,opts = wcsv.parse(text,opts)

Opts can be used to control how the parsing is performed pass in a 
seperator value to contol how a line is split.

	lines,opts = wcsv.parse(text,{seperator=","})

Note that we also return the seperator we used within the second return 
and will guess the right one using the first line if one is not given.



## lua.wetgenes.csv.unesc


Unescape special chars within a csv cell.



## lua.wetgenes.csv.unquote


Remove quotes from a strine and unescape any "" within that string. If 
the string is not in quotes then we return it as is.



## lua.wetgenes.deepcompare


	deepcompare(a,b)

Returns true if a==b , this iterates and recurses into tables.



## lua.wetgenes.deepcopy


	deepcopy(tab)

Create and return a new table containing the same data as the input. If any of
the table values (not keys) are tables then these are also duplicated,
recursively.

If this is called with a value that is not a table then that value is just
returned so it's safe to call on values without checking them.



## lua.wetgenes.export


	... = wetgenes.export(table,...)

Export multiple names from this table as multiple returns, can be 
used to pull functions out of this module and into locals like so

	local export,lookup,deepcopy=require("wetgenes"):export("export","lookup","deepcopy")

Or copy it into other modules to provide them with the same functionality.

	M.lookup=require("wetgenes").lookup



## lua.wetgenes.grd


	local wgrd=require("wetgenes.grd")

We use wgrd as the local name of this library.

Handle bitmap creation, loading, saving and blitting. The bitmap and 
the colormap for indexed bitmaps are represented by the same data 
structure which describes a continuous chunk of memory with optional 
ability to select an area of a larger chunk using simple byte spans.

Swanky Paint uses this to manage its bitmaps and its also used to 
convert art into data at build time for use in the GameCake engine. The 
PageCake engine uses this for image management, creating live thumbnails 
and so on.

We load and save jpeg, png and gif. The png lib contains extensions for 
apng which allows animation chunks. Animations are contained in the Z 
(depth) dimension of the grd.

The following are possible format options that we support. Most of them 
are OpenGL friendly.

	wgrd.FMT_U8_RGBA
	
32 bits per pixel with a byte order of red, green, blue, alpha and a 
little endian U32 of ABGR. We prefer this byte order because OpenGL.

	wgrd.FMT_U8_ARGB

32 bits per pixel with a byte order of alpha, red, green, blue and a 
little endian U32 of BGRA.

	wgrd.FMT_U8_RGB

24 bits per pixel with a byte order of red, green, blue.

	wgrd.FMT_U8_INDEXED
	
8 bits per pixel which indexes a wgrd.FMT_U8_RGBA palette.

	wgrd.FMT_U8_LUMINANCE

8 bits per pixel, grey scale only.

	wgrd.FMT_U8_ALPHA

8 bits per pixel, alpha only.

	wgrd.FMT_U16_RGBA_5551

16 bits per pixel with 5 bits each of red,green,blue and 1 bit of alpha 
packed into a single U16.

	wgrd.FMT_U16_RGBA_4444

16 bits per pixel with 4 bits each of red,green,blue,alpha packed into 
a single U16.

	wgrd.FMT_U16_RGBA_5650

16 bits per pixel with 4 bits of red, 5 bits of green and 4 bits of 
blue packed into a single U16.


	wgrd.FMT_U8_RGBA_PREMULT
	wgrd.FMT_U8_ARGB_PREMULT
	wgrd.FMT_U8_INDEXED_PREMULT
	wgrd.FMT_U16_RGBA_5551_PREMULT
	wgrd.FMT_U16_RGBA_4444_PREMULT
	wgrd.FMT_U16_RGBA_5650_PREMULT

Are all just pre-multiplied alpha versions of the base format described 
above.

Check this link out for why pre-multiplied alpha is a good idea for any 
image that will be used as a texture 
http://blogs.msdn.com/b/shawnhar/archive/2009/11/06/premultiplied-alpha.aspx


	wgrd.GRD_FMT_HINT_PNG
	wgrd.GRD_FMT_HINT_JPG
	wgrd.GRD_FMT_HINT_GIF

These are used to choose a specific file format when loading or saving.



## lua.wetgenes.grd.adjust_contrast


	ga:adjust_contrast(sub,con)

sub is the middle grey value, probably 127, and con is the amount of 
contrast.

A con of 0 should have no effect, a con of -1 will be a flat grey and a 
con of 1 will give a huge contrast increase.



## lua.wetgenes.grd.adjust_hsv


	ga:adjust_hsv(hue,saturation,value)

Add hue and adjust -1 to +1 in for saturation and value.



## lua.wetgenes.grd.adjust_rgb


	ga:adjust_rgb(red,green,blue)

Adjust -1 to +1 in for each RGB component.



## lua.wetgenes.grd.attr_redux


	g:attr_redux(cw,ch,num,sub,bak)

Perform attribute clash simulation on an indexed image.

cw,ch are the width and height of each character we are simulating, 8x8 
is the right size for spectrum attrs but could be 4x8 for c64 multicolor 
mode.

num is the number of colors allowed within this area, so 2 for spectrum mode.

sub is the size of sub pallete groups, eg 16 in nes mode or 8 in 
spectrum mode, EG bright simulation in spectrum mode requires all 
colors in a attr block to be from the bright palette or the dark 
palette no mixing so this forces that grouping. Set to 0 or 256 and 
this restriction will be disabled.

bak is the index of the background color that is shared across all 
characters, set to -1 if there is no shared background color.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.blit


	g:blit(gb,x,y,cx,cy,cw,ch)

Blit a 2D area from one grd into another.

gb is the grd to blit from.

x,y is the location to blit too.

cx,cy,cw,ch is a clip area that should be applied to gb before it is 
blitted. EG to specify an area within gb. If not provided it will 
default to the entirety of gb,

g (destination) must be FMT_U8_RGBA and gb (source) must be 
FMT_U8_RGBA_PREMULT this function will blend the images using normal 
alpha blending.

This is not overly optimised but should be reasonably fast.



## lua.wetgenes.grd.clear


	g:clear(color)

Fill this grd with a single color, the color passed in depends on the 
format of the grd, it could be an index value for 8bit images or a 
32bit value for rgba images.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.clip


	gr=g:clip(x,y,z,w,h,d)

create a clipped window into this grd

the actual data is still stored in the original, so any changes there will effect the newly returned grd

x,y,z are the staring pixel and w,h,d are the width height and depth in pixels.

If you intend to use this clipped area for an extended period of time then you should duplicate this grd once you do this.

This returns a new grd with gr.parent set to g (the original grd)

This is a very shallow dangerous copy and should only really be used for temporary actions.



## lua.wetgenes.grd.convert


	g:convert(fmt)

Convert this grd to a new format, eg start with an 8 bit indexed grd 
and convert it to 32 bit like by passing in wgrd.FMT_U8_RGBA as the fmt.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.copy_data


	g:copy_data(gb)

Copy all of the bitmap data from gb into g.



## lua.wetgenes.grd.copy_data_layer


	g:copy_data_layer(gb,z,zb)

Copy one layer (frame) of the bitmap data from gb into g. z is the 
depth of the layer to copy into zb is the depth of the layer to copy 
from.



## lua.wetgenes.grd.create


	ga=wgrd.create(gb)

Duplicate the grd.
	
	ga=wgrd.create(format,width,height,depth)

Create a grd in the given format with the given width height and depth. 

	ga=wgrd.create(filename,opts)

Load an image from the file system.

	ga=wgrd.create()
	
Create a blank grd of 0 dimensions.

This is usually the only wgrd function you would need to call as once you 
have a grd you can use the : calling convention to modify it. The other 
functions will be shown as examples using the : calling convention.

	wgrd.create():load(opts)

For instance could be used if you wish to perform a more esoteric load 
than from the file system.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a grd object.



## lua.wetgenes.grd.create_convert


	g:create_convert(fmt)

Like convert but returns a new grd rather than converting in place.



## lua.wetgenes.grd.create_normal


	gr=g:create_normal()

convert a greyscale height map  into an rgb normal map using the sobel filter.



## lua.wetgenes.grd.destroy


	g:destroy()

Free the grd and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.
	


## lua.wetgenes.grd.duplicate


	ga = g:duplicate()

Create a duplicate of this grd and return it.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.fillmask


	ga:fillmask(gb,seedx,seedy)

Fill gb with a fillmask version of ga that starts the floodfill at 
seedx,seedy



## lua.wetgenes.grd.flipx


	g:flipx()
	
This function flips the image reversing the x axis.
	


## lua.wetgenes.grd.flipy


	g:flipy()
	
This function flips the image reversing the y axis.

Some image formats and rendering engines like to use upside down images 
so this is rather useful.



## lua.wetgenes.grd.info


	g:info()

This function looks at the userdata stored in g[0] and fills in the g 
table with its values. So it refreshes the width height etc values to 
reflect any changes. This should not need to be called explicitly as it 
is called whenever we change anything.



## lua.wetgenes.grd.load


	g:load(opts)

Load an image from memory or file system depending on settings in opts.

	opts.fmt

Lets you choose an image format, the strings "jpg","png" or "gif" will 
be converted to the appropriate wgrd.FMT_HINT_* value.

	opts.data

Flags this as a load from memory and provides the data string to load 
from.

	opts.filename

Flags this as a load the file system and provides the file name to 
open.
 
Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.load_data


	g:load_data(datastring,format)

Load an image from memory.
 
Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.load_file


	g:load_file(filename,format)

Load an image from the file system.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.
 


## lua.wetgenes.grd.paint


	g:paint(gb,x,y,cx,cy,cw,ch,mode,trans,color)

Blit a 2D area from one grd into another using dpaint style paint modes.

Both grids must be indexed - FMT_U8_INDEXED

gb is the grd to blit from.

x,y is the location to blit too.

cx,cy,cw,ch is a clip area that should be applied to gb before it is 
blitted. EG to specify an area within gb. If not provided it will 
default to the entirety of gb,

mode is one of the following

	PAINT_MODE_TRANS
	
Skip the transparent color.

	GRD_PAINT_MODE_COLOR
	
Skip the transparent color and make every solid pixel the same color.

	GRD_PAINT_MODE_COPY

Copy the entire area.

	GRD_PAINT_MODE_XOR

XOR the values. (Can be used to find differences in an image)

	GRD_PAINT_MODE_ALPHA

Skip the transparent colors as defined in the palette.


trans is the index of the transparent color, bground color, for use in 
the appropriate modes.

color is the index of the drawing color, foreground color, for use in 
the appropriate modes. 


This is not overly optimised but should be reasonably fast.



## lua.wetgenes.grd.palette


	g:palette(x,w)
	g:palette(x,w,"")
	g:palette(x,w,tab)
	g:palette(x,w,str)
	g:palette(x,w,grd)

These are the same as g:pixels() but refer to the palette information 
which is stored as a 1 pixel high 256 pixel wide rgba image. The use of 
"" to read a string of bytes and passing in either a table of numerical 
values or string of bytes to write into the palette is the same system 
as used with pixels.



## lua.wetgenes.grd.pixels


	g:pixels(x,y,w,h)
	g:pixels(x,y,z,w,h,d)

Read the area of pixels as a table of numerical values, the amount of 
numbers you get per pixel *depends* on the format of the grd.

	g:pixels(x,y,w,h,"")
	g:pixels(x,y,z,w,h,d,"")

Read the area of pixels as a string of byte values, the amount of bytes 
you get per pixel *depends* on the format of the grd. Note the passing 
in of an empty string to indicate that you with to receive a string.

	g:pixels(x,y,w,h,tab)
	g:pixels(x,y,z,w,h,d,tab)

Write the area of pixels from a table of numerical values which is 
provided in tab, the amount of numbers you need to provide per pixel 
*depends* on the format of the grd.

	g:pixels(x,y,w,h,str)
	g:pixels(x,y,z,w,h,d,str)

Write the area of pixels from a string of bytes which is provided in 
str, the amount of bytes you need to provide per pixel *depends* on the 
format of the grd.

	g:pixels(x,y,w,h,grd)
	g:pixels(x,y,z,w,h,d,grd)

Write the area of pixels from a grd which is provided in grd. use 
clip/layer functions to select a portion of a larger grd.

As you can see depending on the arguments given this does one of two 
things, read some pixels or write some pixels. The area that is to be 
used is provided first, as a 2d(x,y,w,h) or 3d(x,y,z,w,h,d) area. To 
read or write the entire 2d image or the first frame of an animation 
use (0,0,g.width,g.height)

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns the requested data.



## lua.wetgenes.grd.quant


	g:quant(num)

Convert to an 8bit indexed image containing a palette of the requested size.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.remap


	ga:remap(gb)

Fill gb with a remaped version of ga, each pixel is mapped to the closest palette entry.



## lua.wetgenes.grd.reset


	g:reset()

Reset the grd which will now be a blank image of 0 dimensions.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.resize


	g:resize(w,h,d)
	
Resize the image to the given dimensions, this does not scale the image 
data so instead each pixel will be in the same place after the resize. 
This gives a crop effect when shrinking and extra blank area at the 
bottom right when growing. Useful if for instance you want to upload a 
texture to OpenGL and need to change the size to be a power of two in 
width and height so you can mipmap it.
	


## lua.wetgenes.grd.save


	g:save(opts)

Save an image to memory or filesytem depending on settings in opts.

	opts.fmt

Lets you choose an image format, the strings "jpg","png" or "gif" will 
be converted to the appropriate wgrd.FMT_HINT_* value.

We will guess opts.fmt from the file name extension if it is not 
provided and a file name is.

	opts.filename

Flags this as a load the file system and provides the file name to 
write to. If no filename is given then we will be saving into memory 
and be returning that data string as the first return value.
 
Returns nil,error if something goes wrong so can be used with assert.

If no file name is given then we *return* the data string that we saved.



## lua.wetgenes.grd.scale


	g:scale(w,h,d)
	
Scale the image to the given dimensions, this is currently using a 
terrible scale filter that is only any good at halving or doubling the 
size.

This should only be used to create mipmaps until it is replaced with a 
better scale filter.
	


## lua.wetgenes.grd.shrink


	g:shrink(area)

area is an {x=0,y=0,z=0,w=100,h=100,d=100} style table specifying a 3D
area, set {z=0,d=1} for a 2D area.

You should set this to the full size of the image.

This function looks at the pixels in that area and shrinks each edge 
inwards if it is fully transparent then return this new area in the 
same table that was passed in.

You can then use this information to crop this image resulting in a 
smaller sized grd containing all the solid pixels.



## lua.wetgenes.grd.slide


	g:slide(dx,dy,dz)

Slide the image along the x,y,z axis by the given amounts. The image wraps around the edges 
so no pixels are lost just moved around.



## lua.wetgenes.grd.sort_cmap


	ga:sort_cmap()

Sort cmap into a "good" order and remap the image.



## lua.wetgenes.grd.stream


	stream=g:stream(filename)
	stream=g:stream({filename=filename,...})

Open a GIF stream, returns a table with the following functions,

	stream:write(ga)
	
Add a frame to the gif, each frame should be the same size and color map.

	stream:close()

Close the stream and finalise the GIF.



## lua.wetgenes.grd.xor


	g:xor(ga)

Set our image data to the XOR of the image/palette data from ga and g.

This is intended to be combined with g:shrink to work out the area of 
change between the two images.

Both grds must be the same size and format.



## lua.wetgenes.grdcanvas


	local wgrdcanvas=require("wetgenes.grdcanvas")

We use wgrdcanvas as the local name of this library.




## lua.wetgenes.grdhistory


	local wgrdhistory=require("wetgenes.grdhistory")

We use wgrdhistory as the local name of this library.

Add extra functionality to wetgenes.grd primarily these are functions that 
are used by swanky paint to manage its internal data.



## lua.wetgenes.grdhistory.history


	local history=grdhistory.history(grd)

Create and bind a history object to the given grd object. The history 
lives inside the grd and can be accesd as grd.history just as the grd can 
be accessed through the history as history.grd



## lua.wetgenes.grdlayers


	local wgrdlayers=require("wetgenes.grdlayers")

We use wgrdlayers as the local name of this library.

Add extra functionality to wetgenes.grd primarily these are functions that 
are used by swanky paint to manage its internal data.



## lua.wetgenes.grdpaint


	local wgrdpaint=require("wetgenes.grdpaint")

We use wgrdpaint as the local name of this library.

Add extra functionality to wetgenes.grd primarily these are functions that 
are used by swanky paint to manage its internal data.

Primarily we add a concept of "layers" and "history" these interfaces 
are added to a grd via a new object that lives inside the grd table and 
binds them together.

EG grd.history contains history data and functions.

As these are written first for swankypaint they may only work with 
indexed images and are currently in state of flux so may take a while 
to settle down.



## lua.wetgenes.grdsvg


	wgrdsvg=require("wetgenes.grdsvg")

Build svg files from grd data (bitmaps)



## lua.wetgenes.grdsvg.string


	local svgstring=wgrdsvg.string(grd,opts)

Return an svg formated string that represents the input grd which must 
be an indexed (8bit) format. Each pixel will be converted into a filled 
svg rectangle element. Opts can contain the following options to 
control the generation of the svg file.

	opts.skip_transparent_pixels=true

Do not export a rectangle for fully transparent pixels.

	opts.scalex=1

Width of each exported pixel.

	opts.scaley=1

Height of each exported pixel.



## lua.wetgenes.json


	local wjson=require("wetgenes.json")

	-- or export the main functions like so
	local json_encode,json_decode=require("wetgenes.json"):export("encode","decode")

other json encode/decode using pure lua library seemed too slow, 
here is a fast and loose one lets see if it goes any faster :) 
should be a direct replacement for JSON4Lua which is what I was 
using before I profiled where all the time was getting spent...

I needed it to be pure json as I was running it on googles appengine so
the lua was actually running in java, no C available.

Anyhow I hope it is useful, in order to get it running faster I cut 
across some corners so you may find some obscure problems.



## lua.wetgenes.json.decode


	json_table = wjson.decode(json_string)
	json_table = wjson.decode(json_string,opts)

Convert a json string into a lua table.

Set opts.null to wetgenes.json.null (or indeed any other value) if you 
would like to have this as nulls in your results. By default nulls are 
replaced with nil and therefore invisible.

Any object key string that looks like a number will be converted to a 
number. This will probably reverse any numbers we converted to strings 
when encoding. Set opts.keystring=true to turn off this behaviour.




## lua.wetgenes.json.encode


	json_string = wjson.encode(json_table)
	json_string = wjson.encode(json_table,opts)

Convert a lua table into a json string. Note it must be valid json, 
primarily make sure that the table is either an array or a dictionary 
but never both. Note that we can not tell the difference between an 
empty array and an empty object and will assume it is an object.

An array must have a length>0 and contain an element in the first slot, 
eg array[1] and only contain numerical integer keys between 1 and the 
length. This allows for the possibility of some nil holes depending on 
the length lua returns but holes are not a good idea in arrays in lua. 
Best to use false or the special wjson.null value and avoid holes.

Also some of the internal lua types will cause errors, eg functions 
as these can not be converted into json.

include nulls in the output by using wetgenes.json.null

opts is an optional table that can set the following options.

	ops.pretty=true
	ops.pretty=" "
		Enable pretty printing, line feeds and indents and set each 
		indent level to multiples of the given string or " ".
		
	ops.white=true
	ops.white=" "
		Enable white space but not lines or indents, just a single space 
		between value assignment to make line wrapping easier.
 
	ops.sort=true
		Sort the keys, so we can create stable output for better diffing.



## lua.wetgenes.json_diff


(C) 2024 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

I assume we are not competing in the lua json_diff library world so 
just call it json_diff

	local json_diff=require("wetgenes.json_diff")

When we talk of json objects or json values we mean that the values 
must be valid json. So no storing of functions/etc or mixxing of tables 
and objects. Infinite recursion where data links back into itself is 
also not possible in json so not allowed/expected here.

We assume we have data that could be validly serialised as json but 
this is not enforced. If you want to be 100% safe then convert your 
data to json text and back again before handing it to these functions.

If a table has a length of more than 0 then it is considered a json 
array otherwise it is a json object. An empty table is considered an 
empty object there is no lua equivalent to an empty json array.

Lua is not good with holes in its arrays, so that is also not suported.

We do try and diff arrays of objects, but, since arrays may shift up 
and down it would be better if you had object maps of id to data and 
arrays of ids that references these maps rather than arrays of objects.

Json arrays must be normal lua tables, so first index is 1 not 0 this 
may cause problems if your data is made of objects that have been 
accidently converted into arrays. Probably wont have happened but best 
to be aware of possible sharp edges.



## lua.wetgenes.json_diff.apply


apply a diff , please dupe a before handing it to this function as we 
will be altering it.



## lua.wetgenes.json_diff.array_common


Given two arrays of json data, return the length , starta , startb of 
the longest common subsequence in table indexes or nil if not similar.



## lua.wetgenes.json_diff.array_match


Given two arrays of json data, return two synced arrays of arrays where 
as much json data as possible match in each sub batch.

each of these arrays can be concatonated to create the original array.

When stepping thoigh bothe arrays, matches are shown by the sub array 
having equality and differences will be insertions or deletions 
depending on which of the two has the empty array.

EG these two lists

	[a,b,c,d,e,f]
	[a,b,e,f]

would become

	[ [a,b] , [c,d] , [e,f] ]
	[ [a,b] , [] , [e,f] ]

The two [a,b] arrays will be equal to each other when tested with a 
simple == compare as they are references to the same table.



## lua.wetgenes.json_diff.array_trim


Given two arrays of json data, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.



## lua.wetgenes.json_diff.diff


Return a diff of two values



## lua.wetgenes.json_diff.dupe


create a (possible) deep copy duplicate of a json value



## lua.wetgenes.json_diff.equal


Compare two json values and return true if they are equal, this may 
decend into a tree of tables and objects so can be an expensive test.



## lua.wetgenes.json_diff.similar


Compare two json values and return true if they are similar arrays or 
objects, this may descend into a tree of tables and objects so can be 
an expensive test.

An array or object is similar if it contains at least one value that is 
the same in both.

Will return false if not given two objects or two arrays.



## lua.wetgenes.json_diff.undo


undo a diff which *must* have been created with the both flag set in 
order to have undo data available.



## lua.wetgenes.json_pack


(C) 2024 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local json_pack=require("wetgenes.json_pack")

Turn a valid json table into a binary data string and visa versa.

A valid json table...

	Does not include functions/userdata/etc.
	
	Does not self reference.
	
	Each table should be an array or an object never both.
	
	May have metatables as long as it is not too funky so added 
	function calls for a class should be fine but this will obviously 
	be lost. Please test and check outputs if including meta.

So normal json style data.


Currently we are using cmsgpack and zlib, this may be changed if a 
better way is found.



## lua.wetgenes.json_pack.from_data


	j=json_pack.from_data(d)

Where j is some json data and d is a binary string of the same data.



## lua.wetgenes.json_pack.into_data


	d=json_pack.into_data(j)

Where j is some json data and d is a binary string of the same data.



## lua.wetgenes.lookup


	value = wetgenes.lookup(table,...)

Safe recursive lookup within a table that returns nil if any part of 
the lookup is nil so we never cause an error but just return nil. 
This is intended to replace the following sort of code

	a = b and b.c and b.c.d and b.c.d.e

To get e only if all of its parent bits exist and not to cause any 
error if they do not. instead use

	a = lookup(b,"c","d","e")



## lua.wetgenes.midi.clients


	m:clients()

fetch table of clients




## lua.wetgenes.midi.create


	m=wmidi.create()

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a midi object.



## lua.wetgenes.midi.destroy


	m:destroy()

Free the midi and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.



## lua.wetgenes.midi.event_to_string


	str = m:event_to_string(event)

Convert an event to a single line string for printing to the console.



## lua.wetgenes.midi.get


	m:get()

get all values for this connection and store them in m




## lua.wetgenes.midi.peek


	m:peek(it)

Returns a event if there is one or null if none are currently 
available.



## lua.wetgenes.midi.port_create


	p = m:port_create(name,caps,type)

Create a port with the given name and capability bits and type.

bits and type are either a number or a table of bitnames.

Returns the number of the port created which should be used in 
port_destroy or nil if something went wrong.



## lua.wetgenes.midi.port_destroy


	m:port_destroy(num)

Destroy a previously created port. Returns nil on failure, true on 
success.



## lua.wetgenes.midi.pull


	m:pull(it)

Receive an input midi event, blocking until there is one.

Occasionally, for "reasons" this may return nil.



## lua.wetgenes.midi.push


	m:push(it)

Send an output midi event.



## lua.wetgenes.midi.set


	m:set()

set all values for this connection from values found in m




## lua.wetgenes.midi.string_to_clientport


	client,port = m:string_to_clientport(str)

Convert a "client:port" string to two numbers client,port this can 
either be two decimal numbers or, if a m:scan() has been performed, 
then a partial case insensitive matching to the name of existing 
clients and ports may get a port number.

Will return a nil if we can not work out which client or port you mean.



## lua.wetgenes.midi.subscribe


	m:subscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:subscribe{
		source="0:0",
		dest="1:0",
	}

Creates a persistent subscription between two ports.



## lua.wetgenes.midi.unsubscribe


	m:unsubscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:unsubscribe{
		source="0:0",
		dest="1:0",
	}

Removes a persistent subscription from between two ports.



## lua.wetgenes.package


	wpackage=require("wetgenes.package")

Some generic lowlevel functions for messing about with how lua works, 
eg how modules are loaded.



## lua.wetgenes.package.reload


	local module=wpackage.reload(modulename)

A wrapper for require that forces a very dumb module reload by merging 
a newly loaded module into the old module table if one already exists 
this can easily break everything but should mostly work out of dumb 
luck.



## lua.wetgenes.path


Manage file paths under linux or windows, so we need to deal with \ or 
/ and know the root difference between / and C:\

	local wpath=require("wetgenes.path")



## lua.wetgenes.path.currentdir


Get the current working directory, this requires lfs and if lfs is not 
available then it will return wpath.root this path will have a trailing 
separator so can be joined directly to a filename.

	wpath.currentdir().."filename.ext"



## lua.wetgenes.path.dir


	local dir=wpath.dir(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the dir component of the result.



## lua.wetgenes.path.ext


	local ext=wpath.ext(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the ext component of the result.



## lua.wetgenes.path.folder


	local folder=wpath.folder(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the folder component of the result.



## lua.wetgenes.path.join


join a split path, tables are auto expanded



## lua.wetgenes.path.name


	local name=wpath.name(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the name component of the result.



## lua.wetgenes.path.normalize


remove ".." and "." components from the path string



## lua.wetgenes.path.parent


Resolve input and go up a single directory level, ideally you should 
pass in a directory, IE a string that ends in / or \ and we will return 
the parent of this directory.

If called repeatedly, then eventually we will return wpath.root



## lua.wetgenes.path.parse


split a path into named parts like so

	|--------------------------------------------|
	|                     path                   |
	|-----------------------|--------------------|
	|         dir           |        file        |
	|----------|------------|----------|---------|
	| root [1] | folder [2] | name [3] | ext [4] |
	|----------|------------|----------|---------|
	| /        | home/user/ | file     | .txt    |
	|----------|------------|----------|---------|

this can be reversed with simple joins and checks for nil, note that 
[1][2][3][4] are forced strings so will be "" rather than nil unlike 
their named counterparts. This means you may use wpath.join to reverse 
this parsing.

	dir = (root or "")..(folder or "")
	file = (name or "")..(ext or "")
	path = (dir or "")..(file or "")
	path = (root or "")..(folder or "")..(name or "")..(ext or "")
	path = [1]..[2]..[3]..[4]
	path = wpath.join(it)
	
if root is set then it implies an absolute path and will be something 
like C:\ under windows.



## lua.wetgenes.path.relative


Build a relative path from point a to point b this will probably be a 
bunch of ../../../ followed by some of the ending of the second 
argument.



## lua.wetgenes.path.resolve


Join all path segments and resolve them to absolute using wpath.join 
and wpath.normalize with a prepended wpath.currentdir as necessary.



## lua.wetgenes.path.root


	local root=wpath.root(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the root component of the result.



## lua.wetgenes.path.setup


setup for windows or linux style paths, to force one or the other use

	wpath.setup("win")
	wpath.setup("nix")

We automatically call this at startup and make a best guess, you can 
revert to this best guess with

	wpath.setup()

This is a global setting, so be careful with changes. Mostly its 
probably best to stick with the best guess unless we are mistakenly 
guessing windows.



## lua.wetgenes.path.split


split a path into numbered components



## lua.wetgenes.plate.replace_lookup


	value=wetgenes.plate.replace_lookup(name,data)

Calls "wetgenes.plate.table_lookup" then performs special formatting on 
table returns.

Always returns a string or nil, so number values will converted to a 
string.




## lua.wetgenes.plate.replace_lookup_istable


	bool=wetgenes.plate.replace_lookup_istable(name,data)

Test if the return from "wplate.table_lookup" is a table.




## lua.wetgenes.safecall


	... = wetgenes.safecall(func,...)

Call a function func(...) wrapped in an xpcall to catch and ignore 
errors, the errors are printed to stderr with a traceback and the 
function returns nil on an error.

So provided the function returns not nil on success then you can still 
tell if the function completed OK. Best to use for things that are OK 
to fail and the rest of the code will work around it.



## lua.wetgenes.safewrap


	savefunc = wetgenes.safecall(func)

Wrap a funciton in safecall, so it will never generate errors but can 
be called as normal.



## lua.wetgenes.set_env


	gamecake -e" require('wetgenes').savescripts('./') "

Run the above from the command line.

This will export all the gamecake internal strings into the file system 
it is saved into the current directory so be careful where you run it.

Game Cake checks the files system first so, these files can be modified 
and they will replace the built in versions.

	gamecake -e" require('wetgenes').savescripts('./internal/') "

This is a safer version that will save the files to ./internal/lua/* 
instead of just ./lua/*



## lua.wetgenes.snippets


	-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
	local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

	-- grab some util functions
	local export,lookup,set_env=require("wetgenes"):export("export","lookup","set_env")

	-- single line replacement for the module creation function
	local M={} ; package.loaded[(...)]=M ; M.module_name=(...) ; M.export=export


A place to keep bits of code that needs to be copypasta into most 
modules, for instance above is the current boilerplate code to be 
used when starting a new module.



## lua.wetgenes.spew


	local spew=require("wetgenes.spew").connect(oven.tasks)

You must pass in a previously created tasks object that you will be 
calling update on regually so that our coroutines can run. eg oven.tasks

Connect and talk to the wetgenes spew server using wetgenes.tasks 
task for meta state and a thread for lowlevel sockets.

You push messages in and can pull messages out or, setup a hook 
function to auto pull and process messages in our coroutine as they are 
received.



## lua.wetgenes.spew.connect


	local spew=require("wetgenes.spew").connect(oven.tasks)
	local spew=require("wetgenes.spew").connect(oven.tasks,host)
	local spew=require("wetgenes.spew").connect(oven.tasks,host,port)

You must pass in an active tasks object.

Create task and thread connection to the host. host and port 
default to wetgenes.com and 5223 so can be left blank unless you want 
to connect to a local host for debugging.

There are now 3 ways to handle msgs

	spew.push(msg)

To send a msg

	local msg,s=spew.pull()

To receive a message, will return nil if no messages available. The 
second return value is the input string packet for this decoded 
message.

	spew.hook=function(msg,s) print(msg) end

Set a hook functions to auto pull all available messages durring tasks 
update. Note that if you do this spew.pull() will no longer work as all 
messages are auto pulled and sent to this hook function.

Note when receiving a msg you must not alter or cache the table you 
are given as it is internal data and is reused. You must duplicate it 
if you want to keep it arround.



## lua.wetgenes.spew.test



test



## lua.wetgenes.tardis


Time And Relative Dimensions In Space is of course the perfect name for 
a library of matrix based math functions.

	local tardis=require("wetgenes.tardis")

This tardis is a lua library for manipulating time and space with numbers.
Designed to work as pure lua but with a faster, but less accurate, f32 core by
default. ( this core seems to be slightly faster/same speed as vanilla lua but
slower than luajit , so is currently disabled )

Recoil in terror as we use two glyph names for classes whilst typing in 
random strings of numbers and operators that may or may not contain 
tyops.

	v# vector [#]
	m# matrix [#][#]
	q4 quaternion

Each class is a table of # values [1] to [#] the 2d number streams are 
formatted the same as opengl (row-major) and metatables are used to 
provide methods.

The easy way of remembering the opengl 4x4 matrix layout is that the
translate x,y,z values sit at 13,14,15 and 4,8,12,16 is normally set
to the constant 0,0,0,1 for most transforms.

		 | 1  5  9  13 |
		 | 2  6  10 14 |
	m4 = | 3  7  11 15 |
		 | 4  8  12 16 |

This seems to be the simplest (programmer orientated) description of 
most of the maths used here so go read it if you want to know what the 
funny words mean.

http://www.j3d.org/matrix_faq/matrfaq_latest.html



## lua.wetgenes.tardis.array



Array is the base class for all other tardis classes, it is just a 
stream of numbers, probably in a table but possibly a chunk of f32 
values in a userdata.



## lua.wetgenes.tardis.array.__add


	r = array.__add(a,b)

Add a to b and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.



## lua.wetgenes.tardis.array.__div


	r = array.__div(a,b)

Replace b with 1/b and then call the appropriate product function depending on
the argument classes. Always creates and returns a.new() value.



## lua.wetgenes.tardis.array.__eq


	r = array.__eq(a,b)

meta to call a:compare(b) and return the result



## lua.wetgenes.tardis.array.__mul


	r = array.__mul(a,b)

Call the appropriate product function depending on the argument classes. Always
creates and returns a.new() value.



## lua.wetgenes.tardis.array.__sub


	r = array.__sub(a,b)

Subtract b from a and return a a.new(result) so the class returned will match the
input class of a and neither a or b will be modified.



## lua.wetgenes.tardis.array.__tostring


	string = array.__tostring(it)

Convert an array to a string this is called by the lua tostring() function,



## lua.wetgenes.tardis.array.__unm


	r = array.__unm(a)

Subtract b from 0 and return a a.new(result) so the class returned will match the
input class of a but a will not be modified



## lua.wetgenes.tardis.array.abs


	r=it:abs(r)
	r=it:abs(it.new())

Perform math.abs on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.acos


	r=it:acos(r)
	r=it:acos(it.new())

Perform math.acos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.add


	r=it:add(b,r)
	r=it:add(b,it.new())

Add b to it. b may be a number or another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.asin


	r=it:asin(r)
	r=it:asin(it.new())

Perform math.asin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.atan


	r=it:atan(r)
	r=it:atan(it.new())

Perform math.atan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.ceil


	r=it:ceil(r)
	r=it:ceil(it.new())

Perform math.ceil on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.compare


	a=a:compare(b)
	a=a:compare(1,2,3,4)

Are the numbers in b the same as the numbers in a, this function will 
return true if they are and false if they are not.

If the arrays are of different lengths then this will return false.

Numbers to test for can be given explicitly in the arguments and we 
follow the same one level of tables rule as we do with array.set so any 
class derived from array can be used.



## lua.wetgenes.tardis.array.cos


	r=it:cos(r)
	r=it:cos(it.new())

Perform math.cos on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.exp


	r=it:exp(r)
	r=it:exp(it.new())

Perform math.exp on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.floor


	r=it:floor(r)
	r=it:floor(it.new())

Perform math.floor on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.fract


	r=it:fract(r)
	r=it:fract(it.new())

Return the fractional part of each value using math.modf.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.log


	r=it:log(r)
	r=it:log(it.new())

Perform math.log on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.max


	r=it:max()

Return a single number value that is the maximum of all values in this array.



## lua.wetgenes.tardis.array.min


	r=it:min()

Return a single number value that is the minimum of all values in this array.



## lua.wetgenes.tardis.array.mix


	r=a:mix(b,s,r)

Mix values between a and b where a is returned if s<=0 and b is returned if s>=1

If r is provided then the result is written into r and returned otherwise a is
modified and returned.



## lua.wetgenes.tardis.array.pow


	r=it:pow(p,r)
	r=it:pow(p,it.new())

Perform math.pow(it,p) on all values of this array. p may be a number or
another array of the same size as this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.product


	ma = ma:product(mb)
	ma = ma:product(mb,r)

Look at the type and call the appropriate product function, to produce 

	mb x ma
	
Note the right to left application and default returning of the 
leftmost term for chaining. This seems to make the most sense.

If r is provided then the result is written into r and returned 
otherwise ma is modified and returned.



## lua.wetgenes.tardis.array.quantize


	r=it:quantize(1/1024,r)
	r=it:quantize(s,it.new())

Perform a trunc(v/s)*s on all values of this array. We recomended the 
use of a power of two, eg 1/1024 rather than 1/1000 if you wanted 3 
decimal digits.

If r is provided then the result is written into r and returned 
otherwise it is modified and returned.



## lua.wetgenes.tardis.array.round


	r=it:round(r)
	r=it:round(it.new())

Perform math.floor( v+0.5 ) on all values of this array. Which will 
round up or down to the nearest integer.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.scalar


	r=a:scalar(s,r)

Multiply all value in array by number.

If r is provided then the result is written into r and returned otherwise a is
modified and returned.



## lua.wetgenes.tardis.array.set


	a=a:set(1,2,3,4)
	a=a:set({1,2,3,4})
	a=a:set({1,2},{3,4})
	a=a:set(function(i) return i end)

Assign some numbers to an array, all the above examples will assign 1,2,3,4 to
the first four slots in the given array, as you can see we allow one level of
tables. Any class that is based on this array class can be used instead of an
explicit table. So we can use a v2 or v3 or m4 etc etc.

if more numbers are given than the size of the array then they will be 
ignored.

if less numbers are given than the size of the array then the last number will
be repeated.

if no numbers are given then nothing will be done

if a function is given it will be called with the index and should 
return a number.



## lua.wetgenes.tardis.array.sin


	r=it:sin(r)
	r=it:sin(it.new())

Perform math.sin on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.sub


	r=it:sub(b,r)
	r=it:sub(b,it.new())

Subtract b from it. b may be a number or another array of the same size as this
array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.tan


	r=it:tan(r)
	r=it:tan(it.new())

Perform math.tan on all values of this array.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.trunc


	r=it:trunc(r)
	r=it:trunc(it.new())

Perform math.floor on positive values and math ceil on negative values 
for all values of this array. So a trunication that will always error 
towards 0.

If r is provided then the result is written into r and returned otherwise it is
modified and returned.



## lua.wetgenes.tardis.array.zero


	a=a:zero()

Set all values in this array to zero.



## lua.wetgenes.tardis.class


	metatable=tardis.class(name,class,...)

Create a new metatable for an object class, optionally inheriting from other previously 
declared classes.



## lua.wetgenes.tardis.line


A 3d space line class.

[1]position , [2]normal

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.line.new


	line = tardis.line.new(p,n)

Create a new line and optionally set it to the given values.



## lua.wetgenes.tardis.plane


A 3d space plane class.

[1]position , [2]normal

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.plane.new


	plane = tardis.plane.new(p,n)

Create a new plane and optionally set it to the given values.



## lua.wetgenes.tardis.smoothstep


	f = tardis.step(edge1,edge2,num)

return 0 if num is bellow or equal to edge1. Return 1 if num is the same or
higher as edge2 and smoothly interpolate between 0 and 1 for all other values.



## lua.wetgenes.tardis.step


	i = tardis.step(edge,num)

return 0 if num is bellow edge or 1 if num is the same or higher



## lua.wetgenes.tardis.type


	name=tardis.type(object)

This will return the type of an object previously registered with class



## lua.wetgenes.tasks


	local tasks=require("wetgenes.tasks").create()

Manage tasks that should be performed on seperate threads so as not to block the main thread.

Manage coroutines that can then easily call into these threads by yeilding



## lua.wetgenes.tasks.add_global_thread


	tasks:add_global_thread(thread)

This runs an add_thread inside a named thread so all threads are kept 
together no matter which thread tried to start it.

thread is the same as add_thread

if the thread id already exists then it will not be added again.



## lua.wetgenes.tasks.add_id


	tasks:add_id(it)

Internal function to manage creation of all objects with unique ids.



## lua.wetgenes.tasks.add_memo


	local memo=tasks:add_memo({})
	
Create a memo with a unique auto generated numerical id for linda 
comunication.



## lua.wetgenes.tasks.add_task


	local thread=tasks:add_task({
		id="test",
		code=function(linda,task_id,task_idx,task)
			while true do
				local _,memo= linda:receive( 0 , task_id )
				if memo then
					...
				end
			end
		end,
	})
	
Create a task with various preset values similar to a thread except 
inside a coroutine on the calling thread. As this function is inside a 
coroutine you must yield regulary this yield will then continue on the 
next update. Probably called once ever 60th of a second.

	id

A unique id string to be used by lindas when sending messages into this 
task. The function is expected to sit in an infinite loop testing 
this linda socket and then yielding if there is nothing to do.

	count

The number of tasks to create, they will all use the same instanced 
code function so should be interchangable and it should not matter 
which task we are actually running code on. If you expect the task to 
maintain some state between memos, then this must be 1 .

	code

A lua function to run inside a coroutine, this function will recieve 
tasks.linda (which is a colinda) and the task.id for comunication and 
an index so we know which of the count tasks we are (mostly for 
debugging) and finally the task table itself which make sense to share 
with coroutines.



## lua.wetgenes.tasks.add_thread


	local thread=tasks:add_thread({
		id="test",
		count=1,
		code=function(linda,task_id,task_idx)
			while true do
				local _,memo= linda:receive( nil , task_id )
				if memo then
					...
				end
			end
		end,
	})
	
Create a thread with various preset values:

	id

A unique id string to be used by lindas when sending messages into this 
task. The function is expected to sit in an infinite loop waiting on 
this linda socket.

	count

The number of threads to create, they will all use the same instanced 
code function so should be interchangable and it should not matter 
which thread we are actually running code on. If you expect the task to 
maintain some state between memos, then this must be 1 .

	code

A lua function to run inside each thread, this function will recieve 
tasks.linda and the task.id for comunication and an index so we know 
which of the count threads we are (mostly for debugging)




## lua.wetgenes.tasks.claim_global


	tasks:claim_global(name,value)

Claim a name using the global linda socket, returns value on success 
or false on failure.

The value will be associated with the name on success.

This will block waiting on a result but should be fast.



## lua.wetgenes.tasks.client


Send and/or recieve a (web)socket client memo result.

returns nil,error if something went wrong or returns result if 
something went right.



## lua.wetgenes.tasks.client_code


A basic function to handle (web)socket client connection.



## lua.wetgenes.tasks.cocall


	require("wetgenes.tasks").cocall(f1,f2,...)
	require("wetgenes.tasks").cocall({f1,f2,...})

Manage simple coroutines that can poll each others results and wait on 
them.

Turn a table of "setup" functions into a table of coroutines that can 
yield waiting for other coroutines to complete and run them all.

You still need to be carefull with race conditions but it allows you to 
write code in such away that setup order is no longer important. Setup 
functions can coroutine.yield waiting for another setup to finish first.



## lua.wetgenes.tasks.create


Create a tasks group to contain all associated threads and coroutines 
along with their comunications.



## lua.wetgenes.tasks.create_colinda


	local colinda=require("wetgenes.tasks").colinda(linda)
	
Create a colinda which is a wrapper around a linda providing 
replacement functions to be used inside a coroutine so it will yield 
(and assume it will be resumed) rather than wait.

This should be a dropin replacement for a linda and will fallback to 
normal linda use if not in a coroutine.

If linda is nil then we will create one, the linda used in this colinda 
can be found in colinda.linda if you need raw access.

What we are doing here is wrapping the send/receive functions so that

	colinda:send(time,...)
	colinda:recieve(time,...)

will be replaced with functions that call

	linda:send(0,...)
	linda:recieve(0,...)

and use coroutines.yield to mimic the original timeout value without 
blocking.



## lua.wetgenes.tasks.del_global_thread


	tasks:del_global_thread({id="threadname"})

Destroy a given thread.id or all the threads if thread is nil.



## lua.wetgenes.tasks.del_id


	tasks:del_id(it)

Internal function to manage deletion of all objects with unique ids.



## lua.wetgenes.tasks.del_memo


	tasks:del_memo(memo)
	
Delete a memo.



## lua.wetgenes.tasks.del_task


	tasks:del_task(task)
	
Delete a task.



## lua.wetgenes.tasks.del_thread


	tasks:del_thread(thread)
	
Delete a thread.



## lua.wetgenes.tasks.delete


	tasks:delete()
	
Force stop all threads and delete all data.

Failure to call this will allow any created threads to continue to run 
until program termination.



## lua.wetgenes.tasks.do_memo


	result = tasks:do_memo(memo,timeout)
	result = tasks:do_memo(memo)

Similar to calling tasks:receive but without the problems that come 
from me trying to remember how to spell receive and it returns 
memo.result instead of memo so slightly less mess. This will assert on 
finding a memo.error so less need to check for errors.



## lua.wetgenes.tasks.eject_global


	tasks:eject_global(name)

Eject a name using the global linda socket, returns value associated 
with name on success or false on failure.

The value will no longer be associated with the name when this succeeds.

This will block waiting on a result but should be fast.



## lua.wetgenes.tasks.fetch_name


	tasks:fetch_global(name)

Fetch value associated with the name using the global linda socket or 
false on failure. You can not associated nil or false with a global value.

This will block waiting on a result but should be fast.



## lua.wetgenes.tasks.global_code


A basic function to handle global memos to get/set data shared amongst multiple tasks.



## lua.wetgenes.tasks.http


Create send and return a http memo result.

Returns either the result or nil,error so can be used simply with an 
assert wrapper.



## lua.wetgenes.tasks.http_code


A basic function to handle http memos.



## lua.wetgenes.tasks.receive


	memo = tasks:receive(memo,timeout)
	memo = tasks:receive(memo)
	
Recieve a memo with optional timeout.

This is intended to be run from within a coroutine task on the main 
thread. It will work outside of a task but that will block the main 
thread waiting for a response.

The memo will be deleted after being recieved (ie we will have called 
del_memo) so as to free up its comunication id for another memo.

if the memo has not yet been sent or even been through add_memo (we 
check state for "setup" or nil) then it will be autosent with the same 
timeout before we try and receive it.

After calling check if memo.error is nil then you will find the result in 
memo.result



## lua.wetgenes.tasks.run_task


	tasks:run_task(task)
	
Resume all the coroutines in this task.

Any errors will be logged with a backtrace.

If the tasks have finished running (returned or crashed) then we will 
tasks:del_task(task) this task. Check task.id which will be nil after 
this task has finished.



## lua.wetgenes.tasks.send


	memo = tasks:send(memo,timeout)
	memo = tasks:send(memo)
	
Send a memo with optional timeout.

This is intended to be run from within a coroutine task on the main 
thread. It will work outside of a task but that may block the main 
thread waiting to send.

if memo.id is not set then we will auto call add_memo to create it.

Check memo.error for posible error, this will be nil if everything went 
OK.



## lua.wetgenes.tasks.sqlite


Create send and return a sqlite memo result.

Returns either the result.rows or nil,error so can be used simply with an 
assert wrapper.

Note that rows can be empty so an additional assert(rows[1]) might be 
needed to check you have data returned.



## lua.wetgenes.tasks.sqlite_code


A basic function to handle sqlite memos.

As we are opening an sqlite database here it wont help much to have 
more than one thread per database as they will just fight over file 
access.



## lua.wetgenes.tasks.thread_code


Handle global tasks, starting and stopping and preventing the starting 
of multiple copies of the same task.



## lua.wetgenes.tasks.update


	tasks:update()
	
Resume all current coroutines and wait for them to yield.



## lua.wetgenes.tasks_msgp


	local msgp=require("wetgenes.tasks_msgp")

A stream of lowlevel udp packets with automatic resend ( data will get 
there eventual ) but hopefully without clogging up an ongoing pulse of 
smaller packets that contain EG controller state.

We use lua sockets and lanes with some help from wetgenes.tasks to 
manage the lanes.


Simple UDP data packets with auto resend, little endian with this 6 byte header.

	u16		idx			//	incrementing and wrapping idx of this packet
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated

A maximum packet size of 63k seems to give good throughput, letting the 
lower level code split and recombine packets is probably going to be 
more efficient than us. Note that we care about best case speed, when 
shit lags and packets drop we care less about performance and more 
about maintaining a connection with correct state. IE its important 
that we recover not that we pretend there is no problem. At 255 bits * 
63k bit size we can seen a data of about 15.6m.

Technically 63k UDP packets might just get dropped as too big but I 
suspect modern hardware is not really going to cause problems.

bit/bits only goes up to 255. the 0 value in either of these is 
reserved as a flag for possible slightly strange packets in the future 
eg maybe we need a ping? and should be ignored as a bad packet if you 
do not understand them.

An ack of 0x1234 also implies that 0x9234 to 0x1233 are in the past and 
0x1234 to 0x9233 are in the future.

Also if we have nothing new to say after 100ms but have received new 
packets then we can send a packet with empty data (0 length) as an ack
only packet.

When connecting to a port for the first time, the idx value must start 
at a special number, that number is configurable and should be 
different for each app. An extra bit of sanity during introductions to 
indicate that the data stream will be understood by both parties.

Special packets that do not add to the stream of user data but instead 
are used internally by this protocol.

	id		bits	bit
	PING	0x00	0x02
	PONG	0x00	0x03
	HAND	0x00	0x04
	SHAKE	0x00	0x05
	RESEND	0x00	0x08
	PULSE	0x00	0x10

A PING packet will be accepted and acknowledged after handshaking. 
Sending a PING packet will cause a PONG response with the same data.

A PONG packet will be accepted and acknowledged after handshaking, its 
data should be the same as the PING it is responding too.

A HAND packet begins handshaking, the payload data is a string array. 
Consisting of a series of null terminated utf8 strings.

A SHAKE packet ends handshaking The payload data is a string array. 
Consisting of a series of null terminated utf8 strings.

Both the HAND and the SHAKE packets contain the same payload data which 
consists of the following utf8 strings, each terminated by a 0 byte. 

	host name
	host ip4
	host ip6
	host port
	client addr

When received, each of the strings should to be clamped to 255 bytes 
and the values validated or replaced with empty strings before use.

host name is maybe best considered a random string, it could be anything.

host ip4 will be the hosts best guess at their ip4, it is the ip4 they 
are listening on.

host ip6 will be the hosts best guess at their ip6, it is the ip6 they 
are listening on.

host port will be the local port the host is listening on, ip4 and ip6, 
but as they may be port forwarding we might connect on a different 
port.

client addr is the client ip and port the sender sent this packet too. 
If ipv4 it will be a string of the format "1.2.3.4:5" and if 1pv6 then 
"[1::2]:3" So the ip possibly wrapped in square brackets (ipv6) and 
then a colon followed by the port in url style format.

A RESEND packet consists of a payload of little endian 16bit idxs to 
packets we would like to be resent to fill in missing data.

A PULSE packet contains user data but does not get resent or 
acknowledged, its IDX must be set to 0 and this IDX should be ignored 
when received as this is out of stream data. Pulse packets are small 
regular packets of user data, eg current client input state. Data sent 
is included as part of the normal received data stream but there is no 
guarantee that it will be delivered or when it will be delivered 
relative to other data. The data must be small enough to fit in a 
single packet. Think of this as something of a raw UDP packet in terms 
of how it works.



## lua.wetgenes.tasks_msgp.addr_to_ip_port


parse an ip string and port number from an addr/addr+port/addr_list

returns addr,port that we can then use with luasocket



## lua.wetgenes.tasks_msgp.addr_to_list


parse an ip address + maybe port encoded as a string into a list of 
numbers, the length of the list represents the type so

	#4 {1,2,3,4} -- ip4
	#5 {1,2,3,4,5} -- ip4:port
	#8 {1,2,3,4,5,6,7,8} -- ip6
	#9 {1,2,3,4,5,6,7,8,9} -- [ip6]:port

Optionally include a numeric port to add/replace in the list after 
parsing.



## lua.wetgenes.tasks_msgp.clean_name


Clean a hostname so it becomes upercase letters and numbers with 
possible underscores where any other chars would be, eg whitespace.

we also try not to start or end with an _



## lua.wetgenes.tasks_msgp.ipsniff


Use ( google by default ) public dns servers to check if we can connect 
to the internet and if we have ipv4 and/or ipv6 available.

To use alternative dns pass in an ipv4 and ipv6 address as the first 
two args, eg to use cloudflare.

	ipsniff("1.1.1.1","2606:4700:4700::1111")

returns the ipv4,ipv6 address of this host when connecting to that dns 
or nil if no connection possible.

This result can be used as a bool to indicate working ipv6 or ipv4 
internet and is also a best guess as to our ip when connecting to other 
devices.

Note that this will only work if we are connected to the internet can 
reach the dns servers etc etc, standard networks can be crazy 
disclaimer applies.

So great if this works but still need a fallback plan, eg assuming 
local ipv4 network is available.



## lua.wetgenes.tasks_msgp.list_to_addr


Parse an array of numbers into an ip address and maybe port.

	#4 {1,2,3,4} -- ip4
	#5 {1,2,3,4,5} -- ip4:port
	#8 {1,2,3,4,5,6,7,8} -- ip6
	#9 {1,2,3,4,5,6,7,8,9} -- [ip6]:port



## lua.wetgenes.tasks_msgp.msgp_code


lanes task function for handling msgp communication.



## lua.wetgenes.tasks_msgp.pack


If given aa udp data packet string, convert it to a table.

If given table, convert it to a udp data packet string. little endian

	u16		idx			//	incrementing and wrapping idx of this packet
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated



## lua.wetgenes.tsv


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

	local wtsv = require("wetgenes.tsv")

Load and save tsv files https://pypi.org/project/linear-tsv/1.0.0/

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.
	
When loading we read the entire file and keep all the text in one string 
with function lookups to cut out sections of that string as needed.



## lua.wetgenes.tsv.close


get or set a line of data as a table of strings



## lua.wetgenes.tsv.create


	wtsv.create()
	wtsv.create({filename="filename.tsv"})
	wtsv.create({basedata="1\t2\t3\n4\t5\t6\n"})

Create a tsv, possibly from a datachunk or file



## lua.wetgenes.tsv.flush


save recent changes to disk, appended to loaded file



## lua.wetgenes.tsv.load


save a tsv file to tsv.filename
