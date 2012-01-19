local string=string
local table=table
local math=math

local type=type
local pairs=pairs
local ipairs=ipairs
local tostring=tostring
local setmetatable=setmetatable
local unpack=unpack
local setfenv=setfenv

--
-- Some generic and useful functions, pull them into locals like so:
--[[

local export,lookup,set_env=require("wetgenes"):export("export","lookup","set_env")

]]--

local _ENV=require("wetquire").remodule("wetgenes")
--module("wetgenes")

-----------------------------------------------------------------------------
--
-- export given names from this table as multiple returns
-- can be used to pull function pointers out of a module
--
-- local lookup=require("wetgenes.util"):export("lookup")
--
-- shove it into your module to have a similar effect
--
-----------------------------------------------------------------------------

function export(env,...)
	local tab={}
	for i,v in ipairs{...} do
		tab[i]=env[v]
	end
	return unpack(tab)
end

-----------------------------------------------------------------------------
--
-- safe lookup within a table that returns nil if any part of the lookup is nil
-- so we never cause an error, just returns nil
--
-----------------------------------------------------------------------------
function lookup(tab,...)
	for i,v in ipairs{...} do
		if type(tab)~="table" then return nil end
		tab=tab[v]
	end
	return tab
end


-----------------------------------------------------------------------------
--
-- since setfenv is going byebyes here is a plan to future proof code
--
-- we set the envirtonment of the code that called us only if setfenv exists
-- and we allways return the new environment
--
-- so the following can be used to change the current environment
-- and it should work in lua 5.1 or 5.2
--
-- local _ENV=set_env(new_environment)
--
-- the same is also done in the require overload provided by wetquire.lua
-- so once you wetquire and overload the environment then the following can be used
--
-- local _ENV=module("name")
--
-- or using wetquire without overloading
--
-- local _ENV=require("wetquire").remodule("name")
--
-----------------------------------------------------------------------------
function set_env(env)
	if setfenv then setfenv(2,env) end
	return env
end
