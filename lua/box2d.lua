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


--[[#lua.box2d.version

	major,minor,revision = box2d.version()

Get box2d library version, note this is a function.

]]
box2d.version=core.version

--[[#lua.box2d.meter

	box2d.meter(16)

Set units per meter, should be done first before creating worlds.

So we can use pixels for positions and sizes without breaking boxs 
internal meter based constants.

Defaults to 1.

]]
box2d.meter=core.meter

--[[#lua.box2d.world

	world=chipmunk.world()

Create the world you will be simulating physics in.

]]
box2d.world=function(def)
	local world={}
	world.bodies={}
	world.shapes={}

	setmetatable(world,box2d.world_metatable)
	world[0]=core.world_create(world)

-- hack in this worlds default static body
-- we have a special case in the binding that automatically gets the static body from the space ptr
--	world.static=box2d.body(world[0])
--	world.static.in_world=world
	
	return world
end

