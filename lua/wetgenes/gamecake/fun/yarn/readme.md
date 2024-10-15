

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.gamecake.fun.yarn.cells


	cells = require("wetgenes.gamecake.fun.yarn.cells").create(items)

This module contains only one function which can be used to create an 
cells instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.items


	items = require("wetgenes.gamecake.fun.yarn.items").create()

This module contains only one function which can be used to create an 
items instance and the rest of this documentation concerns the return 
from this create function, not the module itself.



## lua.wetgenes.gamecake.fun.yarn.items.cells


	items.cells

We automatically create a cells object bound to this set of items, this 
cells object should be used to define all your custom game cells.



## lua.wetgenes.gamecake.fun.yarn.items.create


	item = items.create()
	item = items.create({})
	item = items.create({},metatable)

Create a single item, optionally pass in a base item table that will be 
turned into a proper item (using setmetatable to add methods). This 
should always be a new table and will also be returned. If no 
metatable is provided then items.metatable will be used.



## lua.wetgenes.gamecake.fun.yarn.items.create_pages


	items.pages

We automatically create a pages object bound to this set of items, this 
pages object should be used to define all your custom game pages.

This can be considered a level and you may need multiple pages which 
are moved in and out of items.pages



## lua.wetgenes.gamecake.fun.yarn.items.destroy


	item = items.destroy(item)
	item = item:destroy()

Destroy an item, remove it from the master table dump so it will be 
garbage collected.



## lua.wetgenes.gamecake.fun.yarn.items.find


	child_item = item:find(keyname)

Get the first child item that has a [keyname] value in it. All child 
items are searched, but this is not recursive.

returns nil if no child item is found.



## lua.wetgenes.gamecake.fun.yarn.items.get_big


	big_item = item:get_big()

Get the first big item from this container, returns nil if we do not 
contain a big item.



## lua.wetgenes.gamecake.fun.yarn.items.insert


	item = item:insert(parent)

Insert this item into the given parent. Item will automatically be 
removed from its current parent. If the item is_big then it will be 
inserted into the front of the list otherwise it will go at the end. 
This is to help with finding big items, since there should only be one 
per container we only have to check the first child.



## lua.wetgenes.gamecake.fun.yarn.items.iterate_dotnames


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



## lua.wetgenes.gamecake.fun.yarn.items.iterate_parents


	for it in item:iterate_parents() do
		...
	end

Iterate over the parent chain going upwards. The first iteration is the 
parent of this object and so on.



## lua.wetgenes.gamecake.fun.yarn.items.metatable


	items.metatable

Expose the item metatable so more methods can easily be added.



## lua.wetgenes.gamecake.fun.yarn.items.prefabs


	items.prefabs

We automatically create a prefabs object bound to this set of items, this 
prefabs object should be used to define all your custom game prefabs.



## lua.wetgenes.gamecake.fun.yarn.items.remove


	item = item:remove()

Remove this item from its parents table or do nothing if the item does 
not have a parent.



## lua.wetgenes.gamecake.fun.yarn.items.rules


	items.rules

We automatically create a rules object bound to this set of items, this 
rules object should be used to define all your custom game rules.



## lua.wetgenes.gamecake.fun.yarn.pages


	pages = require("wetgenes.gamecake.fun.yarn.pages").create(items)

This module contains only one function which can be used to create an 
pages instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.prefabs


	prefabs = require("wetgenes.gamecake.fun.yarn.prefabs").create(items)

This module contains only one function which can be used to create an 
prefabs instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.prefabs.set


	prefab = prefabs:get(name)
	prefab = prefabs:get(name,prefab)

Build and return a table with all prefab values inherited from all its 
parents. Optionally pass in a prefab to override values and also have 
it returned.

If name doesn't exist at any level and a table is not passed in then nil 
will be returned.

Names are hierarchical, separated by dots, see items.iterate_dotnames



## lua.wetgenes.gamecake.fun.yarn.rules


	rules = require("wetgenes.gamecake.fun.yarn.rules").create(items)

This module contains only one function which can be used to create an 
rules instance and the rest of this documentation concerns the return 
from this create function, not the module itself.




## lua.wetgenes.gamecake.fun.yarn.rules.apply


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



## lua.wetgenes.gamecake.fun.yarn.rules.can


	yes = rules.can(item,method)

Returns true if any rule in this item has the given method.

	yes = item:can(method)

This function is inserted into the items.metatable so it can be called 
directly from an item.



## lua.wetgenes.gamecake.fun.yarn.rules.set


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


