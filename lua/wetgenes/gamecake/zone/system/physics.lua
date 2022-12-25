--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local deepcopy=require("wetgenes"):export("deepcopy")

local wstr=require("wetgenes.string")
local ls=function(...)print(wstr.dump(...))end


local bullet=require("wetgenes.bullet")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc
	B.system=function(system) -- bound to zones for scene etc
		local B={} -- fake bake
		return M.bake_system(oven,B,system)
	end
	return B
end

M.bake_system=function(oven,B,system)
local scene=system.scene
local physics=system


B.physics={}
B.physics_metatable={__index=B.physics}



B.system=function(physics)

	setmetatable(physics,B.physics_metatable)

	physics.caste="physics"

	physics.world=bullet.world()

	physics.world.maxsteps=16
	physics.world.fixedstep=(1/60)

	physics.world:gravity(0,100,0)

	return physics
end


B.physics.update=function(physics)

	physics.world:step(1/60)

end



-- generate any missing boot (json) data
B.physics.gene=function(physics,boot)
	boot=boot or {}
	return boot
end

-- fill in a boot (json) with current state
B.physics.save=function(physics,boot)
	boot=boot or {}
	return boot
end


return B.system(system)
end
