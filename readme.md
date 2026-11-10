this is an orphaned branch of binaries that often changes

you may not rely on it

you may not trust it

but it probably works



---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## draw


	draw()

Draw called every frame, there may be any number of updates between 
each draw but hopefully we are looking at one update followed by a 
draw, if you have an exceptionally fast computer then we may even get 0 
updates between some draws.



## entities



	entities.reset()
	
empty the list of entites to update and draw


	entities.caste(caste)

get the list of entities of a given caste, eg "bullets" or "enemies"


	entities.add(it,caste)
	entities.add(it)

add a new entity of caste or it.caste if caste it missing to the list 
of things to update 


	entities.remove(it)

Remove an entity from its caste table.


	entities.call(fname,...)

for every entity call the function named fname like so it[fname](it,...)


	entities.count(caste)

Return the count of the number of entities in a given caste.


	entities.get(name)

get a value previously saved, this is an easy way to find a unique 
entity, eg the global space but it can be used to save any values you 
wish not just to bookmark unique entities.


	entities.set(name,value)

save a value by a unique name


	entities.manifest(name,value)

get a value previously saved, or initalize it to the given value if it 
does not already exist. The default value is {} as this is intended for 
lists.




## entities.systems


A global table for entity systems to live in.

	entities.systems_call(fname,...)
	
Call the named function on any systems that currently exist. For 
instance entities.systems_call("load") is used at the bottom 
of this file to prepare graphics of registered systems.



## entities.systems.bang


a bang



## entities.systems.donut


	donut = entities.systems.donut.add(opts)

Add an donut.



## entities.systems.horde


The invading horde



## entities.systems.invader


an invader



## entities.systems.item


	item = entities.systems.item.add()

items, can be used for general things, EG physics shapes with no special actions



## entities.systems.level


	entities.systems.level.setup(level)

reset and setup everything for this level idx



## entities.systems.menu


	menu = entities.systems.menu.setup()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.




## entities.systems.missile


a missile



## entities.systems.npc


	npc = entities.systems.npc.add(opts)

Add an npc.



## entities.systems.player


	player = entities.systems.player.add(idx)

Add a player



## entities.systems.score


The score



## entities.systems.space


	space = entities.systems.space.setup()

Create the space that simulates all of the physics.



## entities.systems.stars


The stars



## entities.systems.tile


setup background tile graphics



## entities.systems.yarn


Handle the main yarn setup update and draw.



## entities.tiles.npc


Display a npc



## entities.tiles.sprite


Display a sprite



## entities.tiles.start


The player start point, just save the x,y



## graphics



define all graphics in this global, we will convert and upload to tiles 
at setup although you can change tiles during a game, we try and only 
upload graphics during initial setup so we have a nice looking sprite 
sheet to be edited by artists



## hardware


select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.

This also provides a default main function that will upload the 
graphics and call the provided update/draw callbacks.



## levels


Design levels here



## lua.glslang


Manage string embedding and the glsl compiler and language options.

This is somewhat separate to the main GLES code and can be used without 
linking to any system dependent GL libs.



## lua.glslang.lint_gles2


	verr,ferr = glslang.lint_gles2(vcode,fcode)

Try and compile a vertex/fragment shader pair and return any errors.

returns nil,nil on success or an error string if either shader fails to 
compile.

Note that the glslang compile step seems rather slow, not sure what it 
gets up to but do not consider using this if speed is important.



## lua.glslang.parse_chunks


	shaders=glslang.shader_chunks(text,filename,headers,flags)

		load multiple shader chunks from a single file and return a lookup
		table of name=code for each shader we just loaded. These can then be
		compiled or whatever.
		
		set flags.headers_only to true if you only care about parsing headers
		for later inclusion and do not want to parse shader chunks.
		

	#SHADER "nameofshader"

		is used at the start of a line to say which chunk the following text
		should go into. This is technically the same as #header but makes it
		explicit that this should be all the code for a shader rather than just
		a piece.

	#HEADER "nameofheader"

		can be used to define part of a shader, which can later be included in
		another shader by using

	#INCLUDE "nameofheader"
	
		Will insert the chunk of text previously defined as a #SHADER or #HEADER

	#SHADER or #HEADER

		without a name can be used to ignore the next part of a file, any text
		at the start of a file before the first #SHADER or #HEADER is also
		ignored. This enables these chunks to exist inside comments in a lua or
		whatever file.



## lua.glslang.pp


	code,err = glslang.pp(code,err)

Preprocess a vertex/fragment and return the result.

returns nil,error on error



## lua.glslang.replace_include


process #include statements in src with strings found in the headers table



## lua.glslang.yank_shader_versions


parse a shader source and yank any #version out of the source and into a table
this allows multiple attempts at compiling the same source using different #versions



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



## lua.wetgenes.gamecake.framebuffers


Deal with FrameBuffers as render targets and textures. Color and Depth 
buffers need to be allocated and managed.

So we need to be baked within an oven so we will talk about the return 
from the bake function rather than the module function.

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local fbo=framebuffers.create(256,256,0)	-- a 256x256 texture only
	local fbo=framebuffers.create(256,256,-1)	-- a 256x256 depth only
	local fbo=framebuffers.create(256,256,1)	-- a 256x256 texture and depth
	local fbo=framebuffers.create()				-- 0x0 and we will resize later



## lua.wetgenes.gamecake.framebuffers.create


	local fbo=framebuffers.dirty()
	local fbo=framebuffers.dirty(x,y)
	local fbo=framebuffers.dirty(x,y,framebuffers.NEED_TEXTURE_AND_DEPTH)
	local fbo=framebuffers.dirty(0,0,0,{
		depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT},
		texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE},
		TEXTURE_MIN_FILTER=gl.LINEAR,
		TEXTURE_MAG_FILTER=gl.LINEAR,
		TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE,
		TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE,
	}) -- note this table will be returned as the fbo

Create a new framebuffer object and optionally provide an inital size 
and depth. The depth can use -1,0,1 or the following verbose flags 
framebuffers.NEED_DEPTH,framebuffers.NEED_TEXTURE or 
framebuffers.NEED_TEXTURE_AND_DEPTH to request a depth buffer(1,-1) or not(0). 

Finally you can pass in a table to be returned as the fbo that contains 
defaults or set defaults in the fbo that is returned.

	fbo.depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT}

Can be used to control exactly how a depth buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE}

Can be used to control exactly how a texture buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.TEXTURE_MIN_FILTER=gl.LINEAR
	fbo.TEXTURE_MAG_FILTER=gl.LINEAR
	fbo.TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE
	fbo.TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE

These can be used to control default TexParameters in the fbo.

	fbo.no_uptwopow=true
	
By deafult we will use power of 2 sizes for the fbo that fit the 
requested size. This disables that and doing so will of course cause 
problems with some hardware. Generally if you avoid mipmaps it probably 
wont be a problem.

See #lua.wetgenes.gamecake.framebuffers.fbo for all the functions you 
can call on the fbo returned.



## lua.wetgenes.gamecake.framebuffers.dirty


	framebuffers.dirty()

Mark all framebuffer objects as dirty by setting fbo.dirty to be true. We do not 
do anything with this flag but it is used in external code and this is 
a useful helper function to set the flag.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_depth


	fbo:bind_depth()

BindTexture the depth texture part of this fbo.

If there is no texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_depth_snapshot


	fbo:bind_depth_snapshot()

BindTexture the depth snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_frame


	fbo:bind_depth()

BindFramebuffer the framebuffer of this fbo.

If there is no framebuffer we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_texture


	fbo:bind_texture()

BindTexture the rgba texture part of this fbo.

If there is no texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.bind_texture_snapshot


	fbo:bind_texture_snapshot()

BindTexture the rgba snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.



## lua.wetgenes.gamecake.framebuffers.fbo.check


	fbo:check()

Check and allocatie if missing and needed (eg depth texture may not be 
needed) all our openGL buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.clean


	fbo:clean()

Free all the opengl buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.download


	fbo:download()
	fbo:download(w,h,x,y)

Read back color data from a framebuffer and return it in an RGBA 
PREMULT grd object (which is probably what it is).

If a width,height is given then we will read the given pixels only from 
the x,y location.



## lua.wetgenes.gamecake.framebuffers.fbo.free_depth


	fbo:free_depth()

Free the depth texture only, is safe to call if there is no depth 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.free_frame


	fbo:free_frame()

Free the frame buffer only, is safe to call if there is no frame 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.free_snapshot


	fbo:free_snapshot()

Free the snapshot buffers, fbo.texture_snapshot and fbo.depth_snapshot 
if they exist.



## lua.wetgenes.gamecake.framebuffers.fbo.free_texture


	fbo:free_texture()

Free the rgba texture only, is safe to call if there is no rgba 
buffer.



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap


	fbo:mipmap()

Build mipmaps for all existing texture buffers.



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap_depth


	fbo:mipmap_depth()

Build mipmaps for our depth buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.

It is possible this may fail (hardware issues) and the 
TEXTURE_MIN_FILTER be reset to gl.NEAREST along with a flag to stop us 
even trying in the future,



## lua.wetgenes.gamecake.framebuffers.fbo.mipmap_texture


	fbo:mipmap_texture()

Build mipmaps for our texture buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.



## lua.wetgenes.gamecake.framebuffers.fbo.pingpong


	fbo:pingpong(fbout,shadername,callback)
	framebuffers.pingpong({fbin1,fbin2},fbout,shadername,callback)

Render from one or more fbos into another using a fullscreen shader.

Sometime you need to repeatedly copy a texture back and though applying 
a shader, this is the function for you.

The textures will be bound to tex1,tex2,tex3,etc and the uvs supplied 
in a_texcoord with a_vertex being set to screen coords so can be used 
as is.



## lua.wetgenes.gamecake.framebuffers.fbo.render_start


	fbo:render_start()


Start rendering into this fbo.

Push old matrix and set the matrix mode to MODELVIEW

Set fbo.view and reset the gl state.



## lua.wetgenes.gamecake.framebuffers.fbo.render_stop


	fbo:render_stop()

Stop rendering into this fbo and restore the last fbo so these calls 
can be nested.

Restore the old view and old gl state.

Pop old matrix and set the matrix mode to MODELVIEW



## lua.wetgenes.gamecake.framebuffers.fbo.resize


	fbo:resize(w,h,d)

Change the size of our buffers, which probably means free and then
reallocate them.

The w,h,d use the same rules as framebuffer.create



## lua.wetgenes.gamecake.framebuffers.fbo.snapshot


	fbo:snapshot()

Take a current snapshot copy of the texture and depth if they exist, 
store them in fbo.texture_snapshot and fbo.depth_snapshot for later 
binding.



## lua.wetgenes.gamecake.framebuffers.start


	framebuffers.start()

Called as part of the android life cycle, since we auto reallocate this 
does not actually have to do anything. But may do a forced resize in 
the future if that turns out to be more hardware compatible.



## lua.wetgenes.gamecake.framebuffers.stop


	framebuffers.stop()

Called as part of the android life cycle, we go through and call 
fbo.clean on each fbo to free the opengl resources.



## lua.wetgenes.gamecake.fun.chatdown.chat.get_tag


	tag_value = chat:get_tag(tag_name)

The same as chats:get_tag but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.replace_tags


	output = chat:replace_tags(input)

The same as chats:replace_tags but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_tag


	tag_value = chat:set_tag(tag_name,tag_value)

The same as chats:set_tag but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_tags


	chat:set_tags(tags)

Set all the values in the given table of {tag_name=tag_value} pairs.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_topic


	chat:set_topic(topic_name)

Set the current topic for this chat object, information about this 
topic and its gotos are built from and stored in this chat object.

	chat.topic_name
	
Will be set to the given topic name.

We merge all gotos found in this topic and all topic parents by 
iterating over the dotnames. We only use add each topic once and each 
topic may have a bit of logic that decides if it should be displayed.

	<topic_name?logic_test

So this would goto the topic_name if the logic_test passes. The logic 
test is written simmilar to a url query after the question mark comes a 
number of tests that are then & or | together in left to right order 
(no operator precedence).

	<topic_name?count<10&seen==false

So this would display the goto option if count is less than 10 and seen 
is set to false. These variables we are testing are tag_names and 
default to the current subject chat but could reference other subjects 
by explicitly including a root.

Available logic tests are

	name==value
	name=value

True if the tag is set to this value.

	name!=value

True if the tag is not set to this value.

	name<value

True if the tag is less than this value (numeric test).

	name>value

True if the tag is more than this value (numeric test).

	name<=value

True if the tag is less than or equal to this value (numeric test).

	name>=value

True if the tag is more than or equal to  this value (numeric test).

All of these tests can be preceded by a ! to negate them so

	name!=vale
	!name==value

Are both a test for inequality.

Multiple tests can be joined together by & (and) or | (or) this logic 
will be applied to a running total moving from left to right as the 
values are evaluated with the final value deciding if this goto will be 
displayed.




## lua.wetgenes.gamecake.fun.chatdown.chats.changes


	chats.changes(chat,change,...)

	chats.changes(chat,"subject")
	chats.changes(chat,"topic",topic)
	chats.changes(chat,"goto",goto)
	chats.changes(chat,"tag",tag_name,tag_value)

This is a callback hook, replace to be notified of changes and possibly 
alter then, by default we print debuging information. Replace this 
function with an empty function to prevent this eg

	chats.changes=function()end



## lua.wetgenes.gamecake.fun.chatdown.chats.get_subject


	chat = chats:get_subject(subject_name)
	chat = chats:get_subject()

Get the chat for the given subject or the chat for the last subject 
selected with set_subject if no subject_name is given.



## lua.wetgenes.gamecake.fun.chatdown.chats.get_tag


	tag_value = chats:get_tag(tag_name,subject_name)

Get the tag_value for the given tag_name which can either be 
"tag_root/tag_name" or "tag_name". The subject_name is the default root 
to use if no tag_root is given in the tag_name.



## lua.wetgenes.gamecake.fun.chatdown.chats.replace_tags


	output = chats:replace_tags(input,subject_name)

Tags in the input text can be wrapped in {tag_name} and will be 
replaced with the appropriate tag_value. This is done recursively so 
tag_values can contain references to other tags. If a tag does not 
exist then it will not expand and {tag_name} will remain in the output 
text.

Again if any tag name does not contain an explicit root then 
subject_name will be used as the default chat subject.



## lua.wetgenes.gamecake.fun.chatdown.chats.set_subject


	chat = chats:set_subject(subject_name)

Set the current subject for this chats object, this subject becomes the 
chat that you will get if you call get_subject with no arguments.




## lua.wetgenes.gamecake.fun.chatdown.chats.set_tag


	tag_value = chats:set_tag(tag_name,tag_value,subject_name)

Alter the value of the given tag_name. If the value string begins with 
a "+" or a "-" Then the values will be treated as numbers and added or 
subtracted from the current value. This allows for simple incremental 
flag values.

Again if the tag name does not contain an explicit  root then 
subject_name will be used as the default chat subject.



## lua.wetgenes.gamecake.fun.chatdown.dotnames


	for part_name in chatdown.dotnames(full_name) do
		print(part_name)
	end

Iterate all dotnames so if given "aa.bb.cc" we would iterate through 
"aa.bb.cc" , "aa.bb" and "aa". This is used to inherit data using just 
a naming convention.



## lua.wetgenes.gamecake.fun.chatdown.parse


	rawsubjects = chatdown.parse(text)

Parse text from flat text chatdown format into heirachical chat data, 
which we refer to as rawsubjects, something that can be output easily 
as json.

This gives us a readonly rawsubjects structure that can be used to control 
what text is displayed during a chat session.

This is intended to be descriptive and logic less, any real logic 
should be added using a real language that operates on this rawsubjects and 
gets triggered by the names used. EG, filter out gotos unless certain 
complicated conditions are met or change topics to redirect to an 
alternative.

A self documented example of chatdown formated text can be found in 
lua.wetgenes.gamecake.fun.chatdown.text



## lua.wetgenes.gamecake.fun.chatdown.setup


	chats = chatdown.setup_chats(chat_text,changes)

parse and initialise state data for every subjects chunk creating a 
global chats with a chat object for each subject.



## lua.wetgenes.gamecake.fun.chatdown.setup_chat


	chat = chatdown.setup_chat(chats,subject_name)

Setup the initial chat state for a subject. This is called 
automatically by chatdown.setup and probably should not be used 
elsewhere.



## lua.wetgenes.gamecake.fun.chatdown.text


Here is some example chatdown formatted text full of useful 
information, it it is intended to be a self documented example.

```

- This is a single line comment
-- This is the start of a multi-line comment

All lines are now comment lines until we see a line that begins with a 
control character leading white space is ignored. If for some reason 
you need to start a text line with a special character then it can be 
escaped by preceding it with a #

What follows is a list of these characters and a brief description 
about the parser state they switch to.

	1. - (text to ignore)
	
		A single line comment that does not change parser state and 
		only this line will be ignored so it can be inserted inside 
		other chunks without losing our place.

	2. -- (text to ignore)
	
		Begin a comment chunk, this line and all lines that follow this 
		line will be considered comments and ignored until we switch to 
		a new parser state.

	3. #subject_name

		Begin a new subject chunk, all future topic,goto or tag chunks will 
		belong to this subject.
		
		The text that follows this until the next chunk is the long 
		description intended for when you examine the character.
		
		Although it makes sense for a subject chunk to belong to one 
		character it could also a group conversation with tags being 
		set to change the current talker as the conversation flows.
		
		subject names have simple cascading inheritance according to their 
		name with each level separated by a dot. A chunk named a.b.c 
		will inherit data from any chunks defined for a.b and a in that 
		order of priority.

	4. >topic_name
	
		Begin a topic chunk, all future goto or tag chunks will belong 
		to this topic, the lines that follow are how the character 
		responds when questioned about this topic followed by one or 
		more gotos as possible responses that will lead you onto 
		another topic.
		
		Topics can be broken into parts, to create a pause, by using an 
		unnamed goto followed by an unnamed topic which will both 
		automatically be given the same generated name and hence linked 
		together.
		
	5. <goto_topic_name
	
		Begin a goto chunk, all future tag chunks will belong to this 
		goto, this is probably best thought of as a question that will 
		get a reply from the character. This is a choice made by the 
		player that causes a logical jump to another topic.
		
		Essentially this means GOTO another topic and there can be 
		multiple GOTO options associated with each topic which the 
		reader is expected to choose between.
		
	6. =set_tag_name to this value
	
		If there is any text on the rest of this line then it will be 
		assigned to the tag without changing the current parse state so 
		it can be used in the middle of another chunk without losing 
		our place.
		
		This single line tag assignment is usually all you need. 
		Alternatively, if there is no text on the rest of this first 
		line, only white space, then the parse state will change and 
		text on all following lines will be assigned to the named tag.
		
		This assignment can happen at various places, for instance if 
		it is part of the subject then it will be the starting 
		value for a tag but if it is linked to a topic or goto then the 
		value will be a change as the conversation happens. In all 
		cases the tags are set in a single batch as the state changes 
		so the placement and order makes no difference.
		
		Tags can be used inside text chunks or even GOTO labels by 
		tightly wrapping with {} eg {name} would be replaced with the 
		value of name. Tags from other subjects can be referenced by 
		prefixing the tag name with the subject name followed by a forward 
		slash like so {subject/tag}
				

The hierarchy of these chunks can be expressed by indentation as all 
white space is ignored and combined into a single space. Each SUBJECT will 
have multiple TOPICs associated with it and each TOPIC will have 
multiple GOTOs as options to jump between TOPICs. TAGs can be 
associated with any of these 3 levels and will be evaluated as the 
conversation flows through these points.

So the chunk hierarchy expressed using indentation to denote children 
of.

	SUBJECT
		TAG
		GOTO
			TAG
		TOPIC
			TAG
			GOTO
				TAG

The GOTO chunks in the root SUBJECT chunk are used as prototypes so if a 
GOTO is used in multiple topics its text can be placed within a GOTO 
inside the main SUBJECT chunk rather than repeated constantly. This will 
then be inherited by a GOTO with no text. An alternative to this 
shorthand is to assign an oft-used piece of text to a tag and reference 
that in each topic instead.

SUBJECTs and TOPICs also have simple inheritance based on their names this 
enables the building of a prototype which is later expanded. Each 
inheritance level is separated by a dot so aa.bb.cc will inherit from 
aa.bb and aa with the data in the longer names having precedence. This 
inheritance is additive only so for instance a TAG can not be unset and 
would have to be changed to another value by aa.bb.cc if it existed in 
aa.bb or aa.

In practise this means


```


## lua.wetgenes.gamecake.fun.yarn.cells


	cells = require("wetgenes.gamecake.fun.yarn.cells").create(items)

This module contains only one function which can be used to create an 
cells instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.items


	items = require("wetgenes.gamecake.fun.yarn.items").create()

This module contains only one function which can be used to create an 
items instance and the rest of this documentation concerns the return 
from this create function, not the module itself.



## lua.wetgenes.gamecake.fun.yarn.items.cells


	items.cells

We automatically create a cells object bound to this set of items, this 
cells object should be used to define all your custom game cells.



## lua.wetgenes.gamecake.fun.yarn.items.create


	item = items.create()
	item = items.create({})
	item = items.create({},metatable)

Create a single item, optionally pass in a base item table that will be 
turned into a proper item (using setmetatable to add methods). This 
should always be a new table and will also be returned. If no 
metatable is provided then items.metatable will be used.



## lua.wetgenes.gamecake.fun.yarn.items.create_pages


	items.pages

We automatically create a pages object bound to this set of items, this 
pages object should be used to define all your custom game pages.

This can be considered a level and you may need multiple pages which 
are moved in and out of items.pages



## lua.wetgenes.gamecake.fun.yarn.items.destroy


	item = items.destroy(item)
	item = item:destroy()

Destroy an item, remove it from the master table dump so it will be 
garbage collected.



## lua.wetgenes.gamecake.fun.yarn.items.find


	child_item = item:find(keyname)

Get the first child item that has a [keyname] value in it. All child 
items are searched, but this is not recursive.

returns nil if no child item is found.



## lua.wetgenes.gamecake.fun.yarn.items.get_big


	big_item = item:get_big()

Get the first big item from this container, returns nil if we do not 
contain a big item.



## lua.wetgenes.gamecake.fun.yarn.items.insert


	item = item:insert(parent)

Insert this item into the given parent. Item will automatically be 
removed from its current parent. If the item is_big then it will be 
inserted into the front of the list otherwise it will go at the end. 
This is to help with finding big items, since there should only be one 
per container we only have to check the first child.



## lua.wetgenes.gamecake.fun.yarn.items.iterate_dotnames


	for name,tail in items.iterate_dotnames(names) do ... end

Iterator over a names string, start with the full string and cut off 
the tail on each iteration. This is used for simple inheritance merging 
of named prefabs and rules or anything else.

Second return value is the tail of the string or the string if not 
tail.

for example the following input string
	
	"one.two.three.four"
	
would get you the following iteration loops, one line per loop

	"one.two.three.four" , "four"
	"one.two.three"      , "three"
	"one.two"            , "two"
	"one"                , "one"



## lua.wetgenes.gamecake.fun.yarn.items.iterate_parents


	for it in item:iterate_parents() do
		...
	end

Iterate over the parent chain going upwards. The first iteration is the 
parent of this object and so on.



## lua.wetgenes.gamecake.fun.yarn.items.metatable


	items.metatable

Expose the item metatable so more methods can easily be added.



## lua.wetgenes.gamecake.fun.yarn.items.prefabs


	items.prefabs

We automatically create a prefabs object bound to this set of items, this 
prefabs object should be used to define all your custom game prefabs.



## lua.wetgenes.gamecake.fun.yarn.items.remove


	item = item:remove()

Remove this item from its parents table or do nothing if the item does 
not have a parent.



## lua.wetgenes.gamecake.fun.yarn.items.rules


	items.rules

We automatically create a rules object bound to this set of items, this 
rules object should be used to define all your custom game rules.



## lua.wetgenes.gamecake.fun.yarn.pages


	pages = require("wetgenes.gamecake.fun.yarn.pages").create(items)

This module contains only one function which can be used to create an 
pages instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.prefabs


	prefabs = require("wetgenes.gamecake.fun.yarn.prefabs").create(items)

This module contains only one function which can be used to create an 
prefabs instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.prefabs.set


	prefab = prefabs:get(name)
	prefab = prefabs:get(name,prefab)

Build and return a table with all prefab values inherited from all its 
parents. Optionally pass in a prefab to override values and also have 
it returned.

If name doesn't exist at any level and a table is not passed in then nil 
will be returned.

Names are hierarchical, separated by dots, see items.iterate_dotnames



## lua.wetgenes.gamecake.fun.yarn.rules


	rules = require("wetgenes.gamecake.fun.yarn.rules").create(items)

This module contains only one function which can be used to create an 
rules instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.rules.apply


	item = rules.apply(item,method,...)

item.rules must be a list of rule names and the order in which they should 
be applied to this item.

Call the given method in each rule with the item and the remaining arguments.

If the method returns a value then no more methods will be applied even 
if more rules are listed.

We always return the passed in item so that calls can be chained.

	item = item:apply(method,...)

This function is inserted into the items.metatable so it can be called 
directly from an item.



## lua.wetgenes.gamecake.fun.yarn.rules.can


	yes = rules.can(item,method)

Returns true if any rule in this item has the given method.

	yes = item:can(method)

This function is inserted into the items.metatable so it can be called 
directly from an item.



## lua.wetgenes.gamecake.fun.yarn.rules.set


	rule = rules.set(rule)

Set this base rule into the name space using rule.name which must 
be a string.

Multiple rules can be applied to an item and each rule will be applied 
in the order listed.

A rule is a table of named functions that can be applied to an item.

	rule.setup(item)

Must setup the item so that it is safe to call the other rules on it.

	rule.clean(item)

Should cleanup anything that needs cleaning.

	rule.tick(item)

Should perform a single time tick update on the item.





## lua.wetgenes.gamecake.oven


	oven=require("wetgenes.gamecake.oven").bake(opts)

The oven module must be baked so only exposes a bake function.

All the other functions are returned from within the bake function.

possible ENV settings

	gamecake_tongue=english
	gamecake_flavour=sdl



## lua.wetgenes.gamecake.oven.bake



	oven=wetgenes.gamecake.oven.bake(opts)

Bake creates an instance of a lua module bound to a state. Here we 
are creating the main state that other modules will then bind to.

We call each state an OVEN to fit into the gamecake naming scheme 
then we bake a module in this oven to bind them to the common state.

Think of it as a sub version of require, so require gets the global 
pointer for a module and bake is used to get the a module bound to 
an oven.

By using this bound state we reduce the verbosity of connecting 
modules and sharing state between them.




## lua.wetgenes.gamecake.toaster


	oven=require("wetgenes.gamecake.toaster").bake(opts)

A cut down oven without opengl or even file access intended to be used 
in a sub process or task.



## lua.wetgenes.gamecake.toaster.bake


	oven=wetgenes.gamecake.toaster.bake(opts)

Bake creates an instance of a lua module bound to an oven state. Here 
we are creating the main state that other modules will then bind to.

Modules are then bound together using rebake...

	b=oven.rebake(nameb)
	c=oven.rebake(namec)

All of these will be connected by the same oven and circular 
dependencies should work with the caveat that just because you have the 
table for a baked module does not mean that it has all been filled in 
yet.



## lua.wetgenes.gamecake.toaster.newticks


	ticks=require("wetgenes.gamecake.toaster").newticks(rate,jump)

create a ticks time controller for updates.

rate is time between steps , jump is how far behind we should get 
before giving up ( eg callback takes longer than rate allows ) these 
values are in seconds and default to 1/60 and 1 respectively.

Then we perform one or more update steps like so

	ticks.step( callback ) -- callback may be called zero or more times

After that we should wait a bit (maybe draw a frame) and then call 
again.



## lua.wetgenes.gamecake.widgets


	local widgets=oven.rebake("wetgenes.gamecake.widgets")

A collection of widgets, rendered using gles2 code and controlled 
using the mouse, keyboard or a joystick. EG click fire and move 
left/right to adjust a slider value.

Widgets must be created and bound to an oven, using the 
oven.rebake function.

This has undergone a number of rewrites as we try to simplify the 
widget creation and layout process. Eventually we ended up with a 
fixed size system of widget placement so every widget must have a 
known size in advance, however we allow scaling to occur so for 
instance building a 256x256 widget does not mean that it has to be 
displayed at 256x256 it just means it will be square.

The basic layout just lets you place these widgets in other widgets 
as left to right lines. So as long as you get your sizes right you 
can easily place things just using a list and without keeping track 
of the current position.

Other layout options are available, such as absolute positioning for 
full control and we have simple custom skin versions of the buttons 
as well rather than the built in skins.

All value data is handled by data structures that contain ranges and 
resolutions for data allowing the same data to be bound to multiple 
display widgets. For instance the same data can be linked to the 
position of a slider as well as the content of a text display. I 
think the kids call this sort of thing an MVC pattern but that's a 
terribly dull name.

Swanky paint is probably the most advanced used of the widgets so far
but I suspect we will be making a simple text editor in the near 
future. Designed for advanced in game tweaking of text files.




## lua.wetgenes.gamecake.widgets.button


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="button",...}

A button for pressing and such.



## lua.wetgenes.gamecake.widgets.button.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

Also cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.



## lua.wetgenes.gamecake.widgets.button.update


	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this button then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?



## lua.wetgenes.gamecake.widgets.center


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="center",...}

A layout class for very centered children.



## lua.wetgenes.gamecake.widgets.center.layout


	this function will also call lua.wetgenes.gamecake.widgets.meta.layout

Place any children in the center of this widget. Multiple children will 
overlap so probably best to only have one child.



## lua.wetgenes.gamecake.widgets.center.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

If not explicetly set we will use a size of "full" ie the size of 
parent as that is probably how this class will always be used.



## lua.wetgenes.gamecake.widgets.checkbox


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="checkbox",...}

A button that can be used to display and toggle a single bit of data.



## lua.wetgenes.gamecake.widgets.checkbox.class_hooks


We catch and react to the click hook as we can toggle the data_mask bit 
in the data.



## lua.wetgenes.gamecake.widgets.checkbox.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options

As a button we always need to be solid so this is forced to true.

cursor will be set to "hand" if it is not already set so you can 
tell it is a button when you hover over it.

data_mask defaults to 1 and represents the bit (or bits) that should be 
tested and toggled in the data. The default of 1 and assuming your data 
starts at 0 means that the data will toggle between 0 and 1 using this 
checkbox.

text_false defaults to " " and is the text that will be displayed when 
a data_mask test is false.

text_true defaults to "X" and is the text that will be displayed when 
a data_mask test is true.



## lua.wetgenes.gamecake.widgets.checkbox.update


	this function will also call lua.wetgenes.gamecake.widgets.meta.update

If we have a data assigned to this checkbox then make sure that the 
displayed text is up to date. We should really hook into the data so 
any change there is copied here instead?

We use data_mask to check a single bit and then set the text to either 
text_false or text_true depending on the result.




## lua.wetgenes.gamecake.widgets.data


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local data=master.new_data{}

A number or string that can be shared between multiple widgets given 
basic limits and watched for changes.

This alows the same data to be linked and displayed in multiple widgets 
simultaneously.



## lua.wetgenes.gamecake.widgets.data.add_class_hook




## lua.wetgenes.gamecake.widgets.data.call_hook




## lua.wetgenes.gamecake.widgets.data.call_hook_later




## lua.wetgenes.gamecake.widgets.data.data_dec


adjust number (may trigger hook)



## lua.wetgenes.gamecake.widgets.data.data_get_pos


get display pos, given the size of the parent and our size?



## lua.wetgenes.gamecake.widgets.data.data_get_size


how wide or tall should the handle be given the size of the parent?



## lua.wetgenes.gamecake.widgets.data.data_inc


adjust number (may trigger hook)



## lua.wetgenes.gamecake.widgets.data.data_set


set a number value and min/max range probably without any triggers



## lua.wetgenes.gamecake.widgets.data.data_snap


given the parents size and our relative position/size within it update 
dat.num and return a new position (for snapping)



## lua.wetgenes.gamecake.widgets.data.data_tonumber


get a number from the string



## lua.wetgenes.gamecake.widgets.data.data_tonumber_from_list




## lua.wetgenes.gamecake.widgets.data.data_tostring


get a string from the number



## lua.wetgenes.gamecake.widgets.data.data_tostring_from_list




## lua.wetgenes.gamecake.widgets.data.data_value


set number (may trigger hook unless nohook is set)



## lua.wetgenes.gamecake.widgets.data.del_class_hook




## lua.wetgenes.gamecake.widgets.data.new_data




## lua.wetgenes.gamecake.widgets.datas


Handle a collection of data (IE in the master widget)



## lua.wetgenes.gamecake.widgets.datas.del




## lua.wetgenes.gamecake.widgets.datas.get




## lua.wetgenes.gamecake.widgets.datas.get_number




## lua.wetgenes.gamecake.widgets.datas.get_string




## lua.wetgenes.gamecake.widgets.datas.get_value




## lua.wetgenes.gamecake.widgets.datas.new




## lua.wetgenes.gamecake.widgets.datas.new_datas




## lua.wetgenes.gamecake.widgets.datas.set




## lua.wetgenes.gamecake.widgets.datas.set_infos




## lua.wetgenes.gamecake.widgets.datas.set_string




## lua.wetgenes.gamecake.widgets.datas.set_value




## lua.wetgenes.gamecake.widgets.defs


Helpers to define defaults for each class of widget 



## lua.wetgenes.gamecake.widgets.defs.add




## lua.wetgenes.gamecake.widgets.defs.add_border




## lua.wetgenes.gamecake.widgets.defs.copy




## lua.wetgenes.gamecake.widgets.defs.create




## lua.wetgenes.gamecake.widgets.defs.reset




## lua.wetgenes.gamecake.widgets.defs.set




## lua.wetgenes.gamecake.widgets.dialogs


handle a collection of dialogs that all live in the same place



## lua.wetgenes.gamecake.widgets.dialogs.hide_overlay




## lua.wetgenes.gamecake.widgets.dialogs.setup




## lua.wetgenes.gamecake.widgets.dialogs.show




## lua.wetgenes.gamecake.widgets.dialogs.show_overlay




## lua.wetgenes.gamecake.widgets.drag


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="drag",...}

A button to drag arround.



## lua.wetgenes.gamecake.widgets.drag.drag




## lua.wetgenes.gamecake.widgets.drag.setup




## lua.wetgenes.gamecake.widgets.drag.update




## lua.wetgenes.gamecake.widgets.paragraph


	local master=oven.rebake("wetgenes.gamecake.widgets").setup()
	local widget=master:add{class="paragraph",...}

A layout class for a paragraph of wordwrapped text that ignores 
quantity of whitespace and possibly child widgets insereted into the 
text and apropriate control points.

We will be the full width of our parent and as tall as we need to be to 
fit the given text.



## lua.wetgenes.gamecake.widgets.paragraph.setup


	see lua.wetgenes.gamecake.widgets.meta.setup for generic options



## lua.wetgenes.gamecake.widgets.setup


	master=oven.rebake("wetgenes.gamecake.widgets").setup()

	master=oven.rebake("wetgenes.gamecake.widgets").setup(
		{font="Vera",text_size=16,grid_size=32,skin=0} )

Create a master widget, this widget which is considered the root of 
your GUI. It will be filled with functions/data and should contain all 
the functions you need to add data and widgets.

You can pass in these configuration values in a table, the example 
shown above has soom good defaults.

	font="Vera"

The default font to use, this must have already been loaded via 
wetgenes.gamecake.fonts functions.

	text_size=16
	
The default pixel height to render text at.

	grid_size=32
	
The size in pixels that we try and create buttons at.




## lua.wetgenes.gamecake.zone.scene


	zone_scene = require("wetgenes.gamecake.zone.scene")

Manage a simple structure to help group, update and draw discrete game objects.

This module contains only one function which can be used to create a scene
instance and the rest of this documentation concerns the return from this
create function, not the module

	scene = zone_scene.create()
	scene = zone_scene.create( { sortby={"first","second",...} } )
	
Create and return a scene object. A base object can be passed in and will
be filled in and returned. This can used to supply a sortby table to help
control the order of updates of different castes.

If passed in sortby is a table of [caste_name]=weight values to help control
the order of updates by caste and make it consistent. For instance you might
find it useful to force monsters to update before the player.



## lua.wetgenes.gamecake.zone.scene.add


	scene.add(it,caste,boot)
	scene.add(it,caste)
	scene.add(it)

Add a new item of caste or it.caste to the list of things to update.

The optional boot table contains initialisation/reset values and will 
be remembered in item.boot. If boot.id is given then we will remember 
this item with a call to scene.set(id,it)

The actual act of initalising the item from the boot table is left to 
custom code.



## lua.wetgenes.gamecake.zone.scene.call


	scene.call(fname,...)

If fname is a string then that method will be invoked for every item 
where it exists like so. Only the first item of each caste is tested, 
if the function exists there then it is expected to exist for all items 
of a given caste. This enables us to skip entire castes whilst still 
making all functions optional.

	if it[fname] then it[fname](it,...) end
	
if fname is a function then it will result in every item geting called with
that function like so.

	fname(it,...)

The calls are always made backwards through the table so that an item can
choose to delete itself.

Finally we return the number of calls we made so you can keep track of
currently active items.



## lua.wetgenes.gamecake.zone.scene.caste


	scene.caste(caste)

Get the list of items of a given caste, eg "bullets" or "enemies"

This list will be created if it does not already exist.

scene.sortby is used to keep this list in order and an empty system 
will be autocreated if needed.



## lua.wetgenes.gamecake.zone.scene.find_uid


	local it = scene.find_uid( uid )

Return the item with the given uid or nil if no such item has been 
remembered or a nil uid has been passed in.



## lua.wetgenes.gamecake.zone.scene.forget_uid


	local it = scene.forget_uid( {uid=uid} )

Remove the item from the map of uids. Returns the item or nil if uid was
invalid unset or our map was not pointing to the correct item.

	local it = scene.forget_uid( scene.find_uid( uid ) )

Chain with find_uid to forget an item by uid and this is safe even if the item
does not exist.



## lua.wetgenes.gamecake.zone.scene.generate_uid


	local uid = scene.generate_uid()

	local uid = scene.generate_uid(uid)

Simple unique incremental IDs for items to be used instead of 
pointers, starts at 1 and goes up as items are added.

Ideally you will also need extra code for compacting these ids ( 
probably as an export, renumber, import cycle ) to stop them getting 
huge and breaking. So a save/load system which is out of scope for this 
code.

Generally, unless we are creating and destroying millions of items we 
will run out of memory way before this number gets too big.

If a uid is passed in then we will return uid+1 and continue on upwards 
for subsequent calls.

The point of uids is so we can refer to and lazy link items as weak 
values making it easy to delete an item without worrying too much about 
where it has been used. So instead of putting the item table inside 
another item table we can use a uid reference instead.

Good for serialising data as well.



## lua.wetgenes.gamecake.zone.scene.get


	scene.get(name)

get a value previously saved, this is an easy way to find a unique entity, eg
the global space but it can be used to save any values you wish not just to
bookmark unique items.



## lua.wetgenes.gamecake.zone.scene.manifest


	scene.manifest(name,value)

get a value previously saved, or initalize it to the given value if it does not
already exist. The default value is {} as this is intended for lists.



## lua.wetgenes.gamecake.zone.scene.remember_uid


	local it = scene.remember_uid( {} )

Remember an item in the uids table, generating a uid and setting it.uid to this
value if one does not already exist.



## lua.wetgenes.gamecake.zone.scene.remove


	scene.remove(it)

Remove this item, this is slightly expensive as we need to search in a table
to find it before calling table.remove which then has to shuffle the table to
fill in the hole.

With very dynamic items it can be faster to allocate all the items you need at
the start and then flag them on/off rather than add and remove dynamically.

It may make more sense to create a system which handles its own list of
objects, such as particles. Then only use the items to keep track of a
master particles item that contains many particles and can add/remove/recycle
as it sees fit.



## lua.wetgenes.gamecake.zone.scene.reset


	scene.reset()
	
Empty the list of items to update and draw, this does not reset the systems
table that should be modified with the insert and remove functions.



## lua.wetgenes.gamecake.zone.scene.set


	scene.set(name,value)

save a value by a unique name



## lua.wetgenes.gamecake.zone.scene.sortby_update


	scene.sortby_update()

A function that takes the array part of scene.sortby and reverses the
key=value so a simple order list can be provided without any explicit weights.
The first caste name in the array gets a weight of 1, second 2 and so on.



## lua.wetgenes.gamecake.zone.scene.status


	print( scene.status() )

Return a debug string giving details about the system order and current number
of items of each caste.



## lua.wetgenes.gamecake.zone.scene.systems


	scene.systems={name=system,[1]=system}

A sorted table and lookup by caste name of each system. Table is sorted so it
will be traversed backwards, backwards traversal allows the current item to
delete itself.



## lua.wetgenes.gamecake.zone.scene.systems.call


	scene.systems_call(fname,...)

For every system call the function called fname like so.

	system[fname](system,...)

Returns the number of calls made, which will be the number of systems that had
an fname function to call.



## lua.wetgenes.gamecake.zone.scene.systems.cocall


	scene.systems_cocall(fname,...)

For every system call the function called fname inside a coroutine like 
so.

	system[fname](system,...)
	
This function can yield and should do so if it is waiting for another 
system to do something. All coroutines will be run in a round robin 
style until they all complete.

Returns the number of calls made, which will be the number of systems that had
an fname function to call.



## lua.wetgenes.gamecake.zone.scene.systems.insert


	scene.systems_insert(system)

Insert a new system replacing any system of the same caste. system.caste should
be set to the caste of the system for this to work.



## lua.wetgenes.gamecake.zone.scene.systems.remove


	system = scene.systems_remove(caste)

Remove and return the system of the given caste.



## lua.wetgenes.gamecake.zone.scene.uids


	scene.uids={[uid]=item,...}

A map of currently remembered uids to items.



## lua.wetgenes.gamecake.zone.system.items


Generic game objects, IE things that exist in game and can be interacted with 
by the players.

Some times these exist in world, sometimes they exist in bags or inventory etc 
so are not visible world objects.

As such this item *always* exists here and will be moved in and out of world 
objects as necessary.



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




## lua.wetgenes.rnd64k


	rnd = wetgenes.rnd64k(seed)
	
	r	= rnd()		-- a fraction between 0 and 1 inclusive
	r	= rnd(a)	-- an integer between 1 and a inclusive
	r	= rnd(a,b)	-- an integer between a and b inclusive

a sequence of 65536 numbers starting at seed which should be an integer 
between 0 and 65535 or a string which will be used to generate this 
number.



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



## lua.wetgenes.tardis.m2


The metatable for a 2x2 matrix class, use the new function to actually create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m2.adjugate


	m2 = m2:adjugate()
	m2 = m2:adjugate(r)

Adjugate this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.cofactor


	m2 = m2:cofactor()
	m2 = m2:cofactor(r)

Cofactor this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.determinant


	value = m2:determinant()

Return the determinant value of this m2.



## lua.wetgenes.tardis.m2.identity


	m2 = m2:identity()

Set this m2 to the identity matrix.



## lua.wetgenes.tardis.m2.inverse


	m2 = m2:inverse()
	m2 = m2:inverse(r)

Inverse this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.minor_xy


	value = m2:minor_xy()

Return the minor_xy value of this m2.



## lua.wetgenes.tardis.m2.new


	m2 = tardis.m2.new()

Create a new m2 and optionally set it to the given values, m2 methods 
usually return the input m2 for easy function chaining.



## lua.wetgenes.tardis.m2.scale


	m2 = m2:scale(s)
	m2 = m2:scale(s,r)

Scale this m2 by s.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m2.transpose


	m2 = m2:transpose()
	m2 = m2:transpose(r)

Transpose this m2.

If r is provided then the result is written into r and returned 
otherwise m2 is modified and returned.



## lua.wetgenes.tardis.m3


The metatable for a 3x3 matrix class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m3.adjugate


	m3 = m3:adjugate()
	m3 = m3:adjugate(r)

Adjugate this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.cofactor


	m3 = m3:cofactor()
	m3 = m3:cofactor(r)

Cofactor this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.determinant


	value = m3:determinant()

Return the determinant value of this m3.



## lua.wetgenes.tardis.m3.identity


	m3 = m3:identity()

Set this m3 to the identity matrix.



## lua.wetgenes.tardis.m3.inverse


	m3 = m3:inverse()
	m3 = m3:inverse(r)

Inverse this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.m4


	m4 = m3:m4()

Expand an m3 matrix into an m4 matrix.



## lua.wetgenes.tardis.m3.minor_xy


	value = m3:minor_xy()

Return the minor_xy value of this m3.



## lua.wetgenes.tardis.m3.new


	m3 = tardis.m3.new()

Create a new m3 and optionally set it to the given values, m3 methods 
usually return the input m3 for easy function chaining.



## lua.wetgenes.tardis.m3.scale


	m3 = m3:scale(s)
	m3 = m3:scale(s,r)

Scale this m3 by s.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.transpose


	m3 = m3:transpose()
	m3 = m3:transpose(r)

Transpose this m3.

If r is provided then the result is written into r and returned 
otherwise m3 is modified and returned.



## lua.wetgenes.tardis.m3.v3


	v3 = m3:v3(n)

Extract and return a "useful" v3 from an m3 matrix. The first vector is the x
axis, then y axis , then z axis.




## lua.wetgenes.tardis.m4


The metatable for a 4x4 matrix class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.m4.add


	m4 = m4:add(m4b)
	m4 = m4:add(m4b,r)

Add m4b this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.adjugate


	m4 = m4:adjugate()
	m4 = m4:adjugate(r)

Adjugate this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.arotate


	m4 = m4:arotate(degrees,v3a)
	m4 = m4:arotate(degrees,v3a,r)

Apply a rotation in degres around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.cofactor


	m4 = m4:cofactor()
	m4 = m4:cofactor(r)

Cofactor this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.determinant


	value = m4:determinant()

Return the determinant value of this m4.



## lua.wetgenes.tardis.m4.get_rotation_q4


	q4 = m4:get_rotation_q4(r)

Get quaternion rotation from a scale/rot/trans matrix. Note that scale 
is assumed to be uniform which it usually is. If that is not the case 
then remove the scale first.

If r is provided then the result is written into r and returned 
otherwise a new q4 is created and returned.



## lua.wetgenes.tardis.m4.get_scale_v3


	v3 = m4:get_scale_v3(r)

Get v3 scale from a scale/rot/trans matrix

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.m4.get_translation_v3


	v3 = m4:get_translation_v3(r)

Get v3 translation from a scale/rot/trans matrix

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.m4.identity


	m4 = m4:identity()

Set this m4 to the identity matrix.



## lua.wetgenes.tardis.m4.inverse


	m4 = m4:inverse()
	m4 = m4:inverse(r)

Inverse this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.lerp


	m4 = m4:lerp(m4b,s)
	m4 = m4:lerp(m4b,s,r)

Lerp from m4 to m4b by s.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.m3


	m4 = m4:m3()

Grab tthe top,left parts of the m4 matrix as an m3 matrix.



## lua.wetgenes.tardis.m4.minor_xy


	value = m4:minor_xy()

Return the minor_xy value of this m4.



## lua.wetgenes.tardis.m4.new


	m4 = tardis.m4.new()

Create a new m4 and optionally set it to the given values, m4 methods 
usually return the input m4 for easy function chaining.



## lua.wetgenes.tardis.m4.prearotate


	m4 = m4:prearotate(degrees,v3a)
	m4 = m4:prearotate(degrees,v3a,r)

Pre apply a rotation in degrees around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.preqrotate


	m4 = m4:preqrotate(q)
	m4 = m4:preqrotate(q,r)

Pre apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.prerotate


	m4 = m4:prerotate(degrees,v3a)
	m4 = m4:prerotate(degrees,v3a,r)
	m4 = m4:prerotate(degrees,x,y,z)
	m4 = m4:prerotate(degrees,x,y,z,r)
	m4 = m4:prerotate(q)
	m4 = m4:prerotate(q,r)

Pre apply quaternion or angle rotation to this matrix depending on 
arguments provided.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.prerrotate


	m4 = m4:prerrotate(radians,v3a)
	m4 = m4:prerrotate(radians,v3a,r)

Pre apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.prescale


	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Pre Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.pretranslate


	m4 = m4:pretranslate(x,y,z)
	m4 = m4:pretranslate(x,y,z,r)
	m4 = m4:pretranslate(v3a)
	m4 = m4:pretranslate(v3a,r)

Translate this m4 along its global axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.pretranslate_v3


	m4 = m4:pretranslate_v3(v3a)
	m4 = m4:pretranslate_v3(v3a,r)

Translate this m4 along its global axis by v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.qrotate


	m4 = m4:qrotate(q)
	m4 = m4:qrotate(q,r)

Apply a quaternion rotation to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.rotate


	m4 = m4:rotate(degrees,v3a)
	m4 = m4:rotate(degrees,v3a,r)
	m4 = m4:rotate(degrees,x,y,z)
	m4 = m4:rotate(degrees,x,y,z,r)
	m4 = m4:rotate(q)
	m4 = m4:rotate(q,r)

Apply quaternion or angle rotation to this matrix depending on 
arguments provided.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.rrotate


	m4 = m4:rrotate(radians,v3a)
	m4 = m4:rrotate(radians,v3a,r)

Apply a rotation in radians around the given axis to this matrix.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.scale


	m4 = m4:scale(s)
	m4 = m4:scale(s,r)
	m4 = m4:scale(x,y,z)
	m4 = m4:scale(x,y,z,r)
	m4 = m4:scale(v3)
	m4 = m4:scale(v3,r)

Scale this m4 by {s,s,s} or {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.scale_v3


	m4 = m4:scale_v3(v3)
	m4 = m4:scale_v3(v3,r)

Scale this m4 by {x,y,z} or v3.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.setrot


	m4 = m4:setrot(degrees,v3a)

Set this matrix to a rotation matrix around the given normal by the given degrees.

we will automatically normalise v3a if necessary.



## lua.wetgenes.tardis.m4.setrrot


	m4 = m4:setrrot(radians,v3a)

Set this matrix to a rotation matrix around the given normal by the given radians.

we will automatically normalise v3a if necessary.



## lua.wetgenes.tardis.m4.sub


	m4 = m4:sub(m4b)
	m4 = m4:sub(m4b,r)

Subtract m4b this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.translate


	m4 = m4:translate(x,y,z)
	m4 = m4:translate(x,y,z,r)
	m4 = m4:translate(v3a)
	m4 = m4:translate(v3a,r)

Translate this m4 along its local axis by {x,y,z} or v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.translate_v3


	m4 = m4:translate_v3(v3a)
	m4 = m4:translate_v3(v3a,r)

Translate this m4 along its local axis by v3a.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.transpose


	m4 = m4:transpose()
	m4 = m4:transpose(r)

Transpose this m4.

If r is provided then the result is written into r and returned 
otherwise m4 is modified and returned.



## lua.wetgenes.tardis.m4.v3


	v3 = m4:v3(n)

Extract and return a "useful" v3 from an m4 matrix. The first vector is the x
axis, then y axis , then z axis and finally transform.

If n is not given or not a known value then we return the 4th vector which is
the "opengl" transform as that is the most useful v3 part of an m4.



## lua.wetgenes.tardis.m4_stack


	stack = tardis.m4_stack()

create an m4 stack that is very similar to an old school opengl transform
stack.



## lua.wetgenes.tardis.plane


A 3d space plane class.

[1]position , [2]normal

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.plane.new


	plane = tardis.plane.new(p,n)

Create a new plane and optionally set it to the given values.



## lua.wetgenes.tardis.q4


The metatable for a quaternion class, use the new function to actually create an object.

We also inherit all the functions from tardis.v4



## lua.wetgenes.tardis.q4.get_yaw_pitch_roll


	v3 = q4:get_yaw_pitch_roll()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.q4.get_yaw_pitch_roll_in_radians


	v3 = q4:get_yaw_pitch_roll_in_radians()

Get a yaw,pitch,roll degree rotation from this quaternion

If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



## lua.wetgenes.tardis.q4.identity


	q4 = q4:identity()

Set this q4 to its 0,0,0,1 identity



## lua.wetgenes.tardis.q4.lerp


	q4 = q4:lerp(q4b,s)
	q4 = q4:lerp(q4b,s,r)

Nlerp from q4 to q4b by s.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.new


	q4 = tardis.q4.new()

Create a new q4 and optionally set it to the given values, q4 methods 
usually return the input q4 for easy function chaining.



## lua.wetgenes.tardis.q4.prerotate


	q4 = q4:prerotate(degrees,v3a)
	q4 = q4:prerotate(degrees,v3a,r)

Pre apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.prerrotate


	q4 = q4:prerrotate(radians,v3a)
	q4 = q4:prerrotate(radians,v3a,r)

Pre apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.reverse


	q4 = q4:reverse()
	q4 = q4:reverse(r)

Reverse the rotation of this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.rotate


	q4 = q4:rotate(degrees,v3a)
	q4 = q4:rotate(degrees,v3a,r)

Apply a degree rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.rrotate


	q4 = q4:rrotate(radians,v3a)
	q4 = q4:rrotate(radians,v3a,r)

Apply a radian rotation to this quaternion.

If r is provided then the result is written into r and returned 
otherwise q4 is modified and returned.



## lua.wetgenes.tardis.q4.set


	q4 = tardis.q4.set(q4,{0,0,0,1})
	q4 = tardis.q4.set(q4,0,0,0,1)
	q4 = tardis.q4.set(q4,{"xyz",0,90,0})
	q4 = tardis.q4.set(q4,"xyz",0,90,0)
	q4 = tardis.q4.set(q4,"xyz",{0,90,0})

If the first item in the stream is not a string then this is just a normal
array.set style.

If first parameter of the stream is a string then initialise the quaternion
using a simple axis rotation notation Where the string is a list of axis. This
string is lower case letters. x y or z and then the following numbers are
amount of rotation to apply around that axis in degrees. You should provide as
many numbers as letters.

Essentially this gives you a way of initialising quaternion rotations in an
easily readable way.



## lua.wetgenes.tardis.q4.set_yaw_pitch_roll


	q4 = q4:set_yaw_pitch_roll(v3)
	q4 = q4:set_yaw_pitch_roll({90,60,30})	-- 30yaw 60pitch 90roll

Set a V3(roll,pitch,yaw) degree rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last



## lua.wetgenes.tardis.q4.set_yaw_pitch_roll_in_radians


	q4 = q4:set_yaw_pitch_roll_in_radians(v)

Set a V3(roll,pitch,yaw) radian rotation into this quaternion

	yaw   v[3] is rotation about the z axis and is applied first
	pitch v[2] is rotation about the y axis and is applied second
	roll  v[1] is rotation about the z axis and is applied last



## lua.wetgenes.tardis.q4.setrot


	q4 = q4:setrot(degrees,v3a)

Set this quaternion to a rotation around the given normal by the given degrees.



## lua.wetgenes.tardis.q4.setrrot


	q4 = q4:setrrot(radians,v3a)

Set this quaternion to a rotation around the given normal by the given radians.



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



## lua.wetgenes.tardis.v2


The metatable for a 2d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v2.add


	v2 = v2:add(v2b)
	v2 = v2:add(v2b,r)

Add v2b to v2.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.cross


	value = v2:cross(v2b)

Extend to 3d then only return z value as x and y are always 0



## lua.wetgenes.tardis.v2.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v2.dot


	value = v2:dot(v2b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v2.identity


	v2 = v2:identity()

Set this v2 to all zeros.



## lua.wetgenes.tardis.v2.len


	value = v2:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v2.lenlen


	value = v2:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v2.mul


	v2 = v2:mul(v2b)
	v2 = v2:mul(v2b,r)

Multiply v2 by v2b.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.new


	v2 = tardis.v2.new()

Create a new v2 and optionally set it to the given values, v2 methods 
usually return the input v2 for easy function chaining.



## lua.wetgenes.tardis.v2.normalize


	v2 = v2:normalize()
	v2 = v2:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.oo


	v2 = v2:oo()
	v2 = v2:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.scale


	v2 = v2:scale(s)
	v2 = v2:scale(s,r)

Scale this v2 by s.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v2.sub


	v2 = v2:sub(v2b)
	v2 = v2:sub(v2b,r)

Subtract v2b from v2.

If r is provided then the result is written into r and returned 
otherwise v2 is modified and returned.



## lua.wetgenes.tardis.v3


The metatable for a 3d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v3.add


	v3 = v3:add(v3b)
	v3 = v3:add(v3b,r)

Add v3b to v3.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.angle


	radians,axis = v3a:dot(v3b)
	radians,axis = v3a:dot(v3b,axis)

Return radians and axis of rotation between these two vectors. If axis is given 
then it must represent a positive world aligned axis normal. So V3(1,0,0) or 
V3(0,1,0) or V3(0,0,1) only. The point of providing an axis allows the returned 
angle to be over a 360 degree range rather than flipping the axis after 180 
degrees this means the second axis returned value can be ignored as it will 
always be the axis that is passed in.



## lua.wetgenes.tardis.v3.cross


	v3 = v3:cross(v3b)
	v3 = v3:cross(v3b,r)

Return the cross product of these two vectors.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v3.dot


	value = v3:dot(v3b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v3.identity


	v3 = v3:identity()

Set this v3 to all zeros.



## lua.wetgenes.tardis.v3.len


	value = v3:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v3.lenlen


	value = v3:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v3.mul


	v3 = v3:mul(v3b)
	v3 = v3:mul(v3b,r)

Multiply v3 by v3b.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.new


	v3 = tardis.v3.new()

Create a new v3 and optionally set it to the given values, v3 methods 
usually return the input v3 for easy function chaining.



## lua.wetgenes.tardis.v3.normalize


	v3 = v3:normalize()
	v3 = v3:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.oo


	v3 = v3:oo()
	v3 = v3:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.scale


	v3 = v3:scale(s)
	v3 = v3:scale(s,r)

Scale this v3 by s.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v3.sub


	v3 = v3:sub(v3b)
	v3 = v3:sub(v3b,r)

Subtract v3b from v3.

If r is provided then the result is written into r and returned 
otherwise v3 is modified and returned.



## lua.wetgenes.tardis.v4


The metatable for a 4d vector class, use the new function to actually 
create an object.

We also inherit all the functions from tardis.array



## lua.wetgenes.tardis.v4.add


	v4 = v4:add(v4b)
	v4 = v4:add(v4b,r)

Add v4b to v4.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.distance


	value = a:distance(b)

Returns the length of the vector between a and b.



## lua.wetgenes.tardis.v4.dot


	value = v4:dot(v4b)

Return the dot product of these two vectors.



## lua.wetgenes.tardis.v4.identity


	v4 = v4:identity()

Set this v4 to all zeros.



## lua.wetgenes.tardis.v4.len


	value = v4:len()

Returns the length of this vector.



## lua.wetgenes.tardis.v4.lenlen


	value = v4:lenlen()

Returns the length of this vector, squared, this is often all you need 
for comparisons so lets us skip the sqrt.



## lua.wetgenes.tardis.v4.mul


	v4 = v4:mul(v4b)
	v4 = v4:mul(v4b,r)

Multiply v4 by v4b.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.new


	v4 = tardis.v4.new()

Create a new v4 and optionally set it to the given values, v4 methods 
usually return the input v4 for easy function chaining.



## lua.wetgenes.tardis.v4.normalize


	v4 = v4:normalize()
	v4 = v4:normalize(r)

Adjust the length of this vector to 1.

An input length of 0 will remain at 0.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.oo


	v4 = v4:oo()
	v4 = v4:oo(r)

One Over value. Build the reciprocal of all elements. 

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.scale


	v4 = v4:scale(s)
	v4 = v4:scale(s,r)

Scale this v4 by s.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.sub


	v4 = v4:sub(v4b)
	v4 = v4:sub(v4b,r)

Subtract v4b from v4.

If r is provided then the result is written into r and returned 
otherwise v4 is modified and returned.



## lua.wetgenes.tardis.v4.to_v3


	v3 = v4:to_v3()
	v3 = v4:to_v3(r)

scale [4] to 1 then throw it away so we have a v3 xyz
 
If r is provided then the result is written into r and returned 
otherwise a new v3 is created and returned.



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


	tasks:del_thread(task)
	
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



## lua.wetgenes.tasks_msgp.ip6_to_addr


Parse an array of 8 numbers into an ip6 address with :: in the first 
longest run of zeros if needed and lowercase hex letters.



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



## lua.wetgenes.txt.diff.find


Given two tables of strings, return the length , starta , startb of the longest 
common subsequence in table indexes or nil if not similar.




## lua.wetgenes.txt.diff.match


Given two tables of strings, return two tables of strings of 
the same length where as many strings as possible match.



## lua.wetgenes.txt.diff.split


Use the delimiter to split a string into a table of strings such that 
each string ends in the delimiter (except for possibly the final string) and a 
table.concat on the result will recreate the input string exactly.

	table = wtxtdiff.split(string,delimiter)

String is the string to split and delimiter is a lua pattern so any 
special chars should be escaped.

for example

	st = wtxtdiff.split(s) -- split on newline (default)
	st = wtxtdiff.split(s,"\n") -- split on newline (explicit)

	st - wtxtdiff.split(s,"%s+") -- split on white space



## lua.wetgenes.txt.diff.trim


Given two tables of strings, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.




## lua.wetgenes.txt.utf.char


	string = wutf.char(number)

convert a single unicode value to a utf8 string of 1-4 bytes



## lua.wetgenes.txt.utf.charpattern


	string:gmatch(wutf.charpattern)

lua pattern to match each utf8 character in a string



## lua.wetgenes.txt.utf.chars


	string = wutf.chars(number,number,...)
	string = wutf.chars({number,number,...})

convert one or more unicode values into a utf8 string



## lua.wetgenes.txt.utf.length


	unicode = wutf.ncode(string,index)

get the utf8 value at the given code index.

Note that this is slower than wutf.code as we must search the string to 
find the byte index of the code. 



## lua.wetgenes.txt.utf.map_latin0_to_unicode


	unicode = wutf.map_latin0_to_unicode[latin0] or latin0



## lua.wetgenes.txt.utf.map_unicode_to_latin0


	latin0 = wutf.map_unicode_to_latin0[unicode] or unicode

I prefer the coverage of latin0 (ISO/IEC 8859-15) for font layout as it 
is just a few small differences for western european languages to get 
most needed glyphs into the first 256 codes.



## lua.wetgenes.txt.utf.size


	size = wutf.size(string,index)

get the size in bytes of the utf8 value at the given byte index.

	size = wutf.size(string)

get the size in bytes of the utf8 value at the start of this string

The return value will be 1-4 as 4 is the biggest utf8 code size.



## lua.wetgenes.txt.utf.string


	unicode = wutf.code(string,index)

get the utf8 value at the given byte index.


	unicode = wutf.code(string)

get the utf8 value at the start of this string



## lua.wetgenes.txt.words.load


	yes = wtxtwords.check(word)

This is a fast check if the word exists.

May call wtxtwords.load() to auto load data.



## lua.wetgenes.txt.words.transform


	list = wtxtwords.transform(word,count,addletters,subletters)

Returns a table of upto count correctly spelled words that you may have 
miss spelt given the input word ordered by probability.

If the input word is spelled correctly then it will probably be the 
first word in this list but that is not guaranteed.

addletters is the maximum number of additive transforms, the higher 
this number the slower this function and it defaults to 4.

subletters is the maximum number of subtractive transforms and will not 
have much impact on speed, this defaults to the same value as 
addletters.

We run subletters subtractive transforms on our starting word and then 
we scan all possible words and perform addletters number of subtractive 
transforms on them and see if they match any of the transforms we built 
from our starting word. A match then means we can add up the number of 
transforms on both sides and that is how many steps it would take to 
get from one word to another by adding and subtracting letters.



## prefabs


The yarn building blocks, recipes to build items from, multiple rules 
can be assigned to each item.



## rules


How the yarn engine should behave



## setup


Called once to setup things in the first update loop after hardware has 
been initialised.



## setup_menu


	menu = setup_menu()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.




## update


	update()

Update and draw loop, called every frame.
