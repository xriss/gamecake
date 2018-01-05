
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.cells

	cells = require("wetgenes.gamecake.fun.yarn.cells").create(items)

This module contains only one function which can be used to create an 
cells instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	local cells={} -- a place to store all cells

	cells.metatable={} -- unique meta table everytime we create
	cells.metatable.__index=cells.metatable -- metatable is full of functions
	setmetatable(cells.metatable,items.metatable) -- inherit

	return cells

end
