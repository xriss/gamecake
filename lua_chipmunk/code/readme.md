

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




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
