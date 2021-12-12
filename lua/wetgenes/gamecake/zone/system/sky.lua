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


B.sky={}
B.sky_metatable={__index=B.sky}

local gl=oven.gl

local geom=oven.rebake("wetgenes.gamecake.spew.geom")
local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")


B.system=function(sky)

	setmetatable(sky,B.sky_metatable)

	sky.caste="sky"
	sky.time_frac=35*24
	sky.time=math.floor(0.5+sky.time_frac)

	sky.RND=math.random()

	return sky
end

B.sky.update=function(sky)
	sky.time_frac=sky.time_frac+(1/60)
	sky.time=math.floor(0.5+sky.time_frac)/24
end

B.sky.draw=function(sky)
--
	gl.state.push()

	gl.state.set({
		[gl.BLEND]						=	gl.FALSE,
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.TRUE,
		[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
		[gl.DEPTH_FUNC]					=	gl.LEQUAL,
	})


	local t={
		-1,	-1,	1,	0,	0, 			
		-1,	 1,	1,	0,	1,
		 1,	-1,	1,	1,	0, 			
		 1,	 1,	1,	1,	1,
	}
	oven.cake.canvas.flat.tristrip("rawuv",t,"zone_sky_base",function(p)
		gl.Uniform4f( p:uniform("time") , sky.time,1,1,1 )

		local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
		local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()

		gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )
		gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )

		gl.Uniform4f( p:uniform("sun") , math.pi*2*(((sky.time)/60)%1) , -(120/360)*math.pi*2 , 0.050 , 0.150 )
	end)

	gl.state.pop()


end

return B
end
