--
-- (C) 2025 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

-- manage base database which is simply the storage of "boot" jsons quickly referenced by "uid" number

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- methods added to the handle returned from the open function
M.handle={}

-- methods added to the cache returned from the cache function
M.cache={}

M.backend=require("wetgenes.gamecake.zone.system.db_mem")

-- open a database handle using the current backend
M.open=function(...)
	return M.backend.open(...)
end


-- start a db cache using the database handle
M.handle.cache=function(db)
	local cache={}
	setmetatable(cache,{__index=M.cache})

	cache.data={} -- main table of cached uid -> boot data
	cache.find={} -- cached finds
	cache.db=db

	return cache
end

M.cache.get_cache=function(cache,uid)
	if not uid then return end
	return (cache.data[uid])
end

M.cache.set_cache=function(cache,it,uid)
	if not uid then uid=it.uid end
	cache.data[uid]=(it)
end

M.cache.alter_cache=function(cache,it,uid)
	if not uid then uid=it.uid end
	local t=cache:get_cache(uid)
	for n,v in pairs(vals) do
		t[n]=(v)
	end
	cache:set_cache(t)
end

M.cache.get=function(cache,uid)
	if not uid then return end
	local it
	it=cache:get_cache(uid) -- try cache first
	if it or it==false then return it end -- cache worked
	it=cache.db:get(uid) -- try db if value not in cache
	cache:set_cache(it and it or false,uid) -- remember value for next time
	return it
end

M.cache.set=function(cache,it,uid)
	cache:set_cache(it,uid)
	cache.db:set(it,uid)
end

M.cache.alter=function(cache,it,uid)
	cache:alter_cache(it,uid)
	cache.db:alter(it,uid)
end

M.cache.list_uids=function(cache,filter)
	return cache.db:list_uids(filter)
end

M.cache.list=function(cache,filter)
	local list=cache.db:list_uids(filter)
	for i=1,#list do
		list[i]=cache:get(list[i])
	end
	return list
end

-- find a single uid, keeping it in cache so multiple finds will be fast
M.cache.find_uid_set=function(cache,uid,...)
	local filters={...}
	local it=cache.find
	for i=1,#filters,2 do -- update cache with this new value
		local name=filters[i]
		local value=filters[i+1]
		if it then
			if not it[name] then it[name]={} end -- create on use
			it=it[name]
		end
		if it then
			if i>=#filters-1 then -- last one
				it[value]=uid -- so store it here
				it=uid
			else
				if not it[value] then it[value]={} end -- create on use
				it=it[value]
			end
		end
	end
end

M.cache.find_uid=function(cache,...)
	local it=cache.find
	local filters={...}
	for i=1,#filters,2 do
		local name=filters[i]
		local value=filters[i+1]
		if it then it=it[name] end
		if it then it=it[value] end
	end
	if it or it==false then -- cache contained a value
		return it
	end
	local filter={}
	for i=1,#filters,2 do
		local name=filters[i]
		local value=filters[i+1]
		filter[name]=value -- build full filter
	end
	local uid=cache:list_uids(filter)[1] or false	-- fetch real db value ( use false so it can be stored in table )
	cache:find_uid_set(uid,...)

	return uid
end
