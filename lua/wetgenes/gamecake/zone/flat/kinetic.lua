--
-- (C) 2021 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
--     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")


--local bullet=require("wetgenes.bullet")


local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.import=function(_,kinetics)
kinetics=kinetics or {}
kinetics.modname=M.modname

kinetics.caste="kinetic"

kinetics.uidmap={
	length=0,
}

kinetics.values={
	LengthUnitsPerMeter=16,
	step=1/16,
	substeps=16,
	defaults={},
}

-- methods added to system
kinetics.system={}
-- methods added to each item
kinetics.item={}

-- need to cleanup old worlds.

kinetics.item.setup=function(kinetic)

--	if not kinetic.sys.singular then kinetic.sys.singular=kinetic end -- auto set singular

	kinetic:get_values()

	local box2d=require("box2d")
	box2d.set({
		LengthUnitsPerMeter=kinetic.LengthUnitsPerMeter,
	})
	local def={}
	for n,v in pairs(kinetic.defaults.world or {}) do -- copy world defs
		def[n]=v
	end
	kinetic.world=box2d.world(def)
	for n,v in pairs(kinetic.defaults) do
		kinetic.world:defaults(n,v)
	end

	return kinetic
end

kinetics.item.clean=function(kinetic)
	if kinetic.world then
		kinetic.world:destroy()
		kinetic.world=nil
	end
end


kinetics.item.update=function(kinetic)

	kinetic:get_values()

end

kinetics.item.update_kinetic=function(kinetic)

	kinetic:get_values()

	kinetic.world:step(kinetic.step,kinetic.substeps)
	
	-- reset and fill in events from the last step
	-- these are indexed by uid for objects
	-- using the shape.uid if set
	kinetic.events=kinetic.world:prepare_events() -- new events
--	kinetic.world:body_events(kinetic.events)
	kinetic.world:sensor_events(kinetic.events)
	kinetic.world:contact_events(kinetic.events)
	
	PRINT("#events",#kinetic.events.all)
	for i,event in ipairs(kinetic.events.all) do
		PRINT("event.is",event.is)
	end

end


kinetics.item.draw=function(kinetic)

end

	return kinetics
end
