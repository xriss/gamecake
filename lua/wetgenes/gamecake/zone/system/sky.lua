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

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local skys=M

skys.caste="sky"

skys.uidmap={
	camera=1,
	length=1,
}

skys.values={
	time_frac=90+45,
	time_speed=360/(20*60*60),
	time_snap=15,

	time=90+45,
	sun=V3(0,0,0),
	moon=V3(0,0,0),
}

skys.types={
	time_frac="get",
	time_speed="get",
	time_snap="get",
	time="tween",
	sun="tween",
	moon="tween",
}

-- methods added to system
skys.system={}
-- methods added to each item
skys.item={}


skys.item.get_values=function(sky)

	sky:get_auto_values()

	sky.time_hour=sky.time_frac/15
	sky.time_minute=(60*(sky.time_hour-math.floor(sky.time_hour)))
	sky.time_second=(60*(sky.time_minute-math.floor(sky.time_minute)))

	sky.time_hour=math.floor(sky.time_hour)
	sky.time_minute=math.floor(sky.time_minute)
	sky.time_second=math.floor(sky.time_second)

end

skys.item.set_values=function(sky)

	sky:set_auto_values()

end

skys.system.setup=function(sys)

	-- oven baked dependencies
	sys.shadow       = sys.oven.rebake("wetgenes.gamecake.zone.shadow")
	sys.screen       = sys.oven.rebake("wetgenes.gamecake.zone.screen")

end


skys.item.setup=function(sky)
	sky.time=0
end

skys.item.update=function(sky)

	sky:get_values()

	sky.time_frac=(sky.time_frac+sky.time_speed ) % (360)

	local time_dest=math.floor((0.5+sky.time_frac)/sky.time_snap)*sky.time_snap
	local d=time_dest-sky.time
	if d> 180 then d=d-360 end
	if d<-180 then d=d+360 end
	sky.time=( sky.time+(d/32) ) % 360

	sky:set_values()

--	sky:update_shadow()

end

skys.item.update_shadow=function(sky)
	local sys=sky.sys
	local shadow=sys.shadow
	local screen=sys.screen

	local camera=sky:depend("camera")

	camera:get_values()
	sky:get_values()

	local s=shadow.maparea -- 40*shadow.mapsize/1024
	local sd=1024*8
	local x=0
	local y=0
	local z=0

	if camera then
		local pos=camera:tget("pos")
		local snap=function(n) return math.floor(n/16)*16 end
		x=(snap(pos[1]))--*-1/s	-- swap z/y as rotation
		y=(snap(pos[2]))--*-1/s
		z=(snap(pos[3]))--*-1/s
		shadow.cam_mtx=camera.mtx
	end

	screen.shader_qs.zone_screen_build_occlusion.SHADOW="0.6,"..(0.04/sd)..","..(0.08/sd)..",0.0"

	local r=(360-90-5)%360
	r=(sky.time+360-90-5)%360

	local  calculate_matrix=function(ang,rot)
		shadow.mtx=M4{
			1,			0,			0,			0,
			0,			1,			0,			0,
			0,			0,			1,			0,
			0,			0,			0,			1,
		}
		shadow.mtx:scale(1/s,1/s,1/sd)
		shadow.mtx:rotate( 90 , 1,0,0 ) -- top down
		shadow.mtx:rotate( ang , 1,0,0 ) -- hemasphere
		shadow.mtx:rotate( -90 + r + rot ,  0,0,1 ) -- time of day
		shadow.mtx:translate(-x,-y,-z)

	end

	-- calculate light matrix
	calculate_matrix(35,0)
	shadow.mtx_sun=shadow.mtx
	-- remember light normal
	sky.sun=V3(-shadow.mtx[3] , -shadow.mtx[7] , -shadow.mtx[11] ):normalize()

	calculate_matrix(0,180)
	shadow.mtx_moon=shadow.mtx
	-- remember light normal
	sky.moon=V3(-shadow.mtx[3] , -shadow.mtx[7] , -shadow.mtx[11] ):normalize()

	local ddd=10

	if r<180 then
		if r<=0+ddd then		screen.day_night[1]=0.5-0.5*(r)/ddd
		elseif r>=180-ddd then	screen.day_night[1]=0.5-0.5*(180-r)/ddd
		else					screen.day_night[1]=0.0
		end
	else
		if r<=180+ddd then		screen.day_night[1]=0.5+0.5*(r-180)/ddd
		elseif r>=360-ddd then	screen.day_night[1]=0.5+0.5*(360-r)/ddd
		else					screen.day_night[1]=1.0
		end
	end

	if r > 180 then -- moon or sun
		shadow.mtx=shadow.mtx_moon
		shadow.light=V3(moon)
	else
		shadow.mtx=shadow.mtx_sun
		shadow.light=V3(sun)
	end

	shadow.power=1

	if r < ddd then
		shadow.power=r/ddd
	end
	if r > 180-ddd then
		shadow.power=(180-r)/ddd
	end
	
	shadow.updated=true
end

skys.item.draw_sky=function(sky)
	local sys=sky.sys
	if not sys.gl then return end -- no gl no draw
	local gl=sys.gl

	sky:get_values()

	gl.state.push()

	gl.state.set({
		[gl.BLEND]						=	gl.FALSE,
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.TRUE,
		[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
	})

if gl.DEPTH_RANGE_REVERSE then
	gl.state.set({
		[gl.DEPTH_FUNC]					=	gl.GEQUAL,
	})
else
	gl.state.set({
		[gl.DEPTH_FUNC]					=	gl.LEQUAL,
	})
end

	gl.Color(1,1,1,1)

	local t={
		-1,	-1,	1,	0,	0,
		-1,	 1,	1,	0,	1,
		 1,	-1,	1,	1,	0,
		 1,	 1,	1,	1,	1,
	}
	sys.oven.cake.canvas.flat.tristrip("rawuv",t,"zone_sky_base",function(p)
		gl.Uniform4f( p:uniform("time") , sky.time,1,1,1 )

		local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
		local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()

		gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )
		gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )

		gl.Uniform3f( p:uniform("sun") , sky.sun )
		gl.Uniform3f( p:uniform("moon") , sky.moon )
	end)

	gl.Color(1,1,1,0.25)

	gl.state.pop()

end


