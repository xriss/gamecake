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
	B.system=function(system) -- bound to zones for scene etc
		local B={} -- fake bake
		return M.bake_system(oven,B,system)
	end
	return B
end

M.bake_system=function(oven,B,system)
local scene=system.scene
local floors=system

B.floors={}
B.floors_metatable={__index=B.floors}

B.floor={}
B.floor_metatable={__index=B.floor}


local gl=oven.gl

local geom=oven.rebake("wetgenes.gamecake.spew.geom")
local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")
local wgrdcanvas=require("wetgenes.grdcanvas")


B.geomfloor=geom.hexafloor({},8192,16)

local siz=0.5

B.system=function(floors)

	setmetatable(floors,B.floors_metatable)

	floors.caste="floor"
	floors.time=0
	floors.y=0

	floors.RND=math.random()

	return floors
end

B.floors.setup=function(floors)

	floors.ramps=wgrdcanvas.ramps(16,{
		{
			{ argb=0xff559933, value=0.0 },
			{ argb=0xff66cc66, value=0.5 },
			{ argb=0xffaacc66, value=1.0 },
		},
		{
			{ argb=0xff330000, value=0.0 },
			{ argb=0xff663333, value=0.5 },
			{ argb=0xff666633, value=1.0 },
		},
	})

	floors.image=oven.cake.images.load("floors/"..tostring(floors),"floors/"..tostring(floors),function() return floors.ramps end)
	floors.image.TEXTURE_WRAP_S		=	gl.CLAMP_TO_EDGE
	floors.image.TEXTURE_WRAP_T		=	gl.CLAMP_TO_EDGE
	floors.image.TEXTURE_MIN_FILTER	=	gl.LINEAR
	floors.image.TEXTURE_MAX_FILTER	=	gl.LINEAR

end
B.floors.clean=function(floors)
	if floors.image then
		oven.cake.images.delete( floors.image )
		floors.image=nil
	end
end


B.floors.update=function(floors)
	floors.time=(floors.time+(1/60))%120
end

--[[

A *big* default floor centered on the camera unless disabled.

]]
B.floors.draw=function(floors)

	if not floors.y then return end -- set to nil to disable

--	local sky=floors.scene.systems.sky

	gl.PushMatrix()

	local camera=floors.scene.get("camera")

	geom.draw(B.geomfloor,"zone_floor_base?RND="..floors.RND,function(p)

		local s=50
		local x=(camera.mtx[9]*-s  + camera.mtx[13])
		local y=(camera.mtx[10]*-s + camera.mtx[14])
		local z=(camera.mtx[11]*-s + camera.mtx[15])


		gl.Uniform4f( p:uniform("offset") , x , floors.y , z , 0 )

		gl.Uniform4f( p:uniform("time") , floors.time,1,1,1 )

--		gl.Uniform4f( p:uniform("sun") , math.pi*2*(((sky.time)/60)%1) , 0 , 0.050 , 0.150 )

--		local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
--		local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()

--		gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )
--		gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )


	end)

	gl.PopMatrix()

end


-- generate any missing boot (json) data
B.floor.gene=function(floor,boot)
	boot=boot or {}
	return boot
end

-- fill in a boot (json) with current state
B.floor.save=function(floor,boot)
	boot=boot or {}
	return boot
end


return B.system(system)
end
