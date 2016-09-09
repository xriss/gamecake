--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#wetgenes.chipmunk

	local chipmunk=require("wetgenes.chipmunk")

We use chipmunk as the local name of this library.

A lua binding to the Chipmunk2D physics library https://chipmunk-physics.net/

]]

local chipmunk={}

local core=require("wetgenes.chipmunk.core")

-- meta methods bound to the various objects

chipmunk.space_functions={is="space"}
chipmunk.space_metatable={__index=chipmunk.space_functions}

chipmunk.body_functions={is="body"}
chipmunk.body_metatable={__index=chipmunk.body_functions}

chipmunk.shape_functions={is="shape"}
chipmunk.shape_metatable={__index=chipmunk.shape_functions}

chipmunk.arbiter_functions={is="arbiter"}
chipmunk.arbiter_metatable={__index=chipmunk.arbiter_functions}

chipmunk.constraint_functions={is="constraint"}
chipmunk.constraint_metatable={__index=chipmunk.constraint_functions}


--[[#wetgenes.chipmunk.space

	space=chipmunk.space()

Create the space you will be simulating physics in.

]]
chipmunk.space=function(...)
	local space={}
	space.bodies={}
	space.shapes={}
	space.callbacks={}
	setmetatable(space,chipmunk.space_metatable)
	space[0]=core.space_create(space)

-- hack in this spaces default static body
-- we have a special case in the binding that automatically gets the static body from the space ptr
	space.static=chipmunk.body(space[0])
	space.static.in_space=space
	
	return space
end

--[[#wetgenes.chipmunk.body

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


]]
chipmunk.body=function(a,...)
	local body={}
	setmetatable(body,chipmunk.body_metatable)
	body.shapes={}
	if type(a)=="userdata" then -- wrap a userdata
		body[0]=a
	else
		body[0]=core.body_create(a,...)
	end
	return body
end

--[[#wetgenes.chipmunk.shape

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

	shape=chipmunk.shape(body,"poly",...)

Form of "poly" is not wired up to anything yet but probably a stream of 
points?

	shape=chipmunk.shape(body,"box",minx,miny,maxx,maxy,radius)

Form of "box" needs two points for opposite corners, lowest pair 
followed by highest pair and a radius. The radius should be 0 unless 
you want rounded corners


]]
chipmunk.shape=function(body,form,...)
	local shape={form=form}
	setmetatable(shape,chipmunk.shape_metatable)
	shape[0]=core.shape_create(body[0],form,...)
	return shape
end

--[[#wetgenes.chipmunk.constraint

	constraint=chipmunk.constraint(abody,bbody,form,...)

Create a constraint between two bodies.

You will need to add the constraint to a space before it has any effect 
so it is normally preferable to use the space:constraint function which 
will call this function and then automatically add the constraint into 
the space.


]]
chipmunk.constraint=function(abody,bbody,form,...)
	local constraint={form=form}
	setmetatable(shape,chipmunk.constraint_metatable)
	constraint[0]=core.constraint_create(abody[0],bbody[0],form,...)
	return constraint
end

--[[#wetgenes.chipmunk.space.iterations

	v=space:iterations()
	v=space:iterations(v)

Get and/or Set the iterations for this space.

]]
chipmunk.space_functions.iterations=function(space,v)
	return core.space_iterations(space[0],v)
end

--[[#wetgenes.chipmunk.space.gravity

	vx,vy=space:gravity()
	vx,vy=space:gravity(vx,vy)

Get and/or Set the gravity vector for this space.

]]
chipmunk.space_functions.gravity=function(space,vx,vy)
	return core.space_gravity(space[0],vx,vy)
end

--[[#wetgenes.chipmunk.space.damping

	v=space:damping()
	v=space:damping(v)

Get and/or Set the damping for this space.

]]
chipmunk.space_functions.damping=function(space,v)
	return core.space_damping(space[0],v)
end

--[[#wetgenes.chipmunk.space.collision_slop

	v=space:collision_slop()
	v=space:collision_slop(v)

Get and/or Set the colision slop for this space.

]]
chipmunk.space_functions.collision_slop=function(space,v)
	return core.space_collision_slop(space[0],v)
end

--[[#wetgenes.chipmunk.space.collision_bias

	v=space:collision_bias()
	v=space:collision_bias(v)

Get and/or Set the colision bias for this space.

]]
chipmunk.space_functions.collision_bias=function(space,v)
	return core.space_collision_bias(space[0],v)
end

--[[#wetgenes.chipmunk.space.add_handler

	space:add_handler(handler,id1,id2)
	space:add_handler(handler,id1)
	space:add_handler(handler)

Add collision callback handler, for the given collision types.

The handler table will have other values inserted in it and will be 
used as an arbiter table in callbacks. So *always* pass in a new one to 
this function. There does not seem to be a way to free handlers so be 
careful what you add.

]]
chipmunk.space_functions.add_handler=function(space,arbiter,id1,id2)

	setmetatable(arbiter,chipmunk.arbiter_metatable)
	return core.space_add_handler(space[0],arbiter,id1,id2)

end

--[[#wetgenes.chipmunk.space.add

	space:add(body)
	space:add(shape)
	space:add(constraint)

Add a body/shape/constraint to the space.

]]
chipmunk.space_functions.add=function(space,it)
	if     it.is=="body" then

		it.in_space=space
		core.body_lookup(it[0],space.bodies,it)
		return core.space_add_body(space[0],it[0])

	elseif it.is=="shape" then

		it.in_space=space
		core.shape_lookup(it[0],space.shapes,it)
		return core.space_add_shape(space[0],it[0])
	
	elseif it.is=="constraint" then

		it.in_space=space
		core.constraint_lookup(it[0],space.constraints,it)
		return core.space_add_constraint(space[0],it[0])

	else
		error("unknown "..it.is)
	end
end

--[[#wetgenes.chipmunk.space.remove

	space:remove(body)
	space:remove(shape)
	space:remove(constraint)

Remove a body/shape/constraint from this space.

]]
chipmunk.space_functions.remove=function(space,it)
	if     it.is=="body" then

		it.in_space=nil
		core.body_lookup(it[0],space.bodies,false)
		return core.space_remove_body(space[0],it[0])

	elseif it.is=="shape" then

		it.in_space=nil
		core.shape_lookup(it[0],space.shapes,false)
		return core.space_remove_shape(space[0],it[0])
	
	elseif it.is=="constraint" then

		it.in_space=nil
		core.constraint_lookup(it[0],space.constraints,false)
		return core.space_remove_constraint(space[0],it[0])
	
	else
		error("unknown "..it.is)
	end
end

--[[#wetgenes.chipmunk.space.contains

	space:contains(body)
	space:contains(shape)
	space:contains(constraint)

Does the space contain this body/shape/constraint, possibly superfluous 
as we can check our own records.

]]
chipmunk.space_functions.contains=function(space,it)
	if     it.is=="body" then

		return core.space_contains_body(space[0],it[0])

	elseif it.is=="shape" then

		return core.space_contains_shape(space[0],it[0])
	
	elseif it.is=="constraint" then

		return core.space_contains_constraint(space[0],it[0])

	else
		error("unknown "..it.is)
	end
end


--[[#wetgenes.chipmunk.space.body

	space:body(...)

Create and add this body to the space.

]]
chipmunk.space_functions.body=function(space,...)
	local body=chipmunk.body(...)
	space:add(body)
	return body
end

--[[#wetgenes.chipmunk.space.constraint

	space:constraint(...)

Create and add this constraint to the space.

]]
chipmunk.space_functions.constraint=function(space,...)
	local constraint=chipmunk.constraint(...)
	space:add(body)
	return constraint
end

--[[#wetgenes.chipmunk.space.step

	space:step(time)

Run the simulation for time in seconds. EG 1/60.

]]
chipmunk.space_functions.step=function(space,ts)
	return core.space_step(space[0],ts)
end

--[[#wetgenes.chipmunk.body.position

	vx,vy=body:position()
	vx,vy=body:position(vx,vy)

Get and/or Set the position for this body.

]]
chipmunk.body_functions.position=function(body,vx,vy)
	return core.body_position(body[0],vx,vy)
end

--[[#wetgenes.chipmunk.body.velocity

	vx,vy=body:velocity()
	vx,vy=body:velocity(vx,vy)

Get and/or Set the velocity for this body.

]]
chipmunk.body_functions.velocity=function(body,vx,vy)
	return core.body_velocity(body[0],vx,vy)
end

--[[#wetgenes.chipmunk.body.angle

	a=body:angle()
	a=body:angle(a)

Get and/or Set the rotation angle in radians for this body.

]]
chipmunk.body_functions.angle=function(body,a)
	return core.body_angle(body[0],a)
end

--[[#wetgenes.chipmunk.body.angular_velocity

	a=body:angular_velocity()
	a=body:angular_velocity(a)

Get and/or Set the angular velocity in radians for this body.

]]
chipmunk.body_functions.angular_velocity=function(body,a)
	return core.body_angular_velocity(body[0],a)
end

--[[#wetgenes.chipmunk.body.shape

	shape=body:shape(form,...)

Add a new shape to this body, returns the shape for further 
modification.

]]
chipmunk.body_functions.shape=function(body,...)
	local shape=chipmunk.shape(body[0],...)
	shape.in_body=body
	body.shapes[shape]=shape
	body.in_space:add(shape)
	return shape
end

--[[#wetgenes.chipmunk.shape.bounding_box

	min_x,min_y,max_x,max_y=shape:bounding_box()

Get the current bounding box for this shape.

]]
chipmunk.shape_functions.bounding_box=function(shape)
	return core.shape_bounding_box(shape[0])
end
--[[#wetgenes.chipmunk.shape.elasticity

	f=shape:elasticity()
	f=shape:elasticity(f)

Get and/or Set the elasticity for this shape.

]]
chipmunk.shape_functions.elasticity=function(shape,f)
	return core.shape_elasticity(shape[0],f)
end

--[[#wetgenes.chipmunk.shape.friction

	f=shape:friction()
	f=shape:friction(f)

Get and/or Set the friction for this shape.

]]
chipmunk.shape_functions.friction=function(shape,f)
	return core.shape_friction(shape[0],f)
end

--[[#wetgenes.chipmunk.shape.collision_type

	f=shape:collision_type()
	f=shape:collision_type(f)

Get and/or Set the collision type for this shape.

]]
chipmunk.shape_functions.collision_type=function(shape,f)
	return core.shape_collision_type(shape[0],f)
end

--[[#wetgenes.chipmunk.shape.filter

	f=shape:filter()
	f=shape:filter(f)

Get and/or Set the filter for this shape.

]]
chipmunk.shape_functions.filter=function(shape,group,categories,mask)
	return core.shape_filter(shape[0],group,categories,mask)
end

--[[#wetgenes.chipmunk.arbiter.points

	points=arbiter:points()
	points=arbiter:points(points)

Get and/or Set the points data for this arbiter.

]]
chipmunk.arbiter_functions.points=function(arbiter,points)
	return core.arbiter_points(arbiter[0],points)
end

--[[#wetgenes.chipmunk.arbiter.surface_velocity

	vx,vy=arbiter:surface_velocity()
	vx,vy=arbiter:surface_velocity(vx,vy)

Get and/or Set the surface velocity for this arbiter.

]]
chipmunk.arbiter_functions.surface_velocity=function(arbiter,vx,vy)
	return core.arbiter_surface_velocity(arbiter[0],vx,vy)
end

--[[#wetgenes.chipmunk.arbiter.ignore

	return arbiter:ignore()

Ignore this collision, from now until the shapes separate.

]]
chipmunk.arbiter_functions.ignore=function(arbiter)
	core.arbiter_ignore(arbiter[0])
	return false
end


return chipmunk

