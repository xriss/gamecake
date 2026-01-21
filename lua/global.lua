--
-- Simple lock on the global table to stop accidental creation of new gobals
--
-- if you really want to create a new global use global.name=value which bypasses this lock
--
-- locks globals by default but this lock can be turned on/off later on
--
-- this assumes you are not using the newindex metamethod on your global table already or
-- doing anything with function environment so I'm free to set or clear it at will
-- 
--
-- the suggested way to enable global with a fallback to _G so that code remains portable
--
-- local global=_G ; pcall(function() global=require("global") end)
--

-- apply new locking metatable to a table, probably _G
-- this gets does some real meta table shenanigans
local __newindex_create_meta_lock;__newindex_create_meta_lock=function( tab )

	local rawget=rawget
	local rawset=rawset
	local mt=getmetatable(tab) -- use old mt
	if not mt then  -- or use new mt
		mt={}
		setmetatable(tab,mt)
	end

	-- allow raw indexing of original tab and meta through this new cache table and guarantee access to lock manipulation
	local cache={}
	cache.__newindex_locked = function(t,n,v) -- the error function
		error("cannot create new `"..n.."' index in locked table",2)
	end
	cache.__newindex_lock   = function ()	-- set error function
		mt.__newindex = cache.__newindex_locked
		return cache
	end
	cache.__newindex_unlock = function ()	-- clear error function
		mt.__newindex = nil
		return cache
	end
	cache.__newindex_create_meta_lock = __newindex_create_meta_lock -- can apply a new lock to a new table

	mt.__index=mt.__index or {}
	if type(mt.__index)=="table" then -- expose lock manipulation functions
		mt.__index.__newindex_lock=cache.__newindex_lock
		mt.__index.__newindex_unlock=cache.__newindex_unlock
		mt.__index.__newindex_create_meta_lock=cache.__newindex_create_meta_lock
	end

	setmetatable(cache,{
		__index    = tab,
		__newindex = function(t,i,v) rawset(tab,i,v) end,
	})

 -- return new cache table which can now also be used to set values on original table even when locked
 	return cache
end


-- the default module function is bad... replace it with one that is less bad
-- this may technically break some modules but probably wont

_G.module = function(modname, ...)
	local dots={...}
	local ns = {}
	if dots and type(dots[1])=="table" then ns=dots[1] end -- force this tab

	if type(package.loaded[modname])=="table" then error("module name clash "..modname) end -- check
	package.loaded[modname]=ns -- set
	
	if not ns._NAME then
		ns._NAME = modname
		ns._M = ns
		ns._PACKAGE = string.gsub (modname, "[^.]*$", "")
	end
	setfenv (2, ns)
	for i, f in ipairs(dots) do
		if type(f)=="function" then f(ns) end
	end
end


-- switch global lock on and return globals new metatable

local g=__newindex_create_meta_lock(_G)
g.__newindex_lock()
return g
