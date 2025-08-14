--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")


local bullet=require("wetgenes.bullet")


local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local kinetics=M

kinetics.caste="kinetic"

kinetics.uidmap={
	length=0,
}

kinetics.values={
	gravity=V3( 0,100,0 ),
}

-- methods added to system
kinetics.system={}
-- methods added to each item
kinetics.item={}

-- need to cleanup old worlds.

kinetics.item.setup=function(kinetic)

	if not kinetic.sys.singular then kinetic.sys.singular=kinetic end

	kinetic:get_values()

	kinetic.world=bullet.world()

	kinetic.step=(1/16) -- amount of time to step each frame
	kinetic.substeps=4 -- number of substeps
	kinetic.fixedstep=kinetic.step/kinetic.substeps -- actual size of each discrete step

	kinetic.world:gravity( kinetic.gravity[1],kinetic.gravity[2],kinetic.gravity[3] )

	return kinetic
end

kinetics.item.clean=function(kinetic)
	-- we need to call this so every sub part else is destroyed first
	-- otherwise GC might do it in the wrong order if you just rely on it going out of scope
	-- can cause a segfault
	if kinetic.world then
		kinetic.world:destroy()
		kinetic.world=nil
	end
end


kinetics.item.update=function(kinetic)

	kinetic:get_values()

-- this resets gravity ( acc ) for all objects
	kinetic.world:gravity( kinetic.gravity[1],kinetic.gravity[2],kinetic.gravity[3] )

end

kinetics.item.update_kinetic=function(kinetic)

	kinetic.world:step( kinetic.fixedstep , kinetic.substeps )

	if kinetic.watch then
		local it=kinetic.watch
		display(it.caste,it.pos,it.rot,it.vel,it.ang)
	end

end


kinetics.item.draw=function(kinetic)

end
