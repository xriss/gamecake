
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.rules

	rules = require("wetgenes.gamecake.fun.yarn.rules").create(items)

This module contains only one function which can be used to create an 
rules instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	local rules={} -- a place to store all rules
	
	rules.metatable={} -- unique meta table everytime we create
	rules.metatable.__index=rules.metatable -- metatable is full of functions
	setmetatable(rules.metatable,items.metatable) -- inherit

	return rules

end
