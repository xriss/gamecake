


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.gamecake.zone.scene


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



## lua.wetgenes.gamecake.zone.scene.add


	scene.add(it,caste,boot)
	scene.add(it,caste)
	scene.add(it)

Add a new item of caste or it.caste to the list of things to update.

The optional boot table contains initialisation/reset values and will 
be remembered in item.boot. If boot.id is given then we will remember 
this item with a call to scene.set(id,it)

The actual act of initalising the item from the boot table is left to 
custom code.



## lua.wetgenes.gamecake.zone.scene.call


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



## lua.wetgenes.gamecake.zone.scene.caste


	scene.caste(caste)

Get the list of items of a given caste, eg "bullets" or "enemies"

This list will be created if it does not already exist.

scene.sortby is used to keep this list in order and an empty system 
will be autocreated if needed.



## lua.wetgenes.gamecake.zone.scene.find_uid


	local it = scene.find_uid( uid )

Return the item with the given uid or nil if no such item has been 
remembered or a nil uid has been passed in.



## lua.wetgenes.gamecake.zone.scene.forget_uid


	local it = scene.forget_uid( {uid=uid} )

Remove the item from the map of uids. Returns the item or nil if uid was
invalid unset or our map was not pointing to the correct item.

	local it = scene.forget_uid( scene.find_uid( uid ) )

Chain with find_uid to forget an item by uid and this is safe even if the item
does not exist.



## lua.wetgenes.gamecake.zone.scene.generate_uid


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



## lua.wetgenes.gamecake.zone.scene.get


	scene.get(name)

get a value previously saved, this is an easy way to find a unique entity, eg
the global space but it can be used to save any values you wish not just to
bookmark unique items.



## lua.wetgenes.gamecake.zone.scene.manifest


	scene.manifest(name,value)

get a value previously saved, or initalize it to the given value if it does not
already exist. The default value is {} as this is intended for lists.



## lua.wetgenes.gamecake.zone.scene.remember_uid


	local it = scene.remember_uid( {} )

Remember an item in the uids table, generating a uid and setting it.uid to this
value if one does not already exist.



## lua.wetgenes.gamecake.zone.scene.remove


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



## lua.wetgenes.gamecake.zone.scene.reset


	scene.reset()
	
Empty the list of items to update and draw, this does not reset the systems
table that should be modified with the insert and remove functions.



## lua.wetgenes.gamecake.zone.scene.set


	scene.set(name,value)

save a value by a unique name



## lua.wetgenes.gamecake.zone.scene.sortby_update


	scene.sortby_update()

A function that takes the array part of scene.sortby and reverses the
key=value so a simple order list can be provided without any explicit weights.
The first caste name in the array gets a weight of 1, second 2 and so on.



## lua.wetgenes.gamecake.zone.scene.status


	print( scene.status() )

Return a debug string giving details about the system order and current number
of items of each caste.



## lua.wetgenes.gamecake.zone.scene.systems


	scene.systems={name=system,[1]=system}

A sorted table and lookup by caste name of each system. Table is sorted so it
will be traversed backwards, backwards traversal allows the current item to
delete itself.



## lua.wetgenes.gamecake.zone.scene.systems.call


	scene.systems_call(fname,...)

For every system call the function called fname like so.

	system[fname](system,...)

Returns the number of calls made, which will be the number of systems that had
an fname function to call.

If fname is a function then it will be called as if it was a method.



## lua.wetgenes.gamecake.zone.scene.systems.cocall


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



## lua.wetgenes.gamecake.zone.scene.systems.insert


	scene.systems_insert(system)

Insert a new system replacing any system of the same caste. system.caste should
be set to the caste of the system for this to work.



## lua.wetgenes.gamecake.zone.scene.systems.remove


	system = scene.systems_remove(caste)

Remove and return the system of the given caste.



## lua.wetgenes.gamecake.zone.scene.uids


	scene.uids={[uid]=item,...}

A map of currently remembered uids to items.



## lua.wetgenes.gamecake.zone.scene.values


	zone_scene = require("wetgenes.gamecake.zone.scene")

	values = zone_scene.create_values()
	
Create and return a values object.

This is an array of objects representing the main object in values[1] 
and future predictions in values[2],values[3] etc etc.

The key values in the higher numbers overide key values in the lower 
numbers so essentially only changes need to be written into the 
predicted slots.



## lua.wetgenes.gamecake.zone.scene.values.get


	value=values:get(key)

Get the current value for this key by searching from the top of the 
stack to the bottom and returning the first non nil value we find.



## lua.wetgenes.gamecake.zone.scene.values.manifest


	value=values:manifest(key,value)

Get the current value for this key if there is no current value then 
set the current value.

This uses values:get and values:set to perform these actions.



## lua.wetgenes.gamecake.zone.scene.values.new


	values2 = values:new()
	
Create and return a new values object with a single slot containing an 
empty object.



## lua.wetgenes.gamecake.zone.scene.values.push


	values:push()
	
Add a new slot to the end of the values array. Future values will be 
writen into this new slot.



## lua.wetgenes.gamecake.zone.scene.values.set


	values:set(key,value)

If this is different from the current value then write the value into 
the key of the object at the top of the stack. Hence future gets will 
get this new value.



## lua.wetgenes.gamecake.zone.scene.values.tween


	value=values:tween(key)

Tween between the current value and the previous value.

A tween of 0 gets the previous value, a tween of 1 gets the current 
value and any other tween will get a mix between the two if that is 
possible.

We can only mix numbers or tardis values, other values will round to 
either 0 or 1 whichever is closest and then get that whole value.



## lua.wetgenes.gamecake.zone.scene.values.unpush


	values:unpush()
	
Remove all pushed slots except for values[1] This has the effect of 
undoing all changes that have been predicted.

Future values will be writen into the base values[1] slot.

