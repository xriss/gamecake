
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local dotnames=function(name)
	local n,r=name,name
	local f=function(a,b)
		r=n -- start with the full string
		n=n and n:match("^(.+)(%..+)$") -- prepare the parent string
		return r
	end
	return f
end


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.prefabs

	prefabs = require("wetgenes.gamecake.fun.yarn.prefabs").create(items)

This module contains only one function which can be used to create an 
prefabs instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	local prefabs={} -- a place to store all prefabs
	
	prefabs.metatable={} -- unique meta table everytime we create
	prefabs.metatable.__index=prefabs.metatable -- metatable is full of functions
	setmetatable(prefabs.metatable,items.metatable) -- inherit

	prefabs.names={} -- look up table by name

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.prefabs.set

	prefab = prefabs:set(prefab)

Set this base prefab into the name space using prefab.name which must 
be a string.

]]
-----------------------------------------------------------------------------
	prefabs.set=function(prefab)
		prefabs.names[assert(prefab.name)]=prefab
		return prefab
	end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.prefabs.set

	prefab = prefabs:get(name)
	prefab = prefabs:get(name,prefab)

Build and return a table with all prefab values inherited from all its 
parents. Optionally pass in a prefab to override values and also have 
it returned.

If name doesn't exist at any level and a table is not passed in then nil 
will be returned.

Names are hierarchical, separated by dots, for instance if the 
requested name was

	one.two.three.four

then all values form each of these would be merged with the longer 
names having priority over the shorter.

	one.two.three.four
	one.two.three
	one.two
	one


]]
-----------------------------------------------------------------------------
	prefabs.get=function(name,prefab)
		
		for n in dotnames(name) do
			local v=prefabs.names[n]
			if v then -- got some data
				prefab=prefab or {} -- manifest return table
				for sn,sv in pairs(v) do -- shallow copy so best not to use sub tables
					if type(prefab[sn])=="nil" then -- priority to first non nil value
						prefab[sn]=sv
					end
				end
			end
		end
		if prefab then prefab.name=name end -- force the name we asked for
		return prefab
	end

	return prefabs

end
