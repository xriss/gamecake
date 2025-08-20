--
-- (C) 2025 Kriss@XIXs.com
--

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local all=M

-- database code
all.cache=all.cache or {}
-- database code
all.db=all.db or {}
-- methods added to scene
all.scene=all.scene or {}


local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end


-- a simple uid->boot cache
all.cache.create=function(cache)
	local cache=cache or {}
	setmetatable(cache,{__index=M.cache})

	cache.uids={} -- main table of cached uid -> boot data

	return cache
end
all.scene.cache=function(scene,cache) return all.cache.create(cache) end

all.cache.get=function(cache,uid)
	if not uid then return end
	return (cache.uids[uid])
end

all.cache.set=function(cache,boot,uid)
	if not uid then uid=boot.uid end
	cache.uids[uid]=boot
end


