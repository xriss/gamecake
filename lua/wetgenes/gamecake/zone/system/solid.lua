--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local zips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local wgrdcanvas=require("wetgenes.grdcanvas")


local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

local deepcopy=require("wetgenes"):export("deepcopy")

local LINE=function() return debug.getinfo(2).currentline end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local solids=M

solids.caste="solid"

solids.uidmap={
}

solids.values={

	pos=V3( 0,0,0 ),
	vel=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	ang=V3( 0,0,0 ),

	size=V3( 4,4,4 ),

	color=0.5,
	shade=0.5,
	sides=6,
}


-- methods added to system
solids.system={}
-- methods added to each item
solids.item={}

solids.item.set_values=function(solid)

	solid:set_auto_values()
	solid:set_body_values()

end

solids.item.get_values=function(solid)

	solid:get_auto_values()
	solid:get_body_values()

end

solids.system.setup=function(sys)

	local gl=sys.gl

	sys.wgeom=sys.oven.rebake("wetgenes.gamecake.spew.geom")
	sys.wgeoms=sys.oven.rebake("wetgenes.gamecake.spew.geoms")

	sys.ramps=wgrdcanvas.ramps(16,{
		{ -- silt
			{ argb=0xffcc6600, value=0.0 },
			{ argb=0xff666600, value=0.5 },
			{ argb=0xff663300, value=1.0 },
		},
		{ -- sand
			{ argb=0xffcccccc, value=0.0 },
			{ argb=0xffcccc00, value=0.5 },
			{ argb=0xffcc6600, value=1.0 },
		},
		{ -- mud
			{ argb=0xff442222, value=0.0 },
			{ argb=0xff663333, value=0.5 },
			{ argb=0xff664444, value=1.0 },
		},
		{ -- grass
			{ argb=0xff004422, value=0.0 },
			{ argb=0xff006600, value=0.5 },
			{ argb=0xff336600, value=1.0 },
		},
		{ -- rock
			{ argb=0xff555577, value=0.0 },
			{ argb=0xff555555, value=0.5 },
			{ argb=0xff777755, value=1.0 },
		},
		{ -- snow
			{ argb=0xff6666cc, value=0.0 },
			{ argb=0xffcccccc, value=0.5 },
			{ argb=0xffffffff, value=1.0 },
		},
		{
			{ argb=0xff555577, value=0.0 },
			{ argb=0xff555555, value=0.5 },
			{ argb=0xff777755, value=1.0 },
		},
	})

	sys.image=sys.oven.cake.images.load(sys.caste.."/"..tostring(sys),sys.caste.."/"..tostring(sys),function() return sys.ramps end)
	sys.image.TEXTURE_WRAP_S		=	gl.CLAMP_TO_EDGE
	sys.image.TEXTURE_WRAP_T		=	gl.CLAMP_TO_EDGE
	sys.image.TEXTURE_MIN_FILTER	=	gl.LINEAR
	sys.image.TEXTURE_MAX_FILTER	=	gl.LINEAR

end

solids.system.clean=function(sys)
	if sys.image then
		sys.oven.cake.images.delete( sys.image )
		sys.image=nil
	end
end

solids.item.clean=function(solid)
	if solid.body then
		solid.body:destroy()
		solid.body=nil
	end
end

solids.item.setup=function(solid)
	local sys=solid.sys

	solid:get_values()
	local world=solid:get_singular("kinetic").world

	local shape_mask={
		[20]=sys.wgeom.icosahedron,
		[12]=sys.wgeom.dodecahedron,
		[8]=sys.wgeom.octahedron,
		[6]=sys.wgeom.hexahedron,
		[4]=sys.wgeom.tetrahedron,
	}
	local hedron=shape_mask[solid.sides] or sys.wgeom.tetrahedron

	solid.geom=hedron( {} ):stable_sort()
--[[
	for i,v in ipairs(rock.geom.verts) do
		local s=0.5+(rock.zip.hmap[i]*1.0)	-- scale 0.5 to 1.5
		v[1]=v[1]*s
		v[2]=v[2]*s
		v[3]=v[3]*s
	end
]]


	local m=M4()

--	m:translate(solid.pos)
	m:scale(solid.size)
--	m:rotate(solid.rot)
	sys.wgeom.adjust_by_m4(solid.geom,m)
--	solid.geom:apply_bevel(7/8)
--	solid.geom:build_normals()

--	solid.geom:build_flat_normals()
--	solid.geom:build_normals()
	for i,v in ipairs(solid.geom.verts) do
		v[7]=solid.shade
		v[8]=solid.color
	end

	local center=V3( solid.geom:find_center() )
	solid.geom:adjust_position(-center[1],-center[2],-center[3]) -- center on our position


	local points={}

	for i,v in ipairs(solid.geom.verts) do
		points[#points+1]=v[1]
		points[#points+1]=v[2]
		points[#points+1]=v[3]
	end
--[[
	for _,p in ipairs(solid.geom.polys) do
--		for ti=0,(#p-3) do
		local ti=0
			for i=2,0,-1 do
				local tv=1+ti+i
				if i==0 then tv=1 end
				local v=solid.geom.verts[ p[tv] ]
				points[#points+1]=v[1]
				points[#points+1]=v[2]
				points[#points+1]=v[3]
			end
--		end
	end
]]
	local shape=world:shape("points",points)

--[[
	local shapep=world:shape("points",points)
	local shape=world:shape("compound",{{shapep,0,0,0}})
]]

--[[
	local tp,tv=sys.wgeom.get_colision_tables( solid.geom )
	local mesh=world:mesh( tp,tv )
	local shape=world:shape("mesh",mesh,true)
]]

	solid.body=world:body( "rigid" , shape , solid.size[1]*solid.size[1],  0,0,0, 0x0002 )
	solid.body.item=solid -- backlink for collison

	solid.body:sleep(4,4) -- we are not using meters so scale these defaults
	solid.body:restitution(0.5)
	solid.body:friction(0.5,0.5,0.5)
	solid.body:damping(1/2,1/4) -- do not roll for ever, please stop
	solid:set_body()


	return solid
end


solids.item.update=function(solid)

	solid:get_values()

-- can mess around with body velocity etc here

	local water=solid.pos[2]-solid.scene.systems.water:get_water_height(solid.pos)
	if water <= 0 then water=false end

	if water then
		solid.body:damping(31/32,3/4) -- water friction
		solid.vel[2]=solid.vel[2] - (((water+2)*32)/16) -- use gravity to float
--		local p=V3( solid.body:support(0,1,0) )
--		solid.body:force( 0,water*32,0 , unpack(p-solid.pos) ) -- rotational
	else
		solid.body:damping(1/2,1/4) -- air friction
	end

	solid:set_body()

end


solids.item.update_kinetic=function(solid)

	solid:get_body()
	solid:set_values()

end





solids.item.draw=function(solid)
	local sys=solid.sys

	solid:get_values()
	local dc=solid.scene.drawtween and solid.drawcache or solid

	local gl=solid.gl

	gl.PushMatrix()
	gl.Translate(dc.pos)
	gl.Rotate(dc.rot)

	sys.wgeom.draw(solid.geom,"gamecake_shader?CAM&NORMAL&TEX&TEXNTOON=1.0",function(p)
		gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
		sys.oven.cake.images.bind(solid.sys.image)
		gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
		gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
		gl.ActiveTexture( gl.TEXTURE0 )
	end)

	gl.PopMatrix()

end

