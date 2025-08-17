
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


--[[#lua.wetgenes.gamecake.zone.scene.require_search

	A table of where to look for system modules

Replace or modify with list of prefxes to try. Defaults to an array
with one entry of "wetgenes.gamecake.zone.system." A string will be
appeneded to the end of these and a require attempted. So it should
terminate in a "." and including a "" on its own will allow a full dot
path to be used as a require since it wont change the appended string.

]]
	scene.require_search={
		"wetgenes.gamecake.zone.system.",
	}
--[[#lua.wetgenes.gamecake.zone.scene.require

	all=scene:require("all")

	Perform a require using scene.require_search as a list of places to
	find the module

]]
	scene.require=function(scene,name)
		local ret
		local ok
		local err=""
		for _,base in ipairs( scene.require_search ) do -- search
			if ret then break end -- done
			local o,e=pcall( function() ret=require(base..name) end ) -- try a require
			if not o then err=err.." "..e end
		end
		assert( ret , name.." not found in scene.require"..err ) -- maybe null
		return ret
	end


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

	local uid = scene:generate_uid()

	local uid = scene:generate_uid(uid)

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
	scene.generate_uid=function(scene,uid)
		if uid then scene.values:set("uid",uid) end
		uid=scene.values:get("uid")
		uid=uid+1
		scene.values:set("uid",uid)
		return uid
	end


--[[#lua.wetgenes.gamecake.zone.scene.remember_uid

	local it = scene:remember_uid( {} )

Remember an item in the uids table, generating a uid and setting it.uid to this
value if one does not already exist.

]]
	scene.remember_uid=function(scene,it)
		it.uid=it.uid or scene:generate_uid()
		scene.uids[it.uid]=it
		return it
	end


--[[#lua.wetgenes.gamecake.zone.scene.forget_uid

	local it = scene:forget_uid( {uid=uid} )

Remove the item from the map of uids. Returns the item or nil if uid was
invalid unset or our map was not pointing to the correct item.

	local it = scene:forget_uid( scene.find_uid( uid ) )

Chain with find_uid to forget an item by uid and this is safe even if the item
does not exist.

]]
	scene.forget_uid=function(scene,it)
		if not it then return nil end
		if not it.uid then return nil end
		if scene.uids[it.uid]~=it then return nil end  -- bad item lookup
		scene.uids[it.uid]=nil -- remove
		return it
	end

--[[#lua.wetgenes.gamecake.zone.scene.find_uid

	local it = scene:find_uid( uid )

Return the item with the given uid or nil if no such item has been
remembered or a nil uid has been passed in.

]]
	scene.find_uid=function(scene,uid)
		if not uid then return nil end
		return scene.uids[uid]
	end


--[[#lua.wetgenes.gamecake.zone.scene.systems.remove

	system = scene:systems_remove(caste)

Remove and return the system of the given caste.

]]
	scene.systems_remove=function(scene,caste)
		scene.systems[caste]=nil
		for i,v in ipairs(scene.systems) do
			if v.caste==caste then
				return table.remove(scene.systems,i)
			end
		end
	end


--[[#lua.wetgenes.gamecake.zone.scene.systems.insert

	scene:systems_insert(system)

Insert a new system replacing any system of the same caste. system.caste should
be set to the caste of the system for this to work.

]]
	scene.systems_insert=function(scene,it)
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
			if av==bv then -- sort by caste name
				return ( a.caste > b.caste ) -- sort backwards
			end
			return ( av > bv ) -- sort backwards
		end)
	end


--[[#lua.wetgenes.gamecake.zone.scene.systems.call

	scene:systems_call(fname,...)

For every system call the function called fname like so.

	system[fname](system,...)

Returns the number of calls made, which will be the number of systems that had
an fname function to call.

If fname is a function then it will be called as if it was a method.

]]
	scene.systems_call=function(scene,fname,...)
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

	scene:systems_cocall(fname,...)

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
	scene.systems_cocall=function(scene,fname,...)
		local functions={}
		local count=0
		if type(fname)=="function" then
			for i=#scene.systems,1,-1 do -- call backwards so item can remove self
				local system=scene.systems[i]
				local fargs={system,...}
				local fcall=fname
				functions[#functions+1]=function() fcall(unpack(fargs)) end
				count=count+1
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

	scene:sortby_update(newtab)

A function that takes the array part of newtab and reverses the
key=value so a simple order list can be provided without any explicit weights.
The first caste name in the array gets a weight of 1, second 2 and so on.

Newtab is coppied into scene.sortby for use a s aweight lookuptable of name to weight.

If newtab is not provided then scene.sortby is used by default.

]]
	scene.sortby_update=function(scene,sortby)
		sortby=sortby or scene.sortby
		for i,v in pairs(sortby) do
			if type(i)=="number" then
				scene.sortby[v]=scene.sortby[v] or i
			else
				scene.sortby[i]=v
			end
		end
	end
	scene:sortby_update(scene.sortby)


--[[#lua.wetgenes.gamecake.zone.scene.reset

	scene:reset()

Empty the list of items to update and draw, this does not reset the systems
table that should be modified with the insert and remove functions.

]]
	scene.reset=function(scene)
		scene.data={} -- main lists of scene
		scene.values=M.create_values() -- values
		scene.values:set("uid",0) -- starting uid
		scene.tweens=M.create_values() -- tweens ( drawing cache of values )
		return scene
	end


--[[#lua.wetgenes.gamecake.zone.scene.caste

	scene:caste(caste)

Get the list of items of a given caste, eg "bullets" or "enemies"

This list will be created if it does not already exist.

scene.sortby is used to keep this list in order and an empty system
will be autocreated if needed.

]]
	scene.caste=function(scene,caste)
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

	scene:add(it)
	scene:add(it,caste)
	scene:add(it,caste,boot)

Add a new item of caste or it.caste to the list of things to update.

The optional boot table contains initialisation/reset values and will
be remembered in item.boot. If boot.id is given then we will remember
this item with a call to scene.set(id,it)

The actual act of initalising the item from the boot table is left to
custom code which ideally should initalize it and then just call
scene.add(it) as the auto shortcuts are unnecesary.

]]
	scene.add=function(scene,it,caste,boot)
		if boot then
			it.boot=boot
			if it.boot.uid   then it.uid   = it.boot.uid    end
			if it.boot.name  then it.name  = it.boot.name   end
			if it.boot.caste then it.caste = it.boot.caste  end
		end
		scene:remember_uid(it)

		-- idealy these should live in a metatable and already be set
		if type(it.scene)=="nil" then it.scene=scene end
		if type(it.caste)=="nil" then it.caste=caste or "generic" end

		if it.name then scene:set( it.name , it ) end

		local items=scene:caste(it.caste)
		items[ #items+1 ]=it -- add to end of items array

		return it
	end


--[[#lua.wetgenes.gamecake.zone.scene.remove

	scene:remove(it)

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
	scene.remove=function(scene,it)
		scene:forget_uid(it)
		if it.id then scene:set( it.id ) end
		local items=scene:caste(it.caste)
		for idx=#items,1,-1 do -- search backwards
			if items[idx]==it then
				table.remove(items,idx)
				return it
			end
		end
	end


--[[#lua.wetgenes.gamecake.zone.scene.call

	scene:call(fname,...)

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
	scene.call=function(scene,fname,...)
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



--[[#lua.wetgenes.gamecake.zone.scene.values_call

	scene:values_call(func,...)

Call this function for every values state tracking entity, so it will 
be called for this scene, for each system and for each item. A values 
state tracking entity contains a values and a tweens member full of 
data.

So as well as explicitly calling on this scene itself we also call the 
scenes systems_call and call functions.

]]
	scene.values_call=function(scene,func,...)
		func(scene,...)
		scene:systems_call(func,...)
		scene:call(func,...)
	end

--[[#lua.wetgenes.gamecake.zone.scene.status

	print( scene:status() )

Return a debug string giving details about the system order and current number
of items of each caste.

]]
	scene.status=function(scene)
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

		local funscene={}
		setmetatable(funscene,{__index=scene})

		funscene.generate_uid=function(uid) return scene.generate_uid(scene,uid) end
		funscene.remember_uid=function(it) return scene.remember_uid(scene,it) end
		funscene.forget_uid=function(it) return scene.forget_uid(scene,it) end
		funscene.find_uid=function(uid) return scene.find_uid(scene,uid) end
		funscene.systems_remove=function(caste) return scene.systems_remove(scene,caste) end
		funscene.systems_insert=function(it) return scene.systems_insert(scene,it) end
		funscene.systems_call=function(fname,...) return scene.systems_call(scene,fname,...) end
		funscene.systems_cocall=function(fname,...) return scene.systems_cocall(scene,fname,...) end
		funscene.sortby_update=function() return scene.sortby_update(scene) end
		funscene.reset=function() return scene.reset(scene) end
		funscene.caste=function(caste) return scene.caste(scene,caste) end
		funscene.add=function(it,caste,boot) return scene.add(scene,caste,boot) end
		funscene.remove=function(it) return scene.remove(scene,it) end
		funscene.call=function(fname,...) return scene.call(scene,fname,...) end
		funscene.status=function() return scene.status(scene) end

		funscene.manifest=function(name,empty) return scene.values:manifest(name,empty) end
		funscene.set=function(name,value) return scene.values:set(name,value) end
		funscene.get=function(name) return scene.values:get(name) end

		funscene.systems.remove=funscene.systems_remove
		funscene.systems.insert=funscene.systems_insert
		funscene.systems.call=funscene.systems_call
		funscene.systems.cocall=funscene.systems_cocall

		return funscene.reset()
	end

-- reset and return the scene, creating the initial data and info tables
	return scene:reset()

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

--[[#lua.wetgenes.gamecake.zone.scene.values.pull

	values:pull()

Merge bottom 2 slots into 1, so shifting all values down one.

]]
values_methods.pull=function(values)
	if #values<2 then return end -- nothing to merge
	local t1=values[1]
	local t2=values[2]
	for n,v in pairs(t2) do -- merge 1 down
		t1[n]=v
	end
	table.remove(values,2) -- remove old slot ( t2 )
end

--[[#lua.wetgenes.gamecake.zone.scene.values.merge

	values:merge()

Merge top 2 slots into 1

]]
values_methods.merge=function(values)
	if #values<2 then return end -- nothing to merge
	local t1=values[#values-1]
	local t2=values[#values] or {}
	for n,v in pairs(t2) do -- merge 1 down
		t1[n]=v
	end
	values[#values]=nil -- remove old slot ( t2 )
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

Remove top pushed slot.

]]
values_methods.unpush=function(values)
	values[#values]=nil
end

--[[#lua.wetgenes.gamecake.zone.scene.values.set

	values:set(key,value)

If this is different from the current value then write the value into
the key of the object at the top of the stack. Hence future gets will
get this new value.

]]
values_methods.set=function(values,key,value)
	if values:get(key)~=value then
		if type(value)=="table" and value.new then -- dupe so we dont accidently mess with the original
			value=value.new(value)
		end
		values[#values][key]=value
	end
	if not values[1][key] then values[1][key]=false end -- make sure all keys are in base
end

--[[#lua.wetgenes.gamecake.zone.scene.values.get

	value=values:get(key)
	value=values:get(key,topidx)

Get the current value for this key by searching from the top of the
stack to the bottom and returning the first non nil value we find.

If topidx is given then get the value at this idx, this would normally
be negative in which it counts back from the end. So it is easy to
time travel back a frame into the cached values with -1 as the idx.

]]
values_methods.get=function(values,key,topidx)
	if not topidx then topidx=#values elseif topidx<=0 then topidx=topidx+#values end
	for idx=topidx,1,-1 do
		local value=values[idx][key]
		local tvalue=type(value)
		if value or (tvalue~="nil") then
			if tvalue=="table" and value.new then -- dupe so we dont accidently mess with the original
				return value.new(value)
			end
			return value
		end
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

	value=values:tween(key,tween)

Tween between the current value(1) and the previous value(0).

A tween of 0 gets the previous value, a tween of 1 gets the current
value and any other tween will get a mix between the two if that is
possible.

We can only mix numbers or tardis values, other values will round to
either 0 or 1 whichever is closest and then get that whole value.

]]
values_methods.tween=function(values,key,tween)
	if (not tween) or (tween>=1) then return values:get(key) end -- shortcut to get

	local a,b
	local len=#values
	a=values[len][key]

	local ta=type(a)
	if ta=="nil" or len==1 then -- both values are the same so can not tween
		return values:get(key)
	else -- got a so find b

		if values[len].notween then return a end -- flag to disable tweening

		for idx=len-1,1,-1 do
			local value=values[idx][key]
			if (type(value)~="nil") then
				b=value
				break
			end
		end
	end

	local tb=type(b)
	if tb=="nil" then
		return values:get(key)
	else
		if ta=="number" and tb=="number" then -- tween numbers
			return a*tween + b*(1-tween)
		elseif tb=="table" and b.mix then -- tween using tardis
			return b:mix(a,tween,b:new())
		end
	end
	if tween<0.5 then return b else return a end -- one or the other
end

--[[#lua.wetgenes.gamecake.zone.scene.values.twrap

	value=values:twrap(key,nmax,tween)

Tween wrap between the current value and the previous value.

Result will be in rang of 0>= n <nmax , blended in the shortest
direction.

nmax may be a number or a tardis value for nmax per dimension

A tween of 0 gets the previous value, a tween of 1 gets the current
value and any other tween will get a mix between the two if that is
possible.

We can only mix numbers or tardis values, other values will round to
either 0 or 1 whichever is closest and then get that whole value.



]]
values_methods.twrap=function(values,key,nmax,tween)

	if (not tween) or (tween>=1) then return values:get(key) end -- shortcut to get

	local a,b
	local len=#values
	a=values[len][key]
	local ta=type(a)
	if ta=="nil" or len==1 then -- both values are the same so can not tween
		return values:get(key)
	else -- got a so find b

		if values[len].notween then return a end -- flag to disable tweening

		for idx=len-1,1,-1 do
			local value=values[idx][key]
			if value or (type(value)~="nil") then
				b=value
				break
			end
		end
	end

	-- tween wrap a snigle number, m must be positive
	local twrap=function(a,b,m,t)
		local tfix=function(n) -- clamp to within range, 0 to m
			if n<0 then return m-((-n)%m)
			else        return n%m        end
		end
		local mo2=(m/2)
		a=tfix(a)
		b=tfix(b)
		local d=a-b
		if     d < -mo2 then a=a+m     -- long way down
		elseif d >  mo2 then b=b+m end -- long way up
		return tfix( a*t + b*(1-t) )
	end

	local tb=type(b)
	if tb~="nil" then
		if ta=="number" and tb=="number" then -- tween numbers

			return twrap(a,b,nmax,tween)

		elseif tb=="table" and b.new then -- tween using tardis
			local r=b.new()
			if type(nmax)=="number" then -- singular nmax
				for i=1,#r do
					r[i]=twrap(a[i],b[i],nmax,tween)
				end
			else -- assume table
				for i=1,#r do
					r[i]=twrap(a[i],b[i],nmax[i],tween)
				end
			end
			return r

		end
	end
	if tween<0.5 then return b else return a end -- one or the other
end


values_methods.save_all=function(values,topidx)
	if not topidx then topidx=#values elseif topidx<=0 then topidx=topidx+#values end
	if topidx > #values then return {} end

	local t={}
	for k,v in pairs( values[1] or {} ) do
		t[k]=values:get(k,topidx)
	end
	return t
end

values_methods.save_diff=function(values,topidx)
	if not topidx then topidx=#values elseif topidx<=0 then topidx=topidx+#values end
	if topidx > #values then return {} end

	local t={}
	for k,v in pairs( values[topidx] or {} ) do
		t[k]=v
	end
	return t
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
