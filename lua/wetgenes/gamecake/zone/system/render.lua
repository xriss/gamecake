--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wstr=require("wetgenes.string")

local deepcopy=require("wetgenes"):export("deepcopy")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local renders=M

renders.caste="render"

renders.uidmap={
	parent=1,
	camera=2,
	length=2,
}

renders.values={
	pos=V2(0,0),	-- position of render in window space
	siz=V2(1,1),	-- size of render in window space
}

-- methods added to system
renders.system={}

renders.system.setup=function(sys)
	if not sys.gl then return end -- no gl no draw

	-- oven baked dependencies
	sys.shadow       = sys.oven.rebake("wetgenes.gamecake.zone.shadow")
	sys.screen       = sys.oven.rebake("wetgenes.gamecake.zone.screen")
	sys.framebuffers = sys.oven.rebake("wetgenes.gamecake.framebuffers")

	sys.shadow.mapsize=2048
	sys.shadow.maparea=256

end

-- methods added to each item
renders.item={}

renders.item.setup=function(render)
	local sys=render.sys
	if not sys.gl then return end -- no gl no draw
	local gl=sys.gl

	render:get_values()

	-- need an fbo to render into
	render.fbo=sys.framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })
	render.view=sys.oven.cake.views.create({
		mode="fbo",
		fbo=render.fbo,
		vz=8192,
		fov=1/2,
		cx=0.5,cy=0.5,cz=0.5,
	})

	-- need a memory of bloom
	render.fbo_bloom=sys.framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })

end

renders.item.clean=function(render)

	if render.fbo then
		render.fbo:clean()
	end
	if render.fbo_bloom then
		render.fbo_bloom:clean()
	end

end


renders.item.render_camera=function(render)
	local sys=render.sys
	if not sys.gl then return end -- no gl no draw
	local gl=sys.gl
	local scene=sys.scene

	local camera=render:depend("camera")
	local sky=camera:depend("sky")
	local water=camera:depend("water")
	camera.sys.singular=camera -- remember this camera

	render:get_values()

	render.fbo_bloom=sys.screen:swap_bloom(render.fbo_bloom)

	sky:update_shadow()

	if camera then
		camera:set_active() -- perform tween update here
		-- rotate bloom with camera movement ( barely noticeable unless you squish one of the axis)
		sys.screen.blurd1=V3(1,0,0)*Q4("z", (camera.tilt+camera.direction)* 1 )
		sys.screen.blurd2=V3(0,1/2,0)*Q4("z", (camera.tilt+camera.direction)* 1 )
	else
		local blur_rot=Q4("z",45)
		sys.screen.blurd1=V3(1,0,0)*blur_rot
		sys.screen.blurd2=V3(0,1,0)*blur_rot
	end

	for idx=1,sys.shadow.count do
		gl.PushMatrix()
		sys.shadow.draw_head(scene,idx)
		scene:call("draw")
		sys.shadow.draw_tail(scene,idx)
		gl.PopMatrix()

	end

	if camera then camera:set_active() end

	gl.PushMatrix()
	sys.screen.draw_head(scene,opts)
	scene:systems_call("draw_head")
	sky:draw_sky()
	scene:call("draw")
	water:draw_water()
	scene:systems_call("draw_tail")
	sys.screen.draw_tail()
	gl.PopMatrix()

	sys.screen.build_occlusion(scene)
	sys.screen.build_bloom(scene)


	-- match screen size
	local pos=render:get("pos")
	local siz=render:get("siz")
	local fbo_w=math.ceil(sys.screen.fbo.w*siz[1])
	local fbo_h=math.ceil(sys.screen.fbo.h*siz[2])
	if fbo_w ~= render.fbo.w or fbo_h ~= render.fbo.h then
		render.fbo:resize( fbo_w , fbo_h , 0 )
	end

	gl.PushMatrix()
	render.fbo:bind_frame()
	sys.oven.cake.views.push_and_apply(render.view)
	gl.state.push(gl.state_defaults)

	sys.screen.draw(scene)
	scene:systems_call("draw_hud")

	gl.state.pop()
	sys.oven.cake.views.pop_and_apply()
	gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
	gl.PopMatrix()

	render.fbo_bloom=sys.screen:swap_bloom(render.fbo_bloom)

--	print( render.fbo_bloom.w , render.fbo_bloom.h )
end

renders.item.render_screen=function(render)
	local sys=render.sys
	if not sys.gl then return end -- no gl no draw
	local gl=sys.gl

	render:get_values()

	gl.PushMatrix()
	gl.state.push(gl.state_defaults)
	gl.state.set({
		[gl.BLEND]					=	gl.FALSE,
	})

	local pos=render:get("pos")
	local siz=render:get("siz")
	local x0=(pos[1] or 0)
	local x1=(siz[1] or 1)
-- flip y
	local y0=1-(siz[2] or 1)
	local y1=1-(pos[2] or 0)

	local t={
		(x0*2)-1,	(y1*2)-1,	0,	0,	1,
		(x0*2)-1,	(y0*2)-1,	0,	0,	0,
		(x1*2)-1,	(y1*2)-1,	0,	1,	1,
		(x1*2)-1,	(y0*2)-1,	0,	1,	0,
	}

	sys.oven.cake.canvas.flat.tristrip("rawuv",t,"gamecake_shader?SCR=1&TEX=1",function(p)

			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			render.fbo:bind_texture()
			gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

	end)

	gl.state.pop()
	gl.PopMatrix()

end
