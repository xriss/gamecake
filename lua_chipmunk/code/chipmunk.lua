--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.chipmunk

	local chipmunk=require("wetgenes.chipmunk")

We use chipmunk as the local name of this library.

A lua binding to the Chipmunk2D physics library [chipmunk-physics.net](https://chipmunk-physics.net/)

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


-- default values to be used if nil
local group_categories_mask=function(group,categories,mask)
	return group or 0,categories or 0xffffffff,mask or 0xffffffff
end

------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space

	space=chipmunk.space()

Create the space you will be simulating physics in.

]]
chipmunk.space=function(...)
	local space={}
	space.bodies={}
	space.shapes={}
	space.constraints={}
	space.callbacks={}
	space.types={}
	setmetatable(space,chipmunk.space_metatable)
	space[0]=core.space_create(space)

-- hack in this spaces default static body
-- we have a special case in the binding that automatically gets the static body from the space ptr
	space.static=chipmunk.body(space[0])
	space.static.in_space=space
	
	return space
end

------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.type

	number = space:type(name)
	name = space:type(number)

Manage collision types, pass in a string and always get a number out. 
This number is consistent only for this space.

Alternatively pass in a number and get a string or nil as a result.

]]
chipmunk.space_functions.type=function(space,name)
	if not space.types[name] then
		if type(name)~="string" then return end -- must not manifest if this is not a string
		local idx=#space.types+1
		space.types[name]=idx
		space.types[idx]=name
	end
	return space.types[name]
end

------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body

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
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape

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

]]
chipmunk.shape=function(body,form,...)
	local shape={form=form}
	setmetatable(shape,chipmunk.shape_metatable)
	shape[0]=core.shape_create(body[0],form,...)
	return shape
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint

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

]]
chipmunk.constraint=function(abody,bbody,form,...)
	local constraint={form=form}
	setmetatable(constraint,chipmunk.constraint_metatable)
	constraint[0]=core.constraint_create(abody[0],bbody[0],form,...)
	return constraint
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.iterations

	v=space:iterations()
	v=space:iterations(v)

Get and/or Set the iterations for this space.

]]
chipmunk.space_functions.iterations=function(space,v)
	return core.space_iterations(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.gravity

	vx,vy=space:gravity()
	vx,vy=space:gravity(vx,vy)

Get and/or Set the gravity vector for this space.

]]
chipmunk.space_functions.gravity=function(space,vx,vy)
	return core.space_gravity(space[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.damping

	v=space:damping()
	v=space:damping(v)

Get and/or Set the damping for this space.

]]
chipmunk.space_functions.damping=function(space,v)
	return core.space_damping(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.idle_speed_threshold

	v=space:idle_speed_threshold()
	v=space:idle_speed_threshold(v)

Get and/or Set the idle speed threshold for this space.

]]
chipmunk.space_functions.idle_speed_threshold=function(space,v)
	return core.space_idle_speed_threshold(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.sleep_time_threshold

	v=space:sleep_time_threshold()
	v=space:sleep_time_threshold(v)

Get and/or Set the sleep time threshold for this space.

]]
chipmunk.space_functions.sleep_time_threshold=function(space,v)
	return core.space_sleep_time_threshold(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.collision_slop

	v=space:collision_slop()
	v=space:collision_slop(v)

Get and/or Set the colision slop for this space.

]]
chipmunk.space_functions.collision_slop=function(space,v)
	return core.space_collision_slop(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.collision_bias

	v=space:collision_bias()
	v=space:collision_bias(v)

Get and/or Set the colision bias for this space.

]]
chipmunk.space_functions.collision_bias=function(space,v)
	return core.space_collision_bias(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.collision_persistence

	v=space:collision_persistence()
	v=space:collision_persistence(v)

Get and/or Set the collision persistence for this space.

]]
chipmunk.space_functions.collision_persistence=function(space,v)
	return core.space_collision_persistence(space[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.current_time_step

	v=space:current_time_step()

Get the current time step for this space.

]]
chipmunk.space_functions.current_time_step=function(space)
	return core.space_current_time_step(space[0])
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.locked

	v=space:locked()

Get the locked state for this space, if true we cannot change shapes.

]]
chipmunk.space_functions.locked=function(space)
	return core.space_locked(space[0])
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.add_handler

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

]]
chipmunk.space_functions.add_handler=function(space,arbiter,id1,id2)

	if type(id1)=="string" then id1=space:type() end
	if type(id2)=="string" then id2=space:type() end

	setmetatable(arbiter,chipmunk.arbiter_metatable)
	return core.space_add_handler(space[0],arbiter,id1,id2)

end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.add

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
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.remove

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
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.contains

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
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.reindex

	space:reindex(shape)
	space:reindex(body)
	space:reindex()

Reindex the shapes, either a specific shape, all the shapes in a body 
or just all the static shapes.

]]
chipmunk.space_functions.reindex=function(space,it)
	if not it then

		return core.space_reindex_static(space[0])

	elseif it.is=="body" then

		return core.space_reindex_shapes_for_body(space[0],it[0])

	elseif it.is=="shape" then

		return core.space_reindex_shape(space[0],it[0])
	
	else
		error("unknown "..it.is)
	end
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.body

	space:body(...)

Create and add this body to the space.

]]
chipmunk.space_functions.body=function(space,...)
	local body=chipmunk.body(...)
	space:add(body)
	return body
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.constraint

	space:constraint(...)

Create and add this constraint to the space.

]]
chipmunk.space_functions.constraint=function(space,...)
	local constraint=chipmunk.constraint(...)
	space:add(constraint)
	return constraint
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.step

	space:step(time)

Run the simulation for time in seconds. EG 1/60.

]]
chipmunk.space_functions.step=function(space,ts)
	return core.space_step(space[0],ts)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_point

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

]]
chipmunk.space_functions.query_point=function(space,x,y,d,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	local dat=core.space_query_point(space[0],x,y,d,group,categories,mask)
	local tab={}
	for i=0,#dat-1,6 do -- format the output so it is a little bit nicer
		local it={}
		tab[1+(i/6)]=it
		it.shape=space.shapes[ dat[1+i] ] -- convert userdata to shape table
		it.point_x    = dat[2+i]
		it.point_y    = dat[3+i]
		it.distance   = dat[4+i]
		it.gradient_x = dat[5+i]
		it.gradient_y = dat[6+i]
	end
	return tab -- an empty table would be no hits
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_point_nearest

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

]]
chipmunk.space_functions.query_point_nearest=function(space,x,y,d,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	local rs,px,py,rd,gx,gy=core.space_query_point_nearest(space[0],x,y,d,group,categories,mask)
	if not rs then return end -- return nil for no hit
	local it={}
	it.shape=space.shapes[ rs ] -- convert userdata to shape table
	it.point_x    = px
	it.point_y    = py
	it.distance   = rd
	it.gradient_x = gx
	it.gradient_y = gy
	return it
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_segment

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

]]
chipmunk.space_functions.query_segment=function(space,sx,sy,ex,ey,r,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	local dat=core.space_query_segment(space[0],sx,sy,ex,ey,r,group,categories,mask)
	local tab={}
	for i=0,#dat-1,6 do -- format the output so it is a little bit nicer
		local it={}
		tab[1+(i/6)]=it
		it.shape=space.shapes[ dat[1+i] ] -- convert userdata to shape table
		it.point_x  = dat[2+i]
		it.point_y  = dat[3+i]
		it.normal_x = dat[4+i]
		it.normal_y = dat[5+i]
		it.alpha    = dat[6+i]
	end
	return tab -- an empty table would be no hits
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_segment_first

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

]]
chipmunk.space_functions.query_segment=function(space,sx,sy,ex,ey,r,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	local rs,px,py,nx,ny,a=core.space_query_segment_first(space[0],sx,sy,ex,ey,r,group,categories,mask)
	if not rs then return end -- return nil for no hit
	local it={}
	it.shape=space.shapes[ rs ] -- convert userdata to shape table
	it.point_x  = px
	it.point_y  = py
	it.normal_x = nx
	it.normal_y = ny
	it.alpha    = a
	return it
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_bounding_box

	array = space:query_bounding_box(lx,ly,hx,hy,group,categories,mask)

Find the shapes that are within this bounding box (lx,ly) to (hx,hy).
Use group,categories and mask to filter the results.

Returns an array of shapes.

]]
chipmunk.space_functions.query_bounding_box=function(space,lx,ly,hx,hy,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	local dat=core.space_query_bounding_box(space[0],lx,ly,hx,hy,group,categories,mask)
	for i=1,#dat do
		dat[i]=space.shapes[ dat[i] ] -- convert userdata to shape table
	end
	return dat -- an empty table would be no hits
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.space.query_shape

	array = space:query_shape(shape)

Find the shapes that intersect with the given shape.

Returns an array of hit data, with each item containing the following.

	it.shape		-- the shape
	it.normal_x		-- the normal at contact (x)
	it.normal_y		-- the normal at contact (y)
	it.contacts		-- array of contact points -> {ax,ay,bx,by,distance,etc...}

]]
chipmunk.space_functions.query_shape=function(space,shape)
	local dat=core.space_query_shape(space[0],shape[0])
	local tab={}
	for i=0,#dat-1,4 do -- format the output so it is a little bit nicer
		local it={}
		tab[1+(i/4)]=it
		it.shape=space.shapes[ dat[1+i] ] -- convert userdata to shape table
		it.normal_x = dat[2+i]
		it.normal_y = dat[3+i]
		it.contacts = dat[4+i]
	end
	return tab -- an empty table would be no hits
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.type

	t=body:type()
	t=body:type(t)

Get and/or Set the type for this body.

]]
chipmunk.body_functions.type=function(body,t)
	local tt={ dynamic=1,kinematic=2,static=3} -- string to number
	local rr={"dynamic","kinematic","static" } -- number to string
	local r=core.body_type(body[0],tt[t])
	return rr[r]
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.mass

	m=body:mass()
	m=body:mass(m)

Get and/or Set the mass for this body.

]]
chipmunk.body_functions.mass=function(body,m)
	return core.body_mass(body[0],m)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.moment

	m=body:moment()
	m=body:moment(m)

Get and/or Set the moment for this body.

]]
chipmunk.body_functions.moment=function(body,m)
	return core.body_moment(body[0],m)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.position

	vx,vy=body:position()
	vx,vy=body:position(vx,vy)

Get and/or Set the position for this body.

]]
chipmunk.body_functions.position=function(body,vx,vy)
	return core.body_position(body[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.center_of_gravity

	vx,vy=body:center_of_gravity()
	vx,vy=body:center_of_gravity(vx,vy)

Get and/or Set the center of gravity for this body.

]]
chipmunk.body_functions.center_of_gravity=function(body,vx,vy)
	return core.body_center_of_gravity(body[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.velocity

	vx,vy=body:velocity()
	vx,vy=body:velocity(vx,vy)

Get and/or Set the velocity for this body.

]]
chipmunk.body_functions.velocity=function(body,vx,vy)
	return core.body_velocity(body[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.force

	vx,vy=body:force()
	vx,vy=body:force(vx,vy)

Get and/or Set the force for this body. This is reset back to 0 after 
each step.

]]
chipmunk.body_functions.force=function(body,vx,vy)
	return core.body_force(body[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.angle

	a=body:angle()
	a=body:angle(a)

Get and/or Set the rotation angle in radians for this body.

]]
chipmunk.body_functions.angle=function(body,a)
	return core.body_angle(body[0],a)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.angular_velocity

	a=body:angular_velocity()
	a=body:angular_velocity(a)

Get and/or Set the angular velocity in radians for this body.

]]
chipmunk.body_functions.angular_velocity=function(body,a)
	return core.body_angular_velocity(body[0],a)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.torque

	a=body:torque()
	a=body:torque(a)

Get and/or Set the torque for this body.

]]
chipmunk.body_functions.torque=function(body,a)
	return core.body_torque(body[0],a)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.apply_force

	body:apply_force(fx,fy,px,py)
	body:apply_force(fx,fy,px,py,"world")

Apply a force to this body at a specific point, the point can be in 
world coordinates if you include the "world" flag but defaults to local 
object coordinates.

]]
chipmunk.body_functions.apply_force=function(body,fx,fy,px,py,world)
	if world=="world" then
		return core.body_apply_force_world_point(body[0],fx,fy,px,py)
	else
		return core.body_apply_force_local_point(body[0],fx,fy,px,py)
	end
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.apply_impulse

	body:apply_impulse(ix,iy,px,py)
	body:apply_impulse(ix,iy,px,py,"world")

Apply a force to this body at a specific point, the point can be in 
world coordinates if you include the "world" flag but defaults to local 
object coordinates.

]]
chipmunk.body_functions.apply_impulse=function(body,ix,iy,px,py,world)
	if world=="world" then
		return core.body_apply_impulse_world_point(body[0],ix,iy,px,py)
	else
		return core.body_apply_impulse_local_point(body[0],ix,iy,px,py)
	end
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.velocity_func

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

]]
chipmunk.body_functions.velocity_func=function(body,cb)
	core.body_velocity_func(body,cb)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.position_func

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

]]
chipmunk.body_functions.velocity_func=function(body,cb)
	core.body_velocity_func(body,cb)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.body.shape

	shape=body:shape(form,...)

Add a new shape to this body, returns the shape for further 
modification.

]]
chipmunk.body_functions.shape=function(body,...)
	local shape=chipmunk.shape(body,...)
	shape.in_body=body
	body.shapes[shape]=shape
	body.in_space:add(shape)
	return shape
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.bounding_box

	min_x,min_y,max_x,max_y=shape:bounding_box()

Get the current bounding box for this shape.

]]
chipmunk.shape_functions.bounding_box=function(shape)
	return core.shape_bounding_box(shape[0])
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.sensor

	f=shape:sensor()
	f=shape:sensor(f)

Get and/or Set the sensor flag for this shape.

]]
chipmunk.shape_functions.sensor=function(shape,f)
	return core.shape_sensor(shape[0],f)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.elasticity

	f=shape:elasticity()
	f=shape:elasticity(f)

Get and/or Set the elasticity for this shape.

]]
chipmunk.shape_functions.elasticity=function(shape,f)
	return core.shape_elasticity(shape[0],f)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.friction

	f=shape:friction()
	f=shape:friction(f)

Get and/or Set the friction for this shape.

]]
chipmunk.shape_functions.friction=function(shape,f)
	return core.shape_friction(shape[0],f)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.surface_velocity

	vx,vy=shape:surface_velocity()
	vx,vy=shape:surface_velocity(vx,vy)

Get and/or Set the surface velocity for this shape.

]]
chipmunk.shape_functions.surface_velocity=function(shape,vx,vy)
	return core.shape_surface_velocity(shape[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.collision_type

	f=shape:collision_type()
	f=shape:collision_type(f)

Get and/or Set the collision type for this shape.

The f argument can be a string in which case it will be converted to a 
number via the space:type function.

]]
chipmunk.shape_functions.collision_type=function(shape,f)
	if type(f)=="string" then f=shape.space:type() end
	return core.shape_collision_type(shape[0],f)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.filter

	f=shape:filter()
	f=shape:filter(f)

Get and/or Set the filter for this shape.

]]
chipmunk.shape_functions.filter=function(shape,group,categories,mask)
	group,categories,mask=group_categories_mask(group,categories,mask)
	return core.shape_filter(shape[0],group,categories,mask)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.radius

	radius=shape:radius()
	radius=shape:radius(radius)

Get and/or Set the radius for this shape. Setting is unsafe and may 
break the physics simulation.

]]
chipmunk.shape_functions.radius=function(shape,radius)
	return core.shape_radius(shape[0],radius)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.query_point

	item = shape:query_point(x,y)

Find the nearest point on the shape from the point at x,y.

returns a table with the following info or nil for no hit

	it.shape		-- the shape
	it.point_x		-- the point of contact (x)
	it.point_y		-- the point of contact (y)
	it.distance		-- the distance to the point of contact
	it.gradient_x	-- the normalised vector to collision (x)
	it.gradient_y	-- the normalised vector to collision (y)

]]
chipmunk.shape_functions.query_point=function(shape,x,y)
	local rs,px,py,rd,gx,gy=core.shape_query_point(shape[0],x,y)
	if not rs then return end -- return nil for no hit
	local it={}
	it.shape=space.shapes[ rs ] -- convert userdata to shape table
	it.point_x    = px
	it.point_y    = py
	it.distance   = rd
	it.gradient_x = gx
	it.gradient_y = gy
	return it
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.shape.query_segment

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

]]
chipmunk.shape_functions.query_segment=function(shape,sx,sy,ex,ey,r)
	local rs,px,py,nx,ny,a=core.shape_query_segment(shape[0],sx,sy,ex,ey,r)
	if not rs then return end -- return nil for no hit
	local it={}
	it.shape=space.shapes[ rs ] -- convert userdata to shape table
	it.point_x  = px
	it.point_y  = py
	it.normal_x = nx
	it.normal_y = ny
	it.alpha    = a
	return it
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.arbiter.points

	points=arbiter:points()
	points=arbiter:points(points)

Get and/or Set the points data for this arbiter.

]]
chipmunk.arbiter_functions.points=function(arbiter,points)
	return core.arbiter_points(arbiter[0],points)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.arbiter.surface_velocity

	vx,vy=arbiter:surface_velocity()
	vx,vy=arbiter:surface_velocity(vx,vy)

Get and/or Set the surface velocity for this arbiter.

]]
chipmunk.arbiter_functions.surface_velocity=function(arbiter,vx,vy)
	return core.arbiter_surface_velocity(arbiter[0],vx,vy)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.arbiter.ignore

	return arbiter:ignore()

Ignore this collision, from now until the shapes separate.

]]
chipmunk.arbiter_functions.ignore=function(arbiter)
	core.arbiter_ignore(arbiter[0])
	return false
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint.max_force

	v=constraint:max_force()
	v=constraint:max_force(v)

Get and/or Set the max force for this constraint.

]]
chipmunk.constraint_functions.max_force=function(constraint,v)
	return core.constraint_max_force(constraint[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint.error_bias

	v=constraint:error_bias()
	v=constraint:error_bias(v)

Get and/or Set the error bias for this constraint.

]]
chipmunk.constraint_functions.error_bias=function(constraint,v)
	return core.constraint_error_bias(constraint[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint.max_bias

	v=constraint:max_bias()
	v=constraint:max_bias(v)

Get and/or Set the max bias for this constraint.

]]
chipmunk.constraint_functions.max_bias=function(constraint,v)
	return core.constraint_max_bias(constraint[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint.collide_bodies

	v=constraint:collide_bodies()
	v=constraint:collide_bodies(v)

Get and/or Set the max collide bodies flag for this constraint.

]]
chipmunk.constraint_functions.collide_bodies=function(constraint,v)
	return core.constraint_collide_bodies(constraint[0],v)
end
------------------------------------------------------------------------
--[[#lua.wetgenes.chipmunk.constraint.impulse

	v=constraint:impulse()

Get the last impulse for this constraint.

]]
chipmunk.constraint_functions.impulse=function(constraint)
	return core.constraint_impulse(constraint[0])
end
------------------------------------------------------------------------


return chipmunk

