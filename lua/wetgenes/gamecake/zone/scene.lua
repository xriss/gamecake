
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

--[[#lua.wetgenes.gamecake.zone.scene.create_values

	Copy of the create_values function from the main zone_scene module.

]]
	scene.create_values=M.create_values

--[[#lua.wetgenes.gamecake.zone.scene.systems

	scene.infos[caste]=info

User supplied map of static data and functions for each system.

This allows us to easily create other scenes that share all the same 
systems with this scene by reusing this chunk of static data to create 
it.

]]
	scene.infos=scene.infos or {} -- global static infos for each caste

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
		uid=scene.values:get("uid")
		uid=uid+1
		scene.values:set("uid",uid)
		return uid
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
		if it.caste then
			scene.systems[it.caste]=it
			scene.systems[it.caste.."s"]=it		-- simples autos plurals pleases
			for i,v in ipairs(scene.systems) do
				if v.caste==it.caste then -- just replace
					scene.systems[i]=it
					return
				end
			end
		end
		scene.systems[#scene.systems+1]=it -- add new
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

If fname is a function then it will be called as if it was a method.

]]
	scene.systems_call=function(fname,...)
		local count=0
		if type(fname)=="function" then
			for i=#scene.systems,1,-1 do -- call backwards so item can remove self
				local system=scene.systems[i]
				fname(system,...)
				count=count+1
			end
		else
			for i=#scene.systems,1,-1 do -- call backwards so item can remove self
				local system=scene.systems[i]
				if system[fname] then
					system[fname](system,...)
					count=count+1
				end
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

If fname is a function then it will be called as if it was a method.

]]
	scene.systems_cocall=function(fname,...)
		local functions={}
		local count=0
		if type(fname)=="function" then
			for i=#scene.systems,1,-1 do -- call backwards so item can remove self
				local system=scene.systems[i]
				if system[fname] then
					local fargs={system,...}
					local fcall=fname
					functions[#functions+1]=function() fcall(unpack(fargs)) end
					count=count+1
				end
			end
		else
			for i=#scene.systems,1,-1 do -- call backwards so item can remove self
				local system=scene.systems[i]
				if system[fname] then
					local fargs={system,...}
					local fcall=system[fname]
					if fcall then
						functions[#functions+1]=function() fcall(unpack(fargs)) end
						count=count+1
					end
				end
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
		scene.data={} -- main lists of scene
		scene.values=M.create_values() -- values
		scene.values:set("uid",0) -- starting uid
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

	scene.add(it)
	scene.add(it,caste)
	scene.add(it,caste,boot)

Add a new item of caste or it.caste to the list of things to update.

The optional boot table contains initialisation/reset values and will 
be remembered in item.boot. If boot.id is given then we will remember 
this item with a call to scene.set(id,it)

The actual act of initalising the item from the boot table is left to 
custom code which ideally should initalize it and then just call 
scene.add(it) as the auto shortcuts are unnecesary.

]]
	scene.add=function(it,caste,boot)
		if boot then
			it.boot=boot
			if it.boot.uid   then it.uid   = it.boot.uid    end
			if it.boot.name  then it.name  = it.boot.name   end
			if it.boot.caste then it.caste = it.boot.caste  end
		end
		scene.remember_uid(it)

		-- idealy these should live in a metatable and already be set
		if type(it.scene)=="nil" then it.scene=scene end
		if type(it.caste)=="nil" then it.caste=caste or "generic" end

		if it.name then scene.set( it.name , it ) end

		local items=scene.caste(it.caste)
		items[ #items+1 ]=it -- add to end of items array
		
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
		return scene.values:get(name)
	end


--[[#lua.wetgenes.gamecake.zone.scene.set

	scene.set(name,value)

save a value by a unique name

]]
	scene.set=function(name,value)
		return scene.values:set(name,value)
	end


--[[#lua.wetgenes.gamecake.zone.scene.manifest

	scene.manifest(name,value)

get a value previously saved, or initalize it to the given value if it does not
already exist. The default value is {} as this is intended for lists.

]]
	scene.manifest=function(name,empty)
		return scene.values:manifest(name,empty)
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




local values_methods={}
local values_meta={__index=values_methods}

--[[#lua.wetgenes.gamecake.zone.scene.values.new

	values2 = values:new()
	
Create and return a new values object with a single slot containing an 
empty object.

]]
values_methods.new=function()
	local values={ {} }
	setmetatable(values,values_meta)
	return values
end

--[[#lua.wetgenes.gamecake.zone.scene.values.push

	values:push()
	
Add a new slot to the end of the values array. Future values will be 
writen into this new slot.

]]
values_methods.push=function(values)
	values[#values+1]={}
end

--[[#lua.wetgenes.gamecake.zone.scene.values.unpush

	values:unpush()
	
Remove all pushed slots except for values[1] This has the effect of 
undoing all changes that have been predicted.

Future values will be writen into the base values[1] slot.


]]
values_methods.unpush=function(values)
	for idx=#values,2,-1 do
		values[idx]=nil
	end
end

--[[#lua.wetgenes.gamecake.zone.scene.values.set

	values:set(key,value)

If this is different from the current value then write the value into 
the key of the object at the top of the stack. Hence future gets will 
get this new value.

]]
values_methods.set=function(values,key,value)
	if values:get(key)~=value then
		values[#values][key]=value
	end
end

--[[#lua.wetgenes.gamecake.zone.scene.values.get

	value=values:get(key)

Get the current value for this key by searching from the top of the 
stack to the bottom and returning the first non nil value we find.

]]
values_methods.get=function(values,key)
	for idx=#values,1,-1 do
		local value=values[idx][key]
		if value or (type(value)~="nil") then return value end
	end
end

--[[#lua.wetgenes.gamecake.zone.scene.values.manifest

	value=values:manifest(key,value)

Get the current value for this key if there is no current value then 
set the current value.

This uses values:get and values:set to perform these actions.

]]
values_methods.manifest=function(values,key,value)
	local ret=values:get(key) -- get
	if ret or (type(ret)~="nil") then return ret end -- return got
	values:set(key,value) -- set
	return value -- return set
end

--[[#lua.wetgenes.gamecake.zone.scene.values.tween

	value=values:tween(key)

Tween between the current value and the previous value.

A tween of 0 gets the previous value, a tween of 1 gets the current 
value and any other tween will get a mix between the two if that is 
possible.

We can only mix numbers or tardis values, other values will round to 
either 0 or 1 whichever is closest and then get that whole value.

]]
values_methods.tween=function(values,key,tween)
	
	local a,b
	local len=#values
	a=values[len][key]
	if type(a)=="nil" or len==1 then -- both values are the same so can not tween
		return values:get(key)
	else -- got a so find b
		for idx=len-1,1,-1 do
			local value=values[idx][key]
			if value or (type(value)~="nil") then
				b=value
				break
			end
		end		
	end
	
	if values[len].notween then return a end -- flag to disable tweening
	
	if type(b)~="nil" then
		if type(a)=="number" and type(b)=="number" then -- tween numbers
			return a*tween + b*(1-tween)
		elseif type(b)=="table" and b.mix then -- tween using tardis
			return b:mix(a,tween,b:new())
		end
	end
	if tween<0.5 then return b else return a end -- one or the other
end



--[[#lua.wetgenes.gamecake.zone.scene.values

	zone_scene = require("wetgenes.gamecake.zone.scene")

	values = zone_scene.create_values()
	
Create and return a values object.

This is an array of objects representing the main object in values[1] 
and future predictions in values[2],values[3] etc etc.

The key values in the higher numbers overide key values in the lower 
numbers so essentially only changes need to be written into the 
predicted slots.

]]
M.create_values=function()
	return values_methods.new()
end
