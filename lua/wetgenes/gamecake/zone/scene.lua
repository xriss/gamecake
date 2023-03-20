
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


--[[#lua.wetgenes.gamecake.zone.scene

	zone_scene = require("wetgenes.gamecake.zone.scene")

Manage a simple structure to help group, update and draw discrete game objects.

This module contains only one function which can be used to create a scene
instance and the rest of this documentation concerns the return from this
create function, not the module

	scene = zone_scene.create()
	scene = zone_scene.create( { sortby={"first","second",...} } )
	
Create and return a scene object. A base object can be passed in and will
be filled in and returned. This can used to supply a sortby table to help
control the order of updates of different castes.

If passed in sortby is a table of [caste_name]=weight values to help control
the order of updates by caste and make it consistent. For instance you might
find it useful to force monsters to update before the player.

]]
M.create=function(scene)

	scene=scene or {} -- a place to store everything that needs to be updated
	
--[[#lua.wetgenes.gamecake.zone.scene.systems

	scene.systems={name=system,[1]=system}

A sorted table and lookup by caste name of each system. Table is sorted so it
will be traversed backwards, backwards traversal allows the current item to
delete itself.

]]
	scene.systems=scene.systems or {} -- global objects for each caste

--[[#lua.wetgenes.gamecake.zone.scene.uids

	scene.uids={[uid]=item,...}

A map of currently remembered uids to items.

]]
	scene.uid=0
	scene.uids=setmetatable({}, { __mode = 'v' }) -- weak values

--[[#lua.wetgenes.gamecake.zone.scene.generate_uid

	local uid = scene.generate_uid()

	local uid = scene.generate_uid(uid)

Simple unique incremental IDs for items to be used instead of 
pointers, starts at 1 and goes up as items are added.

Ideally you will also need extra code for compacting these ids ( 
probably as an export, renumber, import cycle ) to stop them getting 
huge and breaking. So a save/load system which is out of scope for this 
code.

Generally, unless we are creating and destroying millions of items we 
will run out of memory way before this number gets too big.

If a uid is passed in then we will return uid+1 and continue on upwards 
for subsequent calls.

The point of uids is so we can refer to and lazy link items as weak 
values making it easy to delete an item without worrying too much about 
where it has been used. So instead of putting the item table inside 
another item table we can use a uid reference instead.

Good for serialising data as well.

]]
	scene.generate_uid=function(uid)
		if uid then scene.uid=uid end
		scene.uid=scene.uid+1
		return scene.uid
	end


--[[#lua.wetgenes.gamecake.zone.scene.remember_uid

	local it = scene.remember_uid( {} )

Remember an item in the uids table, generating a uid and setting it.uid to this
value if one does not already exist.

]]
	scene.remember_uid=function(it)
		it.uid=it.uid or scene.generate_uid()
		scene.uids[it.uid]=it
		return it
	end


--[[#lua.wetgenes.gamecake.zone.scene.forget_uid

	local it = scene.forget_uid( {uid=uid} )

Remove the item from the map of uids. Returns the item or nil if uid was
invalid unset or our map was not pointing to the correct item.

	local it = scene.forget_uid( scene.find_uid( uid ) )

Chain with find_uid to forget an item by uid and this is safe even if the item
does not exist.

]]
	scene.forget_uid=function(it)
		if not it then return nil end
		if not it.uid then return nil end
		if scene.uids[it.uid]~=it then return nil end  -- bad item lookup
		scene.uids[it.uid]=nil -- remove
		return it
	end

--[[#lua.wetgenes.gamecake.zone.scene.find_uid

	local it = scene.find_uid( uid )

Return the item with the given uid or nil if no such item has been 
remembered or a nil uid has been passed in.

]]
	scene.find_uid=function(uid)
		if not uid then return nil end
		return scene.uids[uid]
	end


--[[#lua.wetgenes.gamecake.zone.scene.systems.remove

	system = scene.systems_remove(caste)

Remove and return the system of the given caste.

]]
	scene.systems_remove=function(caste)
		scene.systems[caste]=nil
		for i,v in ipairs(scene.systems) do
			if v.caste==caste then
				return table.remove(scene.systems,i)
			end
		end
	end
	

--[[#lua.wetgenes.gamecake.zone.scene.systems.insert

	scene.systems_insert(system)

Insert a new system replacing any system of the same caste. system.caste should
be set to the caste of the system for this to work.

]]
	scene.systems_insert=function(it)
--		scene.remember_uid(it)
		if it.caste then
			for i,v in ipairs(scene.systems) do
				if v.caste==it.caste then -- replace
					scene.systems[i]=it
					scene.systems[it.caste]=it
					return
				end
			end
			scene.systems[it.caste]=it
		end
		scene.systems[#scene.systems+1]=it
		table.sort(scene.systems,function(a,b)
			local av=scene.sortby[ a.caste ] or 0 -- use caste to get sortby weight
			local bv=scene.sortby[ b.caste ] or 0 -- put items without a weight last
			return ( av > bv ) -- sort backwards
		end)
	end


--[[#lua.wetgenes.gamecake.zone.scene.systems.call

	scene.systems_call(fname,...)

For every system call the function called fname like so.

	system[fname](system,...)

Returns the number of calls made, which will be the number of systems that had
an fname function to call.

]]
	scene.systems_call=function(fname,...)
		local count=0
		for i=#scene.systems,1,-1 do -- call backwards so item can remove self
			local system=scene.systems[i]
			if system[fname] then
				system[fname](system,...)
				count=count+1
			end
		end
		return count -- number of systems called
	end

--[[#lua.wetgenes.gamecake.zone.scene.systems.cocall

	scene.systems_cocall(fname,...)

For every system call the function called fname inside a coroutine like 
so.

	system[fname](system,...)
	
This function can yield and should do so if it is waiting for another 
system to do something. All coroutines will be run in a round robin 
style until they all complete.

Returns the number of calls made, which will be the number of systems that had
an fname function to call.

]]
	scene.systems_cocall=function(fname,...)
		local functions={}
		local count=0
		for i=#scene.systems,1,-1 do -- call backwards so item can remove self
			local system=scene.systems[i]
			if system[fname] then
				local fargs={system,...}
				local fcall=system[fname]
				functions[#functions+1]=function() fcall(unpack(fargs)) end
				count=count+1
			end
		end
		require("wetgenes.tasks").cocall(functions)
		return count -- number of systems called
	end



	scene.sortby=scene.sortby or {} -- custom sort weights for update/draw order of each caste


--[[#lua.wetgenes.gamecake.zone.scene.sortby_update

	scene.sortby_update()

A function that takes the array part of scene.sortby and reverses the
key=value so a simple order list can be provided without any explicit weights.
The first caste name in the array gets a weight of 1, second 2 and so on.

]]
	scene.sortby_update=function()
		for i,v in pairs(scene.sortby) do if type(i)=="number" then scene.sortby[v]=scene.sortby[v] or i end end
	end
	scene.sortby_update()


--[[#lua.wetgenes.gamecake.zone.scene.reset

	scene.reset()
	
Empty the list of items to update and draw, this does not reset the systems
table that should be modified with the insert and remove functions.

]]
	scene.reset=function()
--		scene.call(function(it) scene.remove(it) end) -- make sure things are removed?
		scene.info={} -- general data
		scene.data={} -- main lists of scene
		return scene
	end


--[[#lua.wetgenes.gamecake.zone.scene.caste

	scene.caste(caste)

Get the list of items of a given caste, eg "bullets" or "enemies"

This list will be created if it does not already exist.

scene.sortby is used to keep this list in order and an empty system 
will be autocreated if needed.

]]
	scene.caste=function(caste)
		caste=caste or "generic"
		if not scene.data[caste] then -- create on use
			if not scene.systems[caste] then -- create system
				scene.systems.insert({caste=caste})
			end
			local items={caste=caste}
			scene.data[caste]=items
		end
		return scene.data[caste]
	end


--[[#lua.wetgenes.gamecake.zone.scene.add

	scene.add(it,caste,boot)
	scene.add(it,caste)
	scene.add(it)

Add a new item of caste or it.caste to the list of things to update.

The optional boot table contains initialisation/reset values and will 
be remembered in item.boot. If boot.id is given then we will remember 
this item with a call to scene.set(id,it)

The actual act of initalising the item from the boot table is left to 
custom code.

]]
	scene.add=function(it,caste,boot)
		it.uid=boot.uid
		scene.remember_uid(it)

		it.scene=scene
		it.boot=boot
		if boot and boot.id then scene.set( boot.id , it ) it.id=boot.id end

		caste=caste or it.caste or (boot and boot.caste) -- probably from item
		caste=caste or "generic"
		it.caste=caste
		local items=scene.caste(caste)
		items[ #items+1 ]=it -- add to end of array
		return it
	end


--[[#lua.wetgenes.gamecake.zone.scene.remove

	scene.remove(it)

Remove this item, this is slightly expensive as we need to search in a table
to find it before calling table.remove which then has to shuffle the table to
fill in the hole.

With very dynamic items it can be faster to allocate all the items you need at
the start and then flag them on/off rather than add and remove dynamically.

It may make more sense to create a system which handles its own list of
objects, such as particles. Then only use the items to keep track of a
master particles item that contains many particles and can add/remove/recycle
as it sees fit.

]]
	scene.remove=function(it)
		scene.forget_uid(it)
		if it.id then scene.set( it.id ) end
		local items=scene.caste(it.caste)
		for idx=#items,1,-1 do -- search backwards
			if items[idx]==it then
				table.remove(items,idx)
				return it
			end
		end	
	end


--[[#lua.wetgenes.gamecake.zone.scene.call

	scene.call(fname,...)

If fname is a string then that method will be invoked for every item 
where it exists like so. Only the first item of each caste is tested, 
if the function exists there then it is expected to exist for all items 
of a given caste. This enables us to skip entire castes whilst still 
making all functions optional.

	if it[fname] then it[fname](it,...) end
	
if fname is a function then it will result in every item geting called with
that function like so.

	fname(it,...)

The calls are always made backwards through the table so that an item can
choose to delete itself.

Finally we return the number of calls we made so you can keep track of
currently active items.

]]
	scene.call=function(fname,...)
		local count=0
		if type(fname)=="function" then
			for i=#scene.systems,1,-1 do
				local sys=scene.systems[i]
				local items=scene.data[ sys.caste ]
				if items then
					for idx=#items,1,-1 do -- call backwards so item can remove self
						local it=items[idx]
						fname(it,...) -- call an explicit function
						count=count+1
					end
				end
			end
		else
			for i=#scene.systems,1,-1 do
				local sys=scene.systems[i]
				local items=scene.data[ sys.caste ]
				if sys and sys[fname] then -- call a system method, if it exists
					sys[fname](sys,...)
					count=count+1
				end
				if items and items[1] and items[1][fname] then -- all items *must* have the same functions available
					for idx=#items,1,-1 do -- call backwards so item can remove self
						local it=items[idx]
						it[fname](it,...)
						count=count+1
					end
				end
			end
		end
		return count -- number of items called
	end


--[[#lua.wetgenes.gamecake.zone.scene.get

	scene.get(name)

get a value previously saved, this is an easy way to find a unique entity, eg
the global space but it can be used to save any values you wish not just to
bookmark unique items.

]]
	scene.get=function(name)
		return scene.info[name]
	end


--[[#lua.wetgenes.gamecake.zone.scene.set

	scene.set(name,value)

save a value by a unique name

]]
	scene.set=function(name,value)
		scene.info[name]=value
		return value
	end


--[[#lua.wetgenes.gamecake.zone.scene.manifest

	scene.manifest(name,value)

get a value previously saved, or initalize it to the given value if it does not
already exist. The default value is {} as this is intended for lists.

]]
	scene.manifest=function(name,empty)
		if not scene.info[name] then
			scene.info[name]=empty or {} -- create empty
		end
		return scene.info[name]
	end

--[[#lua.wetgenes.gamecake.zone.scene.status

	print( scene.status() )

Return a debug string giving details about the system order and current number
of items of each caste.

]]
	scene.status=function()
		local lines={}
		for i=#scene.systems,1,-1 do
			local items=scene.systems[i]
			local datas=scene.data[ items.caste ]
			local count=datas and #datas or 0
			lines[#lines+1]=(items.caste or "").." : "..count
		end
		for n,v in ipairs(scene.info) do
			lines[#lines+1]=tostring(n).." = "..tostring(v)
		end
		return table.concat(lines,"\n")
	end

-- hacks for old fun64 compat which expects these functions in the systems table
	if scene.fun64 then
		scene.systems.remove=scene.systems_remove
		scene.systems.insert=scene.systems_insert
		scene.systems.call=scene.systems_call
		scene.systems.cocall=scene.systems_cocall
	end

-- reset and return the scene, creating the initial data and info tables
	return scene.reset()

end
