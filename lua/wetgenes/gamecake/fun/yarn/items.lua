
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items

	items = require("wetgenes.gamecake.fun.yarn.items").create()

This module contains only one function which can be used to create an 
items instance and the rest of this documentation concerns the return 
from this create function, not the module itself.

]]
-----------------------------------------------------------------------------
M.create=function(items)

	items=items or {} -- a place to store everything that exists

	items.dump={} -- all items that have been created
	items.ids={} -- id map
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.iterate_dotnames

	for name,tail in items.iterate_dotnames(names) do ... end

Iterator over a names string, start with the full string and cut off 
the tail on each iteration. This is used for simple inheritance merging 
of named prefabs and rules or anything else.

Second return value is the tail of the string or the string if not 
tail.

for example the following input string
	
	"one.two.three.four"
	
would get you the following iteration loops, one line per loop

	"one.two.three.four" , "four"
	"one.two.three"      , "three"
	"one.two"            , "two"
	"one"                , "one"

]]
-----------------------------------------------------------------------------
	items.iterate_dotnames=function(name)
		local n,r=name,name
		local f=function(a,b)
			r=n -- start with the full string
			n=n and n:match("^(.+)(%..+)$") -- prepare the parent string
			return r,n and r:sub(#n+2) or r
		end
		return f
	end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.metatable

	items.metatable

Expose the item metatable so more methods can easily be added.

]]
-----------------------------------------------------------------------------
	items.metatable={} -- unique meta table every time we create
	items.metatable.__index=items.metatable -- metatable is full of functions
	items.metatable.items=items -- back link

	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.create

	item = items.create()
	item = items.create({})
	item = items.create({},metatable)

Create a single item, optionally pass in a base item table that will be 
turned into a proper item (using setmetatable to add methods). This 
should always be a new table and will also be returned. If no 
metatable is provided then items.metatable will be used.

]]
-----------------------------------------------------------------------------
	items.create=function(it,metatable)
	
		it=it or {}
		setmetatable(it,metatable or items.metatable)
		
		items.dump[it]=true -- a table of *all* items

		if it.id then items.ids[it.id]=it end
	
		it:apply("setup")

		return it
	end
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.destroy

	item = items.destroy(item)
	item = item:destroy()

Destroy an item, remove it from the master table dump so it will be 
garbage collected.

]]
-----------------------------------------------------------------------------
	items.destroy=function(it)
	
		it:apply("clean")

		it:remove() -- make sure it has no parent
		
		items.dump[it]=nil -- remove from garbage dump
		
		if it.id and items.ids[it.id]==it then items.ids[it.id]=nil end
		
		return it

	end
	items.metatable.destroy=items.destroy -- also a method


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.iterate_parents

	for it in item:iterate_parents() do
		...
	end

Iterate over the parent chain going upwards. The first iteration is the 
parent of this object and so on.

]]
-----------------------------------------------------------------------------
	items.metatable.iterate_parents=function(it)
		return function(it,key)
			return key[0]
		end,it,it
	end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.remove

	item = item:remove()

Remove this item from its parents table or do nothing if the item does 
not have a parent.

]]
-----------------------------------------------------------------------------
	items.metatable.remove=function(it)
	
		if it[0] then -- we have a parent
		
			for i,v in ipairs( it[0] ) do
				if v==it then -- found us, so delete it
					table.remove(it[0],i)
					break
				end
			end
			
			it[0]=nil -- we no longer have a parent
		end
		
		return it -- chainable
	end
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.insert

	item = item:insert(parent)

Insert this item into the given parent. Item will automatically be 
removed from its current parent. If the item is_big then it will be 
inserted into the front of the list otherwise it will go at the end. 
This is to help with finding big items, since there should only be one 
per container we only have to check the first child.

]]
-----------------------------------------------------------------------------
	items.metatable.insert=function(it,parent)
	
		it:remove() -- make sure it has no parent
		
		if it.is_big then	table.insert(parent,1,it) -- big items are first
		else				table.insert(parent,it)   -- small items are last
		end

		it[0]=parent
		
		return it -- chainable
	end
	
-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.get_big

	big_item = item:get_big()

Get the first big item from this container, returns nil if we do not 
contain a big item.

]]
-----------------------------------------------------------------------------
	items.metatable.get_big=function(it)
			
		return it[1] and it[1].is_big and it[1]
		
	end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.find

	child_item = item:find(keyname)

Get the first child item that has a [keyname] value in it. All child 
items are searched, but this is not recursive.

returns nil if no child item is found.

]]
-----------------------------------------------------------------------------
	items.metatable.find=function(it,keyname)
		
		for i,v in ipairs(it) do
			if v[keyname] then return v end
		end
		
	end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.rules

	items.rules

We automatically create a rules object bound to this set of items, this 
rules object should be used to define all your custom game rules.

]]
-----------------------------------------------------------------------------
	items.rules=require("wetgenes.gamecake.fun.yarn.rules").create(items)

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.prefabs

	items.prefabs

We automatically create a prefabs object bound to this set of items, this 
prefabs object should be used to define all your custom game prefabs.

]]
-----------------------------------------------------------------------------
	items.prefabs=require("wetgenes.gamecake.fun.yarn.prefabs").create(items)

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.cells

	items.cells

We automatically create a cells object bound to this set of items, this 
cells object should be used to define all your custom game cells.

]]
-----------------------------------------------------------------------------
	items.cells=require("wetgenes.gamecake.fun.yarn.cells").create(items)

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.items.create_pages

	items.pages

We automatically create a pages object bound to this set of items, this 
pages object should be used to define all your custom game pages.

This can be considered a level and you may need multiple pages which 
are moved in and out of items.pages

]]
-----------------------------------------------------------------------------
	items.pages=require("wetgenes.gamecake.fun.yarn.pages").create(items)


	return items

end
