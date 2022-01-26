--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local deepcopy=require("wetgenes"):export("deepcopy")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc

B.floors={}
B.floors_metatable={__index=B.floors}

B.floor={}
B.floor_metatable={__index=B.floor}


local gl=oven.gl

local geom=oven.rebake("wetgenes.gamecake.spew.geom")
local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")


B.geomfloor=geom.hexafloor({},8192,16)

local siz=0.5

B.system=function(floors)

	setmetatable(floors,B.floors_metatable)

	floors.caste="floor"
	floors.time=0

	floors.RND=math.random()

	return floors
end

B.floors.update=function(floors)
	floors.time=(floors.time+(1/60))%120
end

B.floors.draw=function(floors)

	local sky=floors.scene.systems.sky

	gl.PushMatrix()

	local camera=floors.scene.get("camera")

	geom.draw(B.geomfloor,"zone_floor_base?RND="..floors.RND,function(p)

		local s=50
		local x=(camera.mtx[9]*-s  + camera.mtx[13])
		local y=(camera.mtx[10]*-s + camera.mtx[14])
		local z=(camera.mtx[11]*-s + camera.mtx[15])


		gl.Uniform4f( p:uniform("offset") , x , 0 , z , 0 )

		gl.Uniform4f( p:uniform("time") , floors.time,1,1,1 )

		gl.Uniform4f( p:uniform("sun") , math.pi*2*(((sky.time)/60)%1) , 0 , 0.050 , 0.150 )

		local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
		local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()

		gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )
		gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )


	end)

	gl.PopMatrix()

end

return B
end
