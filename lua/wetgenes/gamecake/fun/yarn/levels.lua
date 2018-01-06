
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.levels

	levels = require("wetgenes.gamecake.fun.yarn.levels").create(items)

This module contains only one function which can be used to create an 
levels instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	local levels={} -- a place to store all levels
	
	levels.metatable={} -- unique meta table everytime we create
	levels.metatable.__index=levels.metatable -- metatable is full of functions
	setmetatable(levels.metatable,items.metatable) -- inherit


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.levels.create

	level = levels.create()
	level = levels.create({})

Create a single level, optionally pass in a base level table that will be 
turned into a proper level (using setmetatable to add methods). This 
should always be a new table and will also be returned.

]]
-----------------------------------------------------------------------------
	levels.create=function(level)
	
		level=items.create(level)
		setmetatable(level,levels.metatable)
		level.class="level"
		
		level.cells=require("wetgenes.gamecake.fun.yarn.cells").create(items,level)
		level.pages=require("wetgenes.gamecake.fun.yarn.pages").create(items,level)
		
		return level
	end

	levels.metatable.get_cell=function(level,cx,cy)
	end


	return levels

end
