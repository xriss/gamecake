--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V1","V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

local wgrdcanvas=require("wetgenes.grdcanvas")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local floors=M

floors.caste="floor"

floors.uidmap={
	length=0,
}

floors.values={
	tile_size=2,

	pos=V3( 0,0,0 ),
	vel=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	ang=V3( 0,0,0 ),

}

-- methods added to system
floors.system={}
-- methods added to each item
floors.item={}

floors.item.get_values=function(floor)

	floor:get_auto_values()
	floor:get_body_values()

end

floors.item.set_values=function(floor)

	floor:set_auto_values()
	floor:set_body_values()

end


floors.system.setup=function(sys)

	sys.geom=sys.oven.rebake("wetgenes.gamecake.spew.geom")
	sys.geoms=sys.oven.rebake("wetgenes.gamecake.spew.geoms")

	local gl=sys.oven.gl

	sys.geomfloor=sys.geom.hexafloor({},8192,16)

	sys.ramps=wgrdcanvas.ramps(16,{
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

	sys.image=sys.oven.cake.images.load("floors/"..tostring(sys),"floors/"..tostring(sys),function() return sys.ramps end)
	sys.image.TEXTURE_WRAP_S		=	gl.CLAMP_TO_EDGE
	sys.image.TEXTURE_WRAP_T		=	gl.CLAMP_TO_EDGE
	sys.image.TEXTURE_MIN_FILTER	=	gl.LINEAR
	sys.image.TEXTURE_MAX_FILTER	=	gl.LINEAR


end

floors.system.clean=function(sys)
	if sys.image then
		sys.oven.cake.images.delete( sys.image )
		sys.image=nil
	end
end


floors.item.setup=function(floor)

	floor:get_values()

	local world=floor:get_singular("kinetic").world


	local shape=world:shape("plane",0,-1,0,0)
	floor.body=world:body( "rigid" , shape , 0,  0,0,0, 0x0001 )

	floor.body.item=floor -- backlink for collison

	floor.body:restitution( 0.5 )
	floor.body:friction( 0.8 , 0.1 , 0.1 )

	floor:set_body()

end

floors.item.update=function(floor)

	floor:get_values()

	floor:set_values()

end

floors.item.draw=function(floor)

	local sys=floor.sys
	local gl=sys.oven.gl

	gl.PushMatrix()

	local camera=floor:get_singular("camera")

	sys.geom.draw(sys.geomfloor,"zone_floor_base",function(p)

		local s=50
		local x=(camera.mtx[9]*-s  + camera.mtx[13])
		local y=(camera.mtx[10]*-s + camera.mtx[14])
		local z=(camera.mtx[11]*-s + camera.mtx[15])

		gl.Uniform4f( p:uniform("offset") , x , floor.pos[2], z , 0 )

	end)

	gl.PopMatrix()

end
