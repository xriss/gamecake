
local oldmodule=module
local oldrequire=require

-- unhappy modules, do not use this code with them
local cruft_list={
	["lanes"]=true,
	["socket"]=true,
}

local os=os

-- a small attempt to replace the module function and to impliment live reloading of lua code
-- on demand without restarting a lua state, this is dodgy but rather useful for testing
-- and live server situations where downtime is to be avoided. Small changes can be pushed
-- to a live server seamlessly with a little care.

--
-- This monkey patches module and require when you call overload and is best applied to the master environment
-- as early as possible.
--

-- define functions as local here so they can call each other
local _overload
local _rerequire
local _remodule
local _require
local _module


-- modules loaded before this time whould be reloaded when required
-- defaults to 0, just set to os.time() then rerequire your main module
-- and it should reload every lua module that supports reloading
local reload_time=0

--
-- we are about to use this new module function in this file so it needs to be
-- defined here
--
function _remodule(name)
--print(name)
--print(package.loaded[name])
	local mod = package.loaded[name] -- reuse old environment on a reload
	if mod~="table" then --  or make a new one
		mod={}
	end
	
-- push some extra data into the module, this flags the module as being a remodule
-- and provides what I need to reload it from disk
-- if _MOD_FILENAME is not going to work then do not use this version of module
-- if you are expecting side effects from the normal module function then again
-- do not use this version :)
	mod._MOD_NAME=name
	mod._MOD_FILENAME=debug.getinfo(2).short_src -- where to reload from
	mod._MOD_LOADTIME=os.time() -- the time that we where loaded 

-- dbg
--	print("Loaded module : ".. mod._MOD_FILENAME)

	package.loaded[name] = mod
	if setfenv then setfenv(2, mod) end -- setfenv may not exist in lua 5.2

	return mod
end


--
-- A require function that can reload the lua source when told to do so
--
function _rerequire(s)
	if cruft_list[s] then return _require(s) end -- use old require and module functions

	local oldstate=_overload(false)

	local mod=oldrequire(s)

	if type(mod)=="table" and mod._MOD_LOADTIME and ( mod._MOD_LOADTIME < reload_time ) then -- force reload

		if mod._MOD_FILENAME then
			if not mod._MOD_DISABLE_RELOAD then
				local f,err=loadfile(mod._MOD_FILENAME)
				if f then
					f()
				else
					print(err) -- treat as warning
				end
			end
		end
	
	end

	_overload(oldstate)
	return mod
end

function _require(s)
	local oldstate=_overload(true)
	local mod=oldrequire(s)
	_overload(oldstate)
	return mod
end

_module=oldmodule

--
-- replace global functions, or restore (restore=true)
-- returns true if they where the original functions
--
function _overload(restore)

	
	local oldstate
	
	oldstate=(require==oldrequire) -- true if functions are not replaced
	
	if restore then -- restore

		require=oldrequire
		module=oldmodule
		
	else -- replace
	
		require=_rerequire
		module=_remodule
		
	end

	return oldstate
end


--use all the above locals to create this module

_remodule("wetquire")

_MOD_DISABLE_RELOAD=true -- disable reload of this module

module=_module
require=_require

remodule=_remodule
rerequire=_rerequire

overload=_overload
--overload=function() return true end -- disable

function set_reload_time(t)
	reload_time=t or os.time() -- mark all modules loaded before now as requiring a reload
end
