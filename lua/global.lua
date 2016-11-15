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

local _G=_G
local rawget=rawget
local rawset=rawset

local g =
{

	__newindex_lock = function ()	-- set error function
		local mt=getmetatable(_G)
			if not mt then
				mt={}
				setmetatable(_G,mt)
			end
			mt.__newindex = function(t,i,v)
				error("cannot create global variable `"..i.."'",2)
			end
		end
	,
	__newindex_unlock = function ()	-- clear error function
		local mt=getmetatable(_G)
			if mt then
				mt.__newindex = null
			end
		end
}
setmetatable(g, {
		__index=function(t,i) return rawget(_G,i) end,
		__newindex=function(t,i,v) rawset(_G,i,v) end,
	})


-- the default module function is bad... replace it with one that is less bad
-- this may technically break some modules but probably wont

local _LOADED = package.loaded
function _G.module (modname, ...)
	local dots={...}
	local ns = {}
	if dots and type(dots[1])=="table" then ns=dots[1] end -- force this tab

	if type(_LOADED[modname])=="table" then error("module name clash "..modname) end -- check
	_LOADED[modname]=ns -- set
	
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

--[[
local req=require
function _G.require(...)
	print("require",...)
	return req(...)
end
]]

-- switch global lock on
g.__newindex_lock()

return g
