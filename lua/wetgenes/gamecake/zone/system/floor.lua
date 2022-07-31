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
			{ argb=0xff669933, value=0.0 },
			{ argb=0xff66cc66, value=0.5 },
			{ argb=0xff99cc66, value=1.0 },
		},
		{
			{ argb=0xff332222, value=0.0 },
			{ argb=0xff663333, value=0.5 },
			{ argb=0xff665555, value=1.0 },
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

B.floors.create=function(floors,boot)
	local floor={floors=floors}
	setmetatable(floor,B.floor_metatable)
	floors.scene.add( floor , floors.caste , boot )

	floor.pos=V3( boot.pos or {  0,  0,  0} )
	floor.siz=V3( boot.siz or {100,  1,100} )
	floor.rez=V3( boot.rez or {100,  0,100} )

do

	local mix=function(a,b,c)
		local d=1-c
		return a*d+b*c
	end
	local goldie_noise=function(x,y)
		local _,r=math.modf(math.tan(x*y))
		return math.abs(r) -- !=r ? 0.0 : r; // replace nan with 0
	end
	local goldie_noises=function(x,y,s)
		local xs,ys=x/s,y/s
		local fx,fy=math.floor(xs),math.floor(ys)
		local cx,cy=math.ceil(xs),math.ceil(ys)
		local rx,ry=xs-fx,ys-fy
		return mix(
		   mix( goldie_noise(fx,fy) , goldie_noise(cx,fy) , rx ) ,
		   mix( goldie_noise(fx,cy) , goldie_noise(cx,cy) , rx ) ,
		   ry)
	end

	local xp=math.random()*1000
	local yp=math.random()*1000
	floor.geom=geom.trigrid({},floor.rez,function(x,y)
		local r=0
		for i=1,6 do
			r=r+( goldie_noises(300+x+xp,500+y+yp,2^i) * (2^(i))/4 )
		end
		if x==0 or x==100 or y==0 or y==100 then r=-100 end -- edge
		return 8-r
	end)
--	floor.geom=geom.box({},{{-10,-10,-10},{10,10,10}})

end

	
	local m=M4()
	m:translate(floor.pos)
	m:scale(floor.siz)
	geom.adjust_by_m4(floor.geom,m)
	floor.geom:build_normals()
	for i,v in ipairs(floor.geom.verts) do
		local ny=1.0-(-v[5])
		ny=ny*16
		if ny<0 then ny=0 end
		if ny>1 then ny=1 end
		v[7]=0.5
		v[8]=0.25 + (0.5*(ny))
	end

	local world=floors.scene.systems.physics.world
	floor.bodys={}
	local tp,tv=geom.get_colision_tables( floor.geom )
	if #tp>0 then
		local mesh=world:mesh( tp,tv )
		local shape=world:shape("mesh",mesh,true)
		local body=world:body( "rigid" , shape , 0,  0,0,0  , 0x0001 )
		floor.bodys[ #floor.bodys+1 ]=body
		body:restitution( 1 )
		body:friction( 0.8 , 0.1 , 0.1 )
	end

	return floor
end

B.floor.update=function(floor)

end

B.floor.draw=function(floor)

--	gl.PushMatrix()

--	gl.Translate(floor.pos)
--	gl.Color(3/8,6/8,3/8,1)
	geom.draw(floor.geom,"gamecake_shader?XYZ&NORMAL&TEX&TEXNTOON=1.0",function(p)
		gl.ActiveTexture(gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
		oven.cake.images.bind(floor.floors.image)
		gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
		gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
		gl.ActiveTexture( gl.TEXTURE0 )
	end)
--	gl.Color(1,1,1,1)

--	gl.PopMatrix()

end

return B
end
