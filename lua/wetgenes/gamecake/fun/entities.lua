
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities

	fun_entities = require("wetgenes.gamecake.fun.entities")

Manage a simple structure to help group, update and draw discrete game 
objects.

This module contains only one function which can be used to create an 
entities instance and the rest of this documentation concerns the 
return from this create function, not the module

	entities = fun_entities.create()
	entities = fun_entities.create( { sortby={"first","second,...} } )
	
Create and return an entities object. A base object can be passed in 
and will be filled in and returned. This can used to supply a sortby 
table to help control the order of updates of different castes.

If passed in sortby is a table of [caste_name]=weight values to help 
control the order of updates by caste and make it consistent. For 
instance you might find it useful to force monsters to update before 
the player.

]]
-----------------------------------------------------------------------------
M.create=function(entities)

	entities=entities or {} -- a place to store everything that needs to be updated

	entities.sortby=entities.sortby or {} -- custom sort weights for update/draw order of each caste

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.sortby_update

	entities.sortby_update()

A function that takes the array part of entities.sortby and reverses 
the key=value so a simple order list can be provided without any 
explicit weights. The first caste name in the array gets a weight of 1, 
second 2 and so on.

]]
-----------------------------------------------------------------------------
	entities.sortby_update=function()
		for i,v in ipairs(entities.sortby) do entities.sortby[v]=i end
	end
	entities.sortby_update()

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.reset

	entities.reset()
	
Empty the list of entites to update and draw

]]
-----------------------------------------------------------------------------
	entities.reset=function()
		entities.info={} -- general data
		entities.data={} -- main lists of entities
		entities.atad={} -- a reverse lookup of the data table
		entities.sorted={} -- a sorted list of entities so we always get the same update order
		return entities
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.caste

	entities.caste(caste)

Get the list of entities of a given caste, eg "bullets" or "enemies"

]]
-----------------------------------------------------------------------------
	entities.caste=function(caste)
		caste=caste or "generic"
		if not entities.data[caste] then -- create on use
			local items={}
			entities.data[caste]=items
			entities.sorted[#entities.sorted+1]=items -- remember in sorted table
			entities.atad[ items ] = caste -- remember caste for later sorting
			table.sort(entities.sorted,function(a,b)
				local av=entities.sortby[ entities.atad[a] ] or math.huge -- get caste then use caste to get sortby weight
				local bv=entities.sortby[ entities.atad[b] ] or math.huge -- put items without a weight last
				return ( av < bv )
			end)
		end
		return entities.data[caste]
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.add

	entities.add(it,caste)
	entities.add(it)

Add a new entity of caste or it.caste to the list of things to update.

]]
-----------------------------------------------------------------------------
	entities.add=function(it,caste)
		caste=caste or it.caste -- probably from item
		caste=caste or "generic"
		local items=entities.caste(caste)
		items[ #items+1 ]=it -- add to end of array
		return it
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.call

	entities.call(fname,...)

for every entity call the function named fname like so it[fname](it,...)

]]
-----------------------------------------------------------------------------
	entities.call=function(fname,...)
		local count=0
		for i=1,#entities.sorted do
			local items=entities.sorted[i]
			for idx=#items,1,-1 do -- call backwards so item can remove self
				local it=items[idx]
				if it[fname] then
					it[fname](it,...)
					count=count+1
				end
			end			
		end
		return count -- number of items called
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.get

	entities.get(name)

get a value previously saved, this is an easy way to find a unique 
entity, eg the global space but it can be used to save any values you 
wish not just to bookmark unique entities.

]]
-----------------------------------------------------------------------------
	entities.get=function(name)
		return entities.info[name]
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.set

	entities.set(name,value)

save a value by a unique name

]]
-----------------------------------------------------------------------------
	entities.set=function(name,value)
		entities.info[name]=value
		return value
	end

-----------------------------------------------------------------------------
--[[#wetgenes.gamecake.fun.entities.manifest

	entities.manifest(name,value)

get a value previously saved, or initalize it to the given value if it 
does not already exist. The default value is {} as this is intended for 
lists.

]]
-----------------------------------------------------------------------------
	entities.manifest=function(name,empty)
		if not entities.info[name] then
			entities.info[name]=empty or {} -- create empty
		end
		return entities.info[name]
	end


-- reset and return the entities, creating the initial data and info tables
	return entities.reset()

end
