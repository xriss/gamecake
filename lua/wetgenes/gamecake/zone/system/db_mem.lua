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

-- we are a memory only database, IE non persistant simple and tempory

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- methods added to the handle returned from the open function
M.handle={}


M.open=function(...)

	local db={}

	-- merge our methods
	for n,v in pairs( M.handle ) do
		if not db[n] then db[n]=v end
	end
	-- merge basic methods
	for n,v in pairs( require("wetgenes.gamecake.zone.system.db").handle ) do
		if not db[n] then db[n]=v end
	end

	return db:open(...)
end


M.handle.open=function(db)
	db.data={} -- main table of uid -> boot data
	return db
end

M.handle.close=function(db)
	db.data=nil
end

M.handle.get=function(db,uid)
	if not uid then return end
	return (db.data[uid])
end

M.handle.set=function(db,it,uid)
	if not uid then uid=it.uid end
	db.data[uid]=(it or false) -- dupe the data we store so we do not get in a mess
end

M.handle.alter=function(db,it,uid)
	if not uid then uid=it.uid end
	local t=db:get(uid)
	for n,v in pairs(vals) do
		t[n]=(v)
	end
	db:set(t)
end

-- get a list items that pass the filter, an empty filter gets everything
M.handle.list_uids=function(db,filter)
	local list={}

	for uid,it in pairs(db.data) do
		local pass=true
		for n,v in pairs(filter or {}) do
			if it[n]~=v then
				pass=false
				break
			end
		end
		if pass then
			list[#list+1]=uid
		end
	end

	return list
end
