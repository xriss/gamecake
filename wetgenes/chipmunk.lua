--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#wetgenes.chipmunk

	local chipmunk=require("wetgenes.chipmunk")

We use chipmunk as the local name of this library.

A lua binding to the Chipmunk2D phsics library https://chipmunk-physics.net/

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
-- we have a special case in the binding that automatically gets the body from the space ptr
	space.static=chipmunk.body(space[0])
	space.static.in_space=space
	
	return space
end

--[[#wetgenes.chipmunk.body

	body=chipmunk.body(mass,inertia)

Create a body with the given mass and inertia.

	body=chipmunk.body("kinematic")

Create a kinematic body.

	body=chipmunk.body("static")

Create a static body.

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

	shape=chipmunk.shape()

Create a shape.

]]
chipmunk.shape=function(...)
	local shape={}
	setmetatable(shape,chipmunk.shape_metatable)
	shape[0]=core.shape_create(...)
	return shape
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

	vx,vy=space:damping()
	vx,vy=space:damping(vx,vy)

Get and/or Set the damping for this space.

]]
chipmunk.space_functions.damping=function(space,v)
	return core.space_damping(space[0],v)
end

--[[#wetgenes.chipmunk.space.add_handler

	space:add_handler(handler,id1,id2)
	space:add_handler(handler,id1)
	space:add_handler(handler)

Add collision callback handler, for the given collision types.

The handler table will have other values inserted in it and is now bound to these types. So always pass in a new one.

]]
chipmunk.space_functions.add_handler=function(space,handler,id1,id2)

	return core.space_add_handler(space[0],handler,id1,id2)

end

--[[#wetgenes.chipmunk.space.add

	space:add(body)

Add a body to the space.

	space:add(shape)

Add a shape to the space.

]]
chipmunk.space_functions.add=function(space,it)
	if     it.is=="body" then

		it.in_space=space
--		space.bodies[it]=it
		core.body_lookup(it[0],space.bodies,it)
		return core.space_add_body(space[0],it[0])

	elseif it.is=="shape" then

		it.in_space=space
--		space.shapes[it]=it
		core.shape_lookup(it[0],space.shapes,it)
		return core.space_add_shape(space[0],it[0])
	
	else
		error("unknown "..it.is)
	end
end

--[[#wetgenes.chipmunk.space.remove

	space:remove(body)

Remove a body from this space.

	space:remove(shape)

Remove a shape from this space.

]]
chipmunk.space_functions.remove=function(space,it)
	if     it.is=="body" then

		it.in_space=nil
--		space.bodies[it]=it
		core.body_lookup(it[0],space.bodies,false)
		return core.space_remove_body(space[0],it[0])

	elseif it.is=="shape" then

		it.in_space=nil
--		space.shapes[it]=nil
		core.shape_lookup(it[0],space.shapes,false)
		return core.space_remove_shape(space[0],it[0])
	
	else
		error("unknown "..it.is)
	end
end

--[[#wetgenes.chipmunk.space.contains

	space:remove(body)

Does the space contain this body, possibly superfluous as we can check our own records.

]]
chipmunk.space_functions.contains=function(space,it)
	if     it.is=="body" then

		return core.space_contains_body(space[0],it[0])

	elseif it.is=="shape" then

		return core.space_contains_shape(space[0],it[0])
	
	else
		error("unknown "..it.is)
	end
end


--[[#wetgenes.chipmunk.space.body

	space:body(time)

Create and add this body to the space.

]]
chipmunk.space_functions.body=function(space,...)
	local body=chipmunk.body(...)
	space:add(body)
	return body
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

	shape=body:shape("circle",radius,x,y)
	shape=body:shape("segment",ax,ay,bx,by,radius)
	shape=body:shape("poly",...)
	shape=body:shape("box",minx,miny,maxx,maxy,radius)

Add a new shape to this body, returns the shape for further modification.

]]
chipmunk.body_functions.shape=function(body,...)
	local shape=chipmunk.shape(body[0],...)
	shape.in_body=body
	body.shapes[shape]=shape
	body.in_space:add(shape)
	return shape
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

return chipmunk
