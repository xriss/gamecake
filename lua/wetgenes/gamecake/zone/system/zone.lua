--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")


local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local zones=M

zones.caste="zone"

zones.uidmap={
	length=0,
}

zones.values={
}

-- methods added to system
zones.system={}
-- methods added to each item
zones.item={}


zones.item.get_values=function(zone)

end

zones.item.setup=function(zone)

	if not zone.sys.singular then zone.sys.singular=zone end -- auto set singular

	zone:get_values()

	return zone
end

zones.item.update=function(zone)

	zone:get_values()

end


zones.item.draw=function(zone)

end
