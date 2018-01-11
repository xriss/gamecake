
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

	item = rules.apply(item,method,...)

item.rules must be a list of rule names and the order in which they should 
be applied to this item.

Call the given method in each rule with the item and the remaining arguments.

If the method returns a value then no more methods will be applied even 
if more rules are listed.

We always return the passed in item so that calls can be chained.

	item = item:apply(method,...)

This function is inserted into the items.metatable so it can be called 
directly from an item.

]]
-----------------------------------------------------------------------------
	rules.apply=function(item,method,...)
		if not item.rules then return item end
		for _,name in ipairs(item.rules) do
			local rule=rules.names[name]
			if rule and rule[method] then
				if rule[method](item,...) then return item end -- stop if method returns true
			end
		end
		return item
	end
	items.metatable.apply=rules.apply
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.rules.can

	yes = rules.can(item,method)

Returns true if any rule in this item has the given method.

	yes = item:can(method)

This function is inserted into the items.metatable so it can be called 
directly from an item.

]]
-----------------------------------------------------------------------------
	rules.can=function(item,method)
		if not item.rules then return false end
		for _,name in ipairs(item.rules) do
			local rule=rules.names[name]
			if rule and rule[method] then
				return true
			end
		end
		return false
	end
	items.metatable.can=rules.can

	return rules

end
