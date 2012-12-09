

local print=print
local table=table
local pairs=pairs

local core = require("box2d.core")


module("box2d.wrap")

--
-- Call new to get a unique table full of functions associated
-- with lots of tasty up values for easy use
--
-- this gives you a table full of functions to call
--
-- local world=require("box2d.wrap").world()
--
function world(def) -- create a new world

local world={}

	if def then for i,v in pairs(def) do world[i]=v end end -- shallow copy def data 
	
	world.hash={} -- a lookup table to turn internal lightuserdatas into associated lua tables (eg for safe callbacks)

	world.core=core.setup(world) -- allocate a hard core
	core.get(world.core,world) -- update the soft body with current settings from the hard core
	
	function world.delete() -- delete this world
		return core.clean(world.core)
	end
	
	function world.step(tim,iter) -- run the simulation
		return core.step(world.core, tim or 1/50, iter or 10)
	end

	function world.body(def) -- create a body in the world
		local body={}
		body.shapes={}
		body.core = core.body(world.core, def)
		
		function body.delete() -- delete this body
			core.body_delete(world.core, body.core)
		end
	
		function body.get()
			core.body_get(world.core, body.core, body)
		end
		
		function body.set(t)
			core.body_set(world.core, body.core,t)
		end
		
		function body.shape(def)
			world.shape(body,def)
		end
		
		core.body_get(world.core, body.core, body)
			
		return body
	end
	
	function world.shape(body,def) -- create a shape in the body
		local shape={}
		table.insert(body.shapes,shape)
		shape.core = core.body_shape(world.core, body.core , def)
		
		function shape.delete() -- delete this body
			core.body_shape_delete(world.core, body.core, shape.core )
		end
		
		return shape
	end
	
	function world.joint(def) -- create a joint
		local joint={}
		joint.core = core.joint(world.core, def)
		
		function joint.delete() -- delete this joint
			core.joint_delete(world.core, joint.core )
		end
		
		return shape
	end
	
	return world
end

