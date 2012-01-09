-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local stashdata=require("wetgenes.www.any.stashdata")


-- a stash is a simple long term cache, it lives in data entities and survives reboots

module(...)


-----------------------------------------------------------------------------
--
-- clear all stashed data, may fail...
-- everything in the stash should be recreatable
--
-----------------------------------------------------------------------------
function clear(srv,id)

	return false --not gonna do this yet, appengine has issues anyhow
end

-----------------------------------------------------------------------------
--
-- delete id from stash
--
-----------------------------------------------------------------------------
function del(srv,id)
	stashdata.del(srv,id)
end

-----------------------------------------------------------------------------
--
-- put id in stash
-- data is data to store (should be json encodable)
-- opts is a table of extra options
--
-----------------------------------------------------------------------------
function put(srv,id,data,opts)
	local e=stashdata.set(srv,id,function(srv,e)
		e.cache.data=data
		if opts then
			e.cache.base=opts.base
			e.cache.func=opts.func
		end
		return e
	end)
	if e then
		return e.cache.data, e
	end	
end

-----------------------------------------------------------------------------
--
-- get id from stash
-- return data,entity
-- the entity can be used for extra validity checks of the date (IE last update  time)
--
-----------------------------------------------------------------------------
function get(srv,id)
	local e=stashdata.get(srv,id)
	if e then
		return e.cache.data, e
	end
end
