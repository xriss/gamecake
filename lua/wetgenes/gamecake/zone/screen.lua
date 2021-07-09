--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

--[[


Manage screen buffer processing effects such as occlusion and bloom.

These can be optionally disabled.


]]


M.bake=function(oven,screen)
	screen=screen or {}
	screen.modname=M.modname
	
	local gl=oven.gl

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local shadow=oven.rebake("wetgenes.gamecake.zone.shadow")

	screen.loads=function()

		local filename="lua/"..string.gsub(M.modname,"%.","/")..".glsl"
		gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	end

	screen.setup=function()

		screen.loads()

		screen.fbo=framebuffers.create(0,0,1,{ no_uptwopow=true , depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT, gl.FLOAT} } )

		screen.view=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo,
			vz=8192,
			pz=0,
			fov=1/2,
			cx=0.5,cy=0.5,
		})

		screen.fbo_occlusion=framebuffers.create(0,0,0,{ no_uptwopow=true , texture_format={gl.RED, gl.RED, gl.UNSIGNED_BYTE} })

		screen.view_occlusion=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo_occlusion,
			vz=8192,
			pz=0,
			fov=1/2,
			cx=0.5,cy=0.5,
		})

		screen.fbo_bloom=framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })

		screen.view_bloom=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo_bloom,
			vz=8192,
			pz=0,
			fov=1/2,
			cx=0.5,cy=0.5,
		})

		screen.fbo_blur=framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })


	end
	
	screen.draw_head=function(scene)

		local tweak=scene.systems.tweak

		local w=math.floor((oven.win.width * (10-0) )/10)
		local h=math.floor((oven.win.height * (10-0) )/10)

		screen.fbo:resize( w , h , 1 )

		screen.fbo_occlusion:resize( math.ceil(w/1) , math.ceil(h/1) , 0 )

		screen.fbo_bloom:resize( math.ceil(w/4) , math.ceil(h/4) , 0 )

		screen.view.vz=8192
		screen.view.pz=0

		screen.view_occlusion.vz=8192
		screen.view_occlusion.pz=0

		screen.view_bloom.vz=8192
		screen.view_bloom.pz=0


		gl.PushMatrix()
		screen.fbo:bind_frame()
		oven.cake.views.push_and_apply(screen.view)
		gl.state.push(gl.state_defaults)

--		gl.state.set({
--			[gl.BLEND]						=	gl.FALSE,
--			[gl.DEPTH_TEST]					=	gl.TRUE,
--		})
		gl.Clear(gl.DEPTH_BUFFER_BIT) -- we promise to draw to the entire screen

	end

	screen.draw_tail=function()

		gl.state.pop()
		oven.cake.views.pop_and_apply()
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		gl.PopMatrix()

	end

	screen.draw=function(scene)

		local tweak=scene.systems.tweak

		gl.PushMatrix()

		local t={
			-1,	 1,	0,	0,	1,
			-1,	-1,	0,	0,	0,
			 1,	 1,	0,	1,	1,
			 1,	-1,	0,	1,	0,
		}

		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_draw?GAMMA=1.5&TWEAK="..tweak.number,function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo:bind_texture()
				gl.Uniform1i( p:uniform("tex0"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_occlusion:bind_texture()
				gl.Uniform1i( p:uniform("tex1"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_bloom:bind_texture()
				gl.Uniform1i( p:uniform("tex2"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.PopMatrix()

	end


	screen.draw_test=function()

		gl.PushMatrix()

		local t={
			0,	1,	0,	0,	1,
			0,	0,	0,	0,	0,
			1,	1,	0,	1,	1,
			1,	0,	0,	1,	0,
		}

		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_draw",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_occlusion:bind_texture()
--				screen.fbo:bind_texture()
--				screen.fbo:bind_depth()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.PopMatrix()

	end

	screen.build_occlusion=function(scene)

		local t={
			-1,	 1,	0,	0,	1,
			-1,	-1,	0,	0,	0,
			 1,	 1,	0,	1,	1,
			 1,	-1,	0,	1,	0,
		}

		local tweak=scene.systems.tweak

		gl.PushMatrix()
		oven.cake.views.push_and_apply(screen.view_occlusion)
		gl.state.push(gl.state_defaults)

		screen.fbo_occlusion:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_occlusion?TWEAK="..tweak.number.."&SHADOW="..shadow.default,function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo:bind_depth()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

				local inverse_projection=M4(gl.matrix(gl.PROJECTION)):inverse()
				gl.UniformMatrix4f(p:uniform("inverse_projection"), inverse_projection )

				local inverse_modelview=M4(gl.matrix(gl.MODELVIEW)):inverse()
				gl.UniformMatrix4f(p:uniform("inverse_modelview"),  inverse_modelview )

		end)

		screen.fbo_blur:resize( screen.fbo_occlusion.w , screen.fbo_occlusion.h , 0 )

		screen.fbo_blur:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_dark?DARK=1",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_occlusion:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		screen.fbo_occlusion:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_dark?DARK=2",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_blur:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.state.pop()
		oven.cake.views.pop_and_apply()
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		gl.PopMatrix()

	end

	screen.build_bloom=function(scene)

		local t={
			-1,	 1,	0,	0,	1,
			-1,	-1,	0,	0,	0,
			 1,	 1,	0,	1,	1,
			 1,	-1,	0,	1,	0,
		}

		local tweak=scene.systems.tweak

		gl.PushMatrix()
		oven.cake.views.push_and_apply(screen.view_bloom)
		gl.state.push(gl.state_defaults)

		screen.fbo_bloom:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_bloom_pick",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo:bind_texture()
				gl.Uniform1i( p:uniform("tex0"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

--[[
				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_occlusion:bind_texture()
				gl.Uniform1i( p:uniform("tex1"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1
]]

		end)

		screen.fbo_blur:resize( screen.fbo_bloom.w , screen.fbo_bloom.h , 0 )

		screen.fbo_blur:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_blur?BLUR_AXIS=0&BLUR=22",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_bloom:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		screen.fbo_bloom:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_blur?BLUR_AXIS=1&BLUR=22",function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_blur:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.state.pop()
		oven.cake.views.pop_and_apply()
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		gl.PopMatrix()

	end

	return screen
end

