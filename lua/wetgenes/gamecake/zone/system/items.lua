--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.gamecake.zone.system.items

Generic game objects, IE things that exist in game and can be interacted with 
by the players.

Some times these exist in world, sometimes they exist in bags or inventory etc 
so are not visible world objects.

As such this item *always* exists here and will be moved in and out of world 
objects as necessary.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc
	B.system=function(system) -- bound to zones for scene etc
		local B={} -- fake bake
		return M.bake_system(oven,B,system)
	end
	return B
end

M.bake_system=function(oven,B,system)
local scene=system.scene
local items=system

B.items={}
B.items_metatable={__index=B.items}

B.item={}
B.item_metatable={__index=B.item}

B.can={}
B.can_metatable={__index=B.can}

B.is={}
B.is_metatable={__index=B.is}

--[[

names that have a . in them are sub classes we need to be able to find them 
using their subclass so turn a name with . into a list of possible names the 
last part may be a number in which case it is the pow value

]]
B.name_to_keys=function(name)
	local keys={}
	local i=1
	repeat
		i=name:find(".",i,true)
		if i then
			keys[ #keys+1 ] = name:sub(1,i-1)
			i=i+1
		end
	until not i
	keys[ #keys+1 ]=name -- add all last
	return keys
end


B.system=function(items)

	setmetatable(items,B.items_metatable)

	items.caste="item"

	return items
end


B.items.create=function(items,boot)
	local item={}
	item.items=items
	setmetatable(item,B.item_metatable)
	items.scene.add( item , items.caste , boot )
	
-- this table of functions represent things an item can do
	item.can={} -- functions
	setmetatable(item.can,B.can_metatable) -- generic functions

-- this table of data represent things an item is
	item.is={}  -- data
	setmetatable(item.is,B.is_metatable) -- generic functions

	item:load(boot)

	return item
end

-- check and call an item.can function if it exists
B.item.call=function(item,fn,...)
	if item.can[fn] then return item.can[fn](item,...)
	end
end

-- set a value in item.is possibly by set_function in item.call
B.item.set=function(item,name,value)
	local fn="set_"..name
	if item.can[fn] then return item.can[fn](item,value)
	else                        item.is[name]=value
	end
end
	
-- get a value from item.is possibly by get_function in item.call
B.item.get=function(item,name)
	local fn="get_"..name
	if item.can[fn] then return item.can[fn](item)
	else                 return item.is[name]
	end
end

B.item.load=function(item,data)
	data=data or {}

--	item.rot=Q4( data.rot or {0,0,0,1} )
--	item.pos=V3( data.pos or {0,0,0} )

end

B.item.save=function(item,data)
	data=data or {}

	data[1]=item.caste

--	data.rot={ item.rot[1] , item.rot[2] , item.rot[3] ,  item.rot[4] }
--	data.pos={ item.pos[1] , item.pos[2] , item.pos[3] }

	return data
end

return B.system(system)
end
