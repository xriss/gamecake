
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

	rules.names={} -- look up table by name

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.rules.set

	rule = rules.set(rule)

Set this base rule into the name space using rule.name which must 
be a string.

Multiple rules can be applied to an item and each rule will be applied 
in the order listed.

A rule is a table of named functions that can be applied to an item.

	rule.setup(item)

Must setup the item so that it is safe to call the other rules on it.

	rule.clean(item)

Should cleanup anything that needs cleaning.

	rule.tick(item)

Should perform a single time tick update on the item.



]]
-----------------------------------------------------------------------------
	rules.set=function(rule)
		rules.names[assert(rule.name)]=rule
		return rule
	end
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.rules.apply

	rules.apply(item,method,...)

item.rules must be a list of rule names and the order in which they should 
be applied to this item.

Call the given method in each rule with the item and the remaining arguments.

If the method returns a value then it will be returned from this 
function and no more methods will be applied even if more rules are 
listed.

	item:apply(method,...)

This function is inserted into the items.metatable so it can be called 
directly from an item.

]]
-----------------------------------------------------------------------------
	rules.apply=function(item,method,...)
		if not item.rules then return end
		for _,name in ipairs(item.rules) do
			local rule=rules.names[name]
			if rule and rule[method] then
				local ret=rule[method](item,...)
				if ret then return ret end
			end
		end
	end
	items.metatable.apply=rules.apply
	
	return rules

end
