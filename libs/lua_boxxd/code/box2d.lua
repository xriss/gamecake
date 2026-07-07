--
-- (C) 2026 Kriss@XIXs.com
--

--[[#lua.box2d

	local box2d=require("box2d")

We use box2d as the local name of this library.

A lua binding to the box2d physics library

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


--[[#lua.box2d.version

	major,minor,revision = box2d.version()

Get box2d library version, note this is a function.

]]
box2d.version=core.version


--[[#lua.box2d.meter

	units = box2d.meter()
	units = box2d.meter(units)

Get/Set units per meter, should be set first before creating a world.

Defaults to 1.

]]
box2d.meter=core.meter


--[[#lua.box2d.world

	world=box2d.world(def)

Create the world you will be simulating physics in.

def is a table containing the values that you can set in a b2WorldDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values, This may also be a 
fake table created by meta access.

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

	world.bodys={}	-- engrish
	world.joints={}
	world.shapes={}

	setmetatable(world,box2d.world_metatable)
	world[0],world.b2id=core.world_create(def)

	return world
end

--[[#lua.box2d.world.body

	body=world:body(def)

Create a body in the world.

def contains the values that you can set in a b2BodyDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values,

Booleans should be true or false

name is a string that should be 31 chars or less and is only used to 
help with debugging.

Values prefixed by lock_ are found in motionLocks

type is a string and must be "static" or "kinematic" or "dynamic"


	allowFastRotation
	angularDamping
	angularVelocity
	enableSleep
	lock_linearX
	lock_linearY
	lock_angularZ
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
	
	body.world=world
	body.shapes={}

	setmetatable(body,box2d.body_metatable)
	body[0],body.b2id=core.body_create(world[0],def)
	
	world.bodys[ body.b2id ]=body

	return body
end

--[[#lua.box2d.world.joint

	joint=world:joint(def)

Create a joint in the world.

def contains the values that you can set in a b2jointDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values,

Booleans should be true or false

localFrameA/B is a transform which is a vector plus a rotation in 
radians, so { x , y , r }

The ID of a body can be found in body[0]

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
box2d.world_functions.body=function(world,def)
	local joint={}
	
	joint.world=world

	setmetatable(joint,box2d.joint_metatable)
	joint[0],joint.b2id=core.joint_create(world[0],def)

	world.joints[ joint.b2id ]=joint

	return joint
end

--[[#lua.box2d.world.body.shape

	shape=body:shape(def)

Create a shape in the body.

def contains the values that you can set in a b2ShapeDef

All of these are optional so can be nil.

Vectors are a table containing { x , y } values,

Booleans should be true or false

filter is an array of 3 numbers { categoryBits , maskBits , groupIndex 
} and since lua(jit) uses doubles these bit masks should *only* use 52 
bits not 64 so the integers can fit safely into a double. When using 
hex the top 3 nibbles should always be 0 like so 0x000fffffffffffff 
that is 13 Fs if you are counting.

material is a table containing named material values so { friction=1 , 
restitution=1 }

	density
	enableContactEvents
	enableHitEvents
	enablePreSolveEvents
	enableSensorEvents
	filter
	invokeContactCreation
	isSensor
	material.customColor
	material.friction
	material.restitution
	material.rollingResistance
	material.tangentSpeed
	material.userMaterialId
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
	
	shape.body=body

	setmetatable(shape,box2d.shape_metatable)
	shape[0],shape.b2id=core.shape_create(body[0],def)
	
	body.shapes[ shape.b2id ]=shape
	body.world.shapes[ shape.b2id ]=shape

	return shape
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

--[[#lua.box2d.world.body_events

	events = world:body_events()

get body events generated by the last step

]]
box2d.world_functions.body_events=function(world)

	local events={}

	events.move_data=core.world_body_events(world[0])

	return events
end

--[[#lua.box2d.world.sensor_events

	events = world:sensor_events()

get sensor events generated by the last step

]]
box2d.world_functions.sensor_events=function(world)

	local events={}

	events.begin_data,events.end_data=core.world_sensor_events(world[0])

	return events
end

--[[#lua.box2d.world.contact_events

	events = world:contact_events()

get contact events generated by the last step

]]
box2d.world_functions.contact_events=function(world)

	local events={}

	events.begin_data,events.end_data,events.hit_data=core.world_contact_events(world[0])

	return events
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
