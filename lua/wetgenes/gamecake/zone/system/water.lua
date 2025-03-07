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
local waters=M

waters.caste="water"

waters.uidmap={
	length=0,
}

waters.values={

	time=0,
	turns=0,
	time_step=1/16,
	turn_step=3/65536,

	pos=V3( 0,0,0 ),
	siz=V3( 1,1,1 ),
	add=V3( 0,0,0 ),

	RND=0,

}

waters.tweens={
	time="tween",
	turns="tween",
	pos="tween",
	siz="tween",
	add="tween",
}

-- methods added to system
waters.system={}
-- methods added to each item
waters.item={}


waters.item.get_values=function(water)

	water:get_auto_values()

end

waters.item.set_values=function(water)

	water:set_auto_values()

end

waters.system.setup=function(sys)

	sys.geom=sys.oven.rebake("wetgenes.gamecake.spew.geom")
	sys.geoms=sys.oven.rebake("wetgenes.gamecake.spew.geoms")

	local gl=sys.oven.gl

	sys.geomwater=sys.geom.hexafloor({},8192,16):subdivide(1024):subdivide(512):subdivide(512):subdivide(256):subdivide(256):subdivide(256):subdivide(256)

	sys.geomwater:subdivide(128)	-- extra?
	sys.geomwater:subdivide(64)	-- extra?
	sys.geomwater:subdivide(32)	-- extra?

	sys.ztri=(8192/16)/(2^7)			-- 4
	sys.Xtri=sys.ztri*math.sqrt(0.75)	-- 3.5ish

end

waters.system.clean=function(sys)

end

-- get the height of the water at this poinr (ignores y) and returns y value
waters.system.get_water_height=function(sys,pos)

	local water=sys.singular
	if water then
		return water.pos[2]
	end

	return 0
end

waters.item.setup=function(water)

	if not water.sys.singular then water.sys.singular=water end

	water:get_values()

	water.time=0
	water.turns=0
	water.time_step=1/16

	water.pos=V3(0,0,0)
	water.siz=V3(1,1,1)
	water.add=V3(0,0,0)

	water.RND=math.random()

	water.turn_step=3/65536

	water.time_radius=1024
	water.time_step_max=1/16
	water.time_step_step=1/1600
	water.time_max=16	-- we will be +- this ish


end

waters.item.update=function(water)

	water:get_values()

	water.turns=( water.turns+water.turn_step )%1

--	print(water.add)

	water.time=water.time+water.time_step
	if water.time>water.time_max            then water.time_step=water.time_step-(water.time_step_step) end
	if water.time<-water.time_max           then water.time_step=water.time_step+(water.time_step_step) end
	if water.time_step>water.time_step_max  then water.time_step=water.time_step_max end
	if water.time_step<-water.time_step_max then water.time_step=-water.time_step_max end

	local camera=water:get_singular("camera")
	if not camera then return end
	local render=camera:depend("render")
	local sky=camera:depend("sky")
	local screen=render.sys.screen


	local c=math.cos(water.turns*math.pi*2)
	local s=math.sin(water.turns*math.pi*2)
	water.add[1]=c*water.time_radius
	water.add[3]=s*water.time_radius

	water.siz[1]=math.abs(c)*1.5+0.5
	water.siz[3]=math.abs(s)*1.5+0.5


	local tide=0
	if sky and sky.moon then
		tide=sky.moon[2]*math.abs(sky.moon[2])*8 -- up/down 16 quims so 4 meters
	end

	local focus=50
	local x=(camera.mtx[9]*-focus  + camera.mtx[13])	-- focus center ahead of camera
	local y=(camera.mtx[10]*-focus + camera.mtx[14])
	local z=(camera.mtx[11]*-focus + camera.mtx[15])

	local snap=4*10
	local sx=math.sqrt(0.75)*snap
	local sz=1*snap

	x=math.floor(0.5+x/sx)*sx	-- snap camera to reduce wobble
--	y=math.floor(0.5+y/snap)*snap
	z=math.floor(0.5+z/sz)*sz

	water.pos[1]=x
	water.pos[2]=tide
	water.pos[3]=z


	water:set_values()

end

waters.item.draw=function(water)

	local sys=water.sys
	local gl=sys.oven.gl

	local camera=water:get_singular("camera")
	local render=camera:depend("render")
	local sky=camera:depend("sky")
	local screen=render.sys.screen

	gl.PushMatrix()

	gl.Color(1,1,1,1.0)

--	print(screen.fbo.frame)
	local watermode="SOFT"
--	if wwin.flavour=="emcc" then -- hack, should test
--		watermode="HARD"
--	end

	if watermode=="SOFT" then
		screen.fbo:snapshot()
	end

	sys.geom.draw(sys.geomwater,"waters?WATER_"..watermode.."&RND="..water.RND..
		(gl.DEPTH_RANGE_REVERSE and "&DEPTH_RANGE_REVERSE" or ""),
			function(p)

		gl.Uniform4f( p:uniform("offset") , water.pos[1] , water.pos[2] , water.pos[3] , 0 )
		gl.Uniform4f( p:uniform("noise_add") , water.add[1] , water.add[2] , water.add[3] , 0 )
		gl.Uniform4f( p:uniform("noise_siz")  , water.siz[1] , water.siz[2] , water.siz[3] , 0 )

--		local s=math.sin((water.time/300)*math.pi*2)*50
		gl.Uniform4f( p:uniform("time") , water.time,1,1,1 )

		local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
		local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()

		gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )
		gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )

		if sky and sky.sun and sky.moon then
			gl.Uniform3f( p:uniform("sun") , sky.sun )
			gl.Uniform3f( p:uniform("moon") , sky.moon )
		end

		if watermode=="SOFT" then
			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			screen.fbo:bind_depth_snapshot()
			gl.Uniform1i( p:uniform("tex_last_depth"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			screen.fbo:bind_texture_snapshot()
			gl.Uniform1i( p:uniform("tex_last_color"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
		end

	end)

	if watermode=="SOFT" then
		screen.fbo:free_snapshot()
	end

	gl.Color(1,1,1,0.25)

	gl.PopMatrix()

end
