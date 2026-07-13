--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

--[[#lua.box2d

	local box2d=require("box2d")

We use box2d as the local name of this library.

A lua binding to the box2d physics library see https://box2d.org/ for 
more documentation on how box2d works.

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local box2d=M

local core=require("box2d.core")

local boxxd=require("boxxd")

-- meta methods bound to the various objects

box2d.world_functions={is="world"}
box2d.world_metatable={__index=box2d.world_functions}

box2d.body_functions={is="body"}
box2d.body_metatable={__index=box2d.body_functions}

box2d.shape_functions={is="shape"}
box2d.shape_metatable={__index=box2d.shape_functions}

box2d.joint_functions={is="joint"}
box2d.joint_metatable={__index=box2d.joint_functions}


--[[#lua.box2d.info

	info = box2d.info()

Get all box2d information in a table. Returned table keys are.

	version

A table containing 3 values

	byteCount

Internal memory use.

This also includes everything returned by box2d.get

]]
box2d.info=function()

	local info=core.info()

	for n,v in pairs(core.get()) do
		info[n]=v
	end
	
	return info
end

--[[#lua.box2d.get

	vars = box2d.get()

Get all box2d variables in a table. Returned table keys are.

	lengthUnitsPerMeter

see box2d.set for more information.

]]
box2d.get=function()

	return core.get()
end

--[[#lua.box2d.set

	box2d.set(vars)

Set all box2d variables from a table. Possible table keys are.

	LengthUnitsPerMeter
	
Units per meter, should be set first before creating a world.

Defaults to 1.

Obviously points and vectors should be divided by units to get their 
value in meters but slightly less obviously density values end up 
getting multiplied by units squared since they are area related.

What that means is if you are thinking in meters eg you want to set 
gravity to 10m/s you will need to multiply it by units. Density which 
would normally be 1 should be 1/(units^2) in order to keep the results 
close to defaults.

Note that Box2d recommends not changing this, be we cool, yeah?

]]
box2d.set=function(vars)

	core.set(vars)
end

--[[#lua.box2d.world

	world=box2d.world(def)

Create the world you will be simulating physics in.

def is a table containing the values that you can set in a b2WorldDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values. This may also be a 
fake table created by lua meta.

Booleans should be true or false

	gravity
	restitutionThreshold
	hitEventThreshold
	contactHertz
	contactDampingRatio
	contactSpeed
	maximumLinearSpeed
	enableSleep
	enableContinuous
	enableContactSoftening
	
]]
box2d.world=function(def)
	local world={}

	world.def=def

	world.caches={} -- cached defaults

	world.bodys={}	-- engrish
	world.joints={}
	world.shapes={}

	setmetatable(world,box2d.world_metatable)
	world[0],world.boxid=core.world_create(def)

	return world
end

--[[#lua.box2d.world.destroy

	world:destroy()

Destroy the world and all sub objects ( body , joint , shape )

]]
box2d.world_functions.destroy=function(world)

	for boxid,joint in pairs(world.joints) do
		joint:destroy()
	end
	for boxid,body in pairs(world.bodys) do -- body will also destroy its shapes
		body:destroy()
	end
	core.world_destroy(world[0])
end


--[[#lua.box2d.world.info

	info = world:info()

This returns the def table used to create the world along with as much 
updated information as we have available, mostly intended as a 
debugging aid.

This is the actual def table (some values are hard to query later) so 
if you edit it or if you create this object using a non unique def 
table things can get strange.

]]
box2d.world_functions.info=function(world)

	local info=world.def
	
	for n,v in pairs(core.world_info(world[0])) do
		info[n]=v
	end

	for n,v in pairs(core.world_get(world[0])) do
		info[n]=v
	end

	info.boxid=world.boxid
	
	return info
end

--[[#lua.box2d.world.get

	vars = world:get()

Get all world variables into a table.

]]
box2d.world_functions.get=function(world)
	return core.world_get(world[0])
end

--[[#lua.box2d.world.set

	world:set(vars)

Set all world variables from a table.

]]
box2d.world_functions.set=function(world,vars)
	return core.world_set(world[0],vars)
end

--[[#lua.box2d.world.step

	world:step(seconds,steps)

Advance the world "seconds" through time using "steps" number of 
discreet sub steps.

Generally the simulation expects (1/seconds)*steps to be 240 ish. 
So 240 substeps per second. 1/60 and 4 are the defaults.

]]
box2d.world_functions.step=function(world,seconds,steps)

	core.world_step(world[0],seconds,steps)

end

--[[#lua.box2d.world.prepare_events

	events = world:prepare_events()
	events = world:prepare_events(events)

Prepare a table to be filled with all events.

	events.uids

Lookup table by uid of events by body.uid or shape.uid
A single event may appear multiple times but it will be the same table.

	events.all

A list of all events regardless of a uid being set.

	events:insert(event,ita,itb)
	
Helper function to place an event in uids and all. ita and itb may be 
nil or a shape or a body. We look for ita.uid and itb.uid to fill the 
uids map.

	events.contact_cache

cache of contacts for multiple calls to events.contact to reuse

	events:contact(id)

Get contact information from a contact id, we cache results in 
events.contact_cache so multiple calls will return the same table so is 
safe to call multiple times with the same id.

A contact is a large chunk of possibly unwanted data so has to be 
explicitly requested later using the contactId provided by contact 
events.

May possibly be nil if an invalid contact ID, eg this is old data and 
you called world:step again and created new contacts.

]]
box2d.world_functions.prepare_events=function(world,events)
	if not events then events={} end
	if not events.uids then events.uids={} end
	if not events.all then events.all={} end
	if not events.contact_cache then events.contact_cache={} end 
	if not events.insert then
		events.insert=function(events,event,ita,itb)
			 -- unique userid set it in your bodys / shapes
			if ita and ita.uid then
				if not events.uids[ita.uid] then events.uids[ita.uid]={} end
				table.insert(events.uids[ita.uid],event)
			end
			if itb and itb.uid then
				if not events.uids[itb.uid] then events.uids[itb.uid]={} end
				table.insert(events.uids[itb.uid],event)
			end
			table.insert(events.all,event)
		end
	end
	if not events.contact then
		events.contact=function(events,id)
			if not events.contact_cache[id] then
				events.contact_cache[id]=core.world_contact(id)
			end
			return events.contact_cache[id]
		end
	end
	return events
end
--[[#lua.box2d.world.body_events

	events = world:body_events()
	events = world:body_events(events)

get body events generated by the last step

]]
box2d.world_functions.body_events=function(world,events)

	events=world:prepare_events(events) -- create or reuse

	events.body_move_data =
		core.world_body_events(world[0])
	
	local dat=events.body_move_data
	for idx=0,#dat-1,5 do
		local event={is="body_move"}
		event.bodyId=dat[idx+1]
		event.fellAsleep=dat[idx+2]
		event.transform={dat[idx+3],dat[idx+4],dat[idx+5]}
		
		-- lookup ids
		event.body=world.bodys[event.bodyId]

		events:insert(event,event.body)
	end

	return events
end

--[[#lua.box2d.world.sensor_events

	events = world:sensor_events()
	events = world:sensor_events(events)

get sensor events generated by the last step

]]
box2d.world_functions.sensor_events=function(world,events)

	events=world:prepare_events(events) -- create or reuse

	events.sensor_begin_data , events.sensor_end_data =
		core.world_sensor_events(world[0])

	local dat=events.sensor_begin_data
	for idx=0,#dat-1,2 do
		local event={is="sensor_begin"}
		
		event.sensorShapeId=dat[idx+1]
		event.visitorShapeId=dat[idx+2]

		-- lookup ids
		event.sensorShape=world.shapes[event.sensorShapeId]
		event.visitorShape=world.shapes[event.visitorShapeId]

		events:insert(event,event.sensorShape,visitorShape)
	end

	local dat=events.sensor_end_data
	for idx=0,#dat-1,2 do
		local event={is="sensor_end"}
		
		event.sensorShapeId=dat[idx+1]
		event.visitorShapeId=dat[idx+2]

		-- lookup ids
		event.sensorShape=world.shapes[event.sensorShapeId]
		event.visitorShape=world.shapes[event.visitorShapeId]

		events:insert(event,event.sensorShape,visitorShape)
	end

	return events
end

--[[#lua.box2d.world.contact_events

	events = world:contact_events()
	events = world:contact_events(events)

get contact events generated by the last step

]]
box2d.world_functions.contact_events=function(world,events)

	events=world:prepare_events(events) -- create or reuse

	events.contact_begin_data , events.contact_end_data , events.contact_hit_data =
		core.world_contact_events(world[0])

	local dat=events.contact_begin_data
	for idx=0,#dat-1,3 do
		local event={is="contact_begin"}
		
		event.shapeIdA=dat[idx+1]
		event.shapeIdB=dat[idx+2]
		event.contactId=dat[idx+3]

		-- lookup ids
		event.shapeA=world.shapes[event.shapeIdA]
		event.shapeB=world.shapes[event.shapeIdB]

		events:insert(event,event.shapeA,shapeB)
	end

	local dat=events.contact_end_data
	for idx=0,#dat-1,3 do
		local event={is="contact_end"}
		
		event.shapeIdA=dat[idx+1]
		event.shapeIdB=dat[idx+2]
		event.contactId=dat[idx+3]

		-- lookup ids
		event.shapeA=world.shapes[event.shapeIdA]
		event.shapeB=world.shapes[event.shapeIdB]

		events:insert(event,event.shapeA,shapeB)
	end

	local dat=events.contact_hit_data
	for idx=0,#dat-1,8 do
		local event={is="contact_hit"}
		
		event.shapeIdA=dat[idx+1]
		event.shapeIdB=dat[idx+2]
		event.contactId=dat[idx+3]
		event.approachSpeed=dat[idx+4]
		event.normal={dat[idx+5],dat[idx+6]}
		event.point={dat[idx+7],dat[idx+8]}

		-- lookup ids
		event.shapeA=world.shapes[event.shapeIdA]
		event.shapeB=world.shapes[event.shapeIdB]

		events:insert(event,event.shapeA,shapeB)
	end

	return events
end

--[[#lua.box2d.world.cast_ray

	hits = world:cast_ray(ray)

cast a ray and return an array of hits


	ray.closest

Boolean value, set true to only return the closest hit in hits[1]. 
Otherwise you must iterate over the returned array.


	ray.origin

Starting world point of ray


	ray.translation

The ray to cast


	ray.filter_categoryBits
	ray.filter_maskBits

Filter masks


	hit1=hits[1]
	hit2=hits[2]
	etc

These are not in any useful order, unless you asked for 
the closest in which case there will only be 1.

If hits[1] is nil or #hits is 0 then there where no hits.


	hits.leafVisits
	hits.nodeVisits

Contain extra debug information

]]
box2d.world_functions.cast_ray=function(world,ray)

	local hits=core.world_cast_ray(world[0],ray)
	
	for i,hit in ipairs(hits) do -- auto get shape from id
		if hit.shapeId then hit.shape=world.shapes[hit.shapeId] end
	end

	return hits
end

--[[#lua.box2d.world.overlap_aabb

	overlaps = world:overlap_aabb(aabb)

Get all shapes overlapping aabb , I believe this might get some shapes 
that do not actually overlap the given box so should probably double 
check.


	aabb.origin

Origin vector of bounds


	aabb.lowerBound
	aabb.upperBound

Lower and upper aabb vectors ( will be added to origin )


	aabb.filter_categoryBits
	aabb.filter_maskBits

Filter masks


	overlaps.shapeIds

An array of shapeIds


	overlaps.shapes

An array of shapes


	overlaps.leafVisits
	overlaps.nodeVisits

Extra debug information

]]
box2d.world_functions.overlap_aabb=function(world,aabb)

	local hits=core.world_overlap_aabb(world[0],aabb)
	
	-- convert shapeIds to shapes, no guarantee that these arrays match
	hits.shapes={}
	for i,shapeId in ipairs(hits.shapeIds) do -- auto get shape from id
		hits.shapes[#hits.shapes+1]=world.shapes[shapeId]
	end

	return hits
end

--[[#lua.box2d.world.fill

	def=world:fill(def,name)

Set previously cached default values under name into the def table unless a 
value already exists. This is intended to be used in body/shape/joint 
creation as a way of applying generic settings for each.

def may be nil in which case a new table is created and returned,

If we do not have any cached values for name then def will not be 
altered. 

This is automatically called to fill in body/shape/joint values that have 
previously been cached with the generic name "body"."shape","joint" 
when creating a new body/shape/joint

You may call it explicitly for slightly less generic defaults, eg

	body:shape( world:fill({...},"shape_special") )

Note that when creating objects def should always be a new table as it 
will be kept within the new object and returned by the info function.

]]
box2d.world_functions.fill=function(world,def,name)

	if not def then def={} end
	
	local cache=world.caches[name]
	if cache then
		for n,v in pairs(cache) do
			if type(def[n])=="nil" then -- replace only nil values
				def[n]=v
			end
		end
	end

	return def
end

--[[#lua.box2d.world.fills

	def=world:fills(def,...)

Same as fill except multiple names may be given and each applied from 
left to right. So the left most values will have precedence and missing 
values will be filled in as we travel left to right.

]]
box2d.world_functions.fills=function(world,def,...)

	if not def then def={} end
	
	local names={...}
	
	for _,name in ipairs(name) do
		world:fill(def,name)
	end

	return def
end

--[[#lua.box2d.world.defaults

	cache=world:defaults(name)
	cache=world:defaults(name,def)

Remember default values in the world cache, this is intended for 
generic body/shape/joint creation.

Returns the current def cache for the name.

The def table will be copied ( top level only ) into a cache of the 
given values adding or replacing them.

if def table is nil then the current cached values will be cleared.

For example to cache a constant density value ( because you changed 
the global units per meters )

	world:defaults("shape",{density=1/256})

When shapes are created they will automatically use the "shape" cache 
to fill in missing values. Same for "body" and "joint" caches.

You can also use this to fill in special object defaults like so.

	world:defaults("shape_bouncy",{material_restitution=1})
	...
	body:shape(world:fill({...},"shape_bouncy"))

]] 
box2d.world_functions.defaults=function(world,name,def)
	
	if not def then
		world.caches[name]=nil
		return
	end
	
	if not world.caches[name] then world.caches[name]={} end
	
	local cache=world.caches[name]
	for n,v in pairs(def) do
		cache[n]=v
	end

	return cache
end

--[[#lua.box2d.world.body

	body=world:body(def)

Create a body in the world.

def contains the values that you can set in a b2BodyDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values. This may also be a 
fake table created by lua meta.

Values with _ are sub structs eg motionLocks_linearX is motionLocks.linearX

Booleans should be true or false

name is a string that should be 31 chars or less and is only used to 
help with debugging.


type is a string and must be "static" or "kinematic" or "dynamic"


	allowFastRotation
	angularDamping
	angularVelocity
	enableSleep
	motionLocks_linearX
	motionLocks_linearY
	motionLocks_angularZ
	gravityScale
	isAwake
	isBullet
	isEnabled
	linearDamping
	linearVelocity
	name
	position
	rotation
	sleepThreshold
	type


]]
box2d.world_functions.body=function(world,def)
	local body={}
	
	def=world:fill(def,"body")
	
	body.def=def
	body.world=world
	body.shapes={}

	setmetatable(body,box2d.body_metatable)
	body[0],body.boxid=core.body_create(world[0],def)
	
	world.bodys[ body.boxid ]=body

	return body
end

--[[#lua.box2d.body.destroy

	body:destroy()

Destroy the body and all sub objects (shapes)

]]
box2d.body_functions.destroy=function(body)

	for boxid,shape in pairs(body.shapes) do
		shape:destroy()
	end
	core.body_destroy(body[0])
	body.world.bodys[body.boxid]=nil
end

--[[#lua.box2d.body.info

	info = body:info()

This returns the def table used to create the body along with as much 
updated information as we have available, mostly intended as a 
debugging aid.

This is the actual def table (some values are hard to query later) so 
if you edit it or if you create this object using a non unique def 
table things can get strange.

]]
box2d.body_functions.info=function(body)

	local info=body.def
	
	for n,v in pairs(core.body_info(body[0])) do
		info[n]=v
	end

	for n,v in pairs(core.body_get(body[0])) do
		info[n]=v
	end

	info.boxid=body.boxid
	
	info.type = body:type()
	info.transform = { body:transform() }
	info.velocity = { body:velocity() }
	info.isAwake = body:awake()
	
	return info
end

--[[#lua.box2d.body.get

	vars = body:get()

Get all body variables in a table.

]]
box2d.body_functions.get=function(body)
	return core.body_get(body[0])
end

--[[#lua.box2d.body.set

	body:set(vars)

Set all body variables from a table.

]]
box2d.body_functions.set=function(body,vars)
	return core.body_set(body[0],vars)
end

--[[#lua.box2d.body.type

	btype = body:type()
	btype = body:type(btype)

get/set body type. btype is a string, Possible values are

	static
	kinematic
	dynamic

]]
box2d.body_functions.type=function(body,btype)
	return core.body_type(body[0],btype)
end

--[[#lua.box2d.body.awake

	b = body:awake()
	b = body:awake(b)

get/set body sleeping state.

]]
box2d.body_functions.awake=function(body,b)
	return core.body_awake(body[0],b)
end

--[[#lua.box2d.body.transform

	x,y,r = body:transform()
	x,y,r = body:transform(x,y,r)

get/set body transform

]]
box2d.body_functions.transform=function(body,x,y,r)
	return core.body_transform(body[0],x,y,r)
end

--[[#lua.box2d.body.velocity

	x,y,r = body:velocity()
	x,y,r = body:velocity(x,y,r)

get/set body velocity

]]
box2d.body_functions.velocity=function(body,x,y,r)
	return core.body_velocity(body[0],x,y,r)
end

--[[#lua.box2d.body.force

	body:force()
	body:force(x,y)
	body:force(x,y,r)
	body:force(nil,nil,r)

set body force , note that this does not return the current force and 
calling it without any values will clear the current force.

x,y is force applied to center mass, will be cleared if nil

r is torque , will be cleared if nil

This will replace force and acceleration and as force is applied then 
cleared during a step, this needs to be set for *every* step.

]]
box2d.body_functions.force=function(body,x,y,r)
	return core.body_force(body[0],x,y,r)
end

--[[#lua.box2d.body.acceleration

	body:acceleration()
	body:acceleration(x,y)
	body:acceleration(x,y,r)
	body:acceleration(nil,nil,r)

set body acceleration, which is to say we set the body force multiplied 
by mass or rotational inertia.

x,y is acceleration per second applied to center mass, will be cleared 
if nil

r is rotational acceleration per second in radians , will be cleared if 
nil

This will replace force and acceleration and as force is applied then 
cleared during a step, this needs to be set for *every* step.

]]
box2d.body_functions.acceleration=function(body,x,y,r)
	return core.body_acceleration(body[0],x,y,r)
end

--[[#lua.box2d.world.body.shape

	shape=body:shape(def)

Create a shape in the body.

def contains the values that you can set in a b2ShapeDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values. This may also be a 
fake table created by lua meta.

Booleans should be true or false

Minor safety issue regarding filter bitmasks since lua(jit) uses 
doubles these bit masks should *only* use 52 bits not 64 so the 
integers can fit safely into a double. When using hex the top 3 nibbles 
should always be 0 like so 0x000fffffffffffff that is 13 Fs if you are 
counting.

	density
	enableContactEvents
	enableHitEvents
	enablePreSolveEvents
	enableSensorEvents
	filter
	invokeContactCreation
	isSensor
	material_customColor
	material_friction
	material_restitution
	material_rollingResistance
	material_tangentSpeed
	material_userMaterialId
	updateBodyMass

if def.shape=="circle" then def also contains the values that you can 
set in a b2Circle

	center
	radius

if def.shape=="segment" then def also contains the values that you can 
set in a b2Segment

	point1
	point2

if def.shape=="capsule" then def also contains the values that you can 
set in a b2Capsule

	center1
	center2
	radius

if def.shape=="box" then def also contains the values that you can 
set in a b2Polygon using b2MakeOffsetRoundedBox

	halfWidth
	halfHeight
	center
	rotation
	radius

]]
box2d.body_functions.shape=function(body,def)
	local shape={}
	
	def=body.world:fill(def,"shape")

	shape.def=def
	shape.body=body
	setmetatable(shape,box2d.shape_metatable)
	shape[0],shape.boxid=core.shape_create(body[0],def)
	
	body.shapes[ shape.boxid ]=shape
	body.world.shapes[ shape.boxid ]=shape

	return shape
end

--[[#lua.box2d.shape.destroy

	shape:destroy()

Destroy the shape and all sub objects (none)

]]
box2d.shape_functions.destroy=function(shape)

	core.shape_destroy(shape[0])
	shape.body.shapes[shape.boxid]=nil
	shape.body.world.shapes[shape.boxid]=nil
end

--[[#lua.box2d.shape.info

	info = shape:info()

This returns the def table used to create the shape along with as much 
updated information as we have available, mostly intended as a 
debugging aid.

This is the actual def table (some values are hard to query later) so 
if you edit it or if you create this object using a non unique def 
table things can get strange.

]]
box2d.shape_functions.info=function(shape)

	local info=shape.def
	
	for n,v in pairs(core.shape_info(shape[0])) do
		info[n]=v
	end

	for n,v in pairs(core.shape_get(shape[0])) do
		info[n]=v
	end
	
	info.boxid=shape.boxid

	return info
end

--[[#lua.box2d.shape.get

	vars = shape:get()

Get all shape variables in a table.

]]
box2d.shape_functions.get=function(shape)
	return core.shape_get(shape[0])
end

--[[#lua.box2d.shape.set

	shape:set(vars)

Set all shape variables from a table.

]]
box2d.shape_functions.set=function(shape,vars)
	return core.shape_set(shape[0],vars)
end

--[[#lua.box2d.world.joint

	joint=world:joint(def)

Create a joint in the world.

def contains the values that you can set in a b2jointDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values. This may also be a 
fake table created by lua meta.

Booleans should be true or false

localFrameA/B is a transform which is a vector plus a rotation in 
radians, so { x , y , r }

The boxid of a body ( for bodyIdA and bodyIdB ) can be found in body.boxid

	bodyIdA
	bodyIdB
	localFrameA
	localFrameB
	forceThreshold
	torqueThreshold
	constraintHertz
	constraintDampingRatio
	collideConnected

if def.joint=="distance" then def also contains the values that you can 
set in a b2DistanceJointDef

	length
	enableSpring
	lowerSpringForce
	upperSpringForce
	hertz
	dampingRatio
	enableLimit
	minLength
	maxLength
	enableMotor
	maxMotorForce
	motorSpeed

if def.joint=="motor" then def also contains the values that you can 
set in a b2MotorJointDef

	linearVelocity
	maxVelocityForce
	angularVelocity
	maxVelocityTorque
	linearHertz
	linearDampingRatio
	maxSpringForce
	angularHertz
	angularDampingRatio
	maxSpringTorque

if def.joint=="filter" then def also contains the values that you can 
set in a b2FilterJointDef

	-- no extra values just those found in b2jointDef

if def.joint=="prismatic" then def also contains the values that you can 
set in a b2PrismaticJointDef

	enableSpring
	hertz
	dampingRatio
	targetTranslation
	enableLimit
	lowerTranslation
	upperTranslation
	enableMotor
	maxMotorForce
	motorSpeed

if def.joint=="revolute" then def also contains the values that you can 
set in a b2RevoluteJointDef

	targetAngle
	enableSpring
	hertz
	dampingRatio
	enableLimit
	lowerAngle
	upperAngle
	enableMotor
	maxMotorTorque
	motorSpeed

if def.joint=="weld" then def also contains the values that you can 
set in a b2WeldJointDef

	linearHertz
	angularHertz
	linearDampingRatio
	angularDampingRatio

if def.joint=="wheel" then def also contains the values that you can 
set in a b2WheelJointDef

	enableSpring
	hertz
	dampingRatio
	enableLimit
	lowerTranslation
	upperTranslation
	enableMotor
	maxMotorTorque
	motorSpeed

]]
box2d.world_functions.joint=function(world,def)
	local joint={}
	
	def=world:fill(def,"joint")

	joint.def=def
	joint.world=world
	
	-- auto convert body tables to body boxids
	if type(def.bodyA)=="table" then def.bodyIdA=def.bodyA.boxid end
	if type(def.bodyB)=="table" then def.bodyIdB=def.bodyB.boxid end

	setmetatable(joint,box2d.joint_metatable)
	joint[0],joint.boxid=core.joint_create(world[0],def)

	world.joints[ joint.boxid ]=joint

	return joint
end

--[[#lua.box2d.joint.destroy

	joint:destroy()

Destroy the joint and all sub objects (none)

]]
box2d.joint_functions.destroy=function(joint)

	core.joint_destroy(joint[0])
	joint.world.joint[joint.boxid]=nil
end

--[[#lua.box2d.joint.info

	info = joint:info()

This returns the def table used to create the joint along with as much 
updated information as we have available, mostly intended as a 
debugging aid.

This is the actual def table (some values are hard to query later) so 
if you edit it or if you create this object using a non unique def 
table things can get strange.

]]
box2d.joint_functions.info=function(joint)

	local info=joint.def
	
	for n,v in pairs(core.joint_info(joint[0])) do
		info[n]=v
	end

	for n,v in pairs(core.joint_get(joint[0])) do
		info[n]=v
	end

	-- auto map ids to current body , possibly nil if body is deleted
	if info.bodyIdA then info.bodyA=joint.world.bodys[ info.bodyIdA ] end
	if info.bodyIdB then info.bodyB=joint.world.bodys[ info.bodyIdB ] end

	info.boxid=joint.boxid
	
	return info
end

--[[#lua.box2d.joint.get

	vars = joint:get()

Get all joint variables in a table.

]]
box2d.joint_functions.get=function(joint)
	return core.joint_get(joint[0])
end

--[[#lua.box2d.joint.set

	joint:set(vars)

Set all joint variables from a table.

]]
box2d.joint_functions.set=function(joint,vars)
	return core.joint_set(joint[0],vars)
end
