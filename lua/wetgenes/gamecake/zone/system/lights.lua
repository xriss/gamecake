--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local deepcopy=require("wetgenes"):export("deepcopy")

local wstr=require("wetgenes.string")
local ls=function(...)print(wstr.dump(...))end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc

B.lights={}
B.lights_metatable={__index=B.lights}

B.light={}
B.light_metatable={__index=B.light}

local gl=oven.gl


B.system=function(lights)

	setmetatable(lights,B.lights_metatable)

	lights.caste="light"

	return lights
end



B.lights.create=function(lights,boot)
	local light={}
	light.lights=lights
	setmetatable(light,B.light_metatable)
	item.scene.add( item , lights.caste , boot )

	light:load(boot)

	return light
end


B.light.load=function(light,data)
	data=data or {}

	light.rot=Q4( data.rot or {0,0,0,1} )
	light.pos=V3( data.pos or {0,0,0} )

end

B.light.save=function(light,data)
	data=data or {}

	data[1]=light.caste

	data.rot={ light.rot[1] , light.rot[2] , light.rot[3] ,  light.rot[4] }
	data.pos={ light.pos[1] , light.pos[2] , light.pos[3] }

	return data
end

return B
end
