--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local wzips=require("wetgenes.zips")

local wques=require("wetgenes.ques")


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


	screen.loads=function()

		local filename="lua/"..string.gsub(M.modname,"%.","/")..".glsl"
		gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	end

	screen.shader_qs={
	
-- all shaders

		zone_screen={
		},

-- named shaders

		zone_screen_draw={
			GAMMA=1.5,
		},
		
		zone_screen_build_occlusion={
--			SHADOW=" 0.0 , 0.0 , 0.0 , 0.0 ",
		},
		
		zone_screen_build_dark={
		},
		
		zone_screen_build_bloom_pick={
		},
		
		zone_screen_build_blur={
			BLUR=22,
		},
	}
	
	screen.get_shader_qs=function(name,opts)

		local q={}
		
		for n,v in pairs( screen.shader_qs.zone_screen or {} ) do
			q[n]=v
		end
		
		for n,v in pairs( screen.shader_qs[name] or {} ) do
			q[n]=v
		end

		for n,v in pairs( opts or {} ) do
			q[n]=v
		end
		
		q[0]=name
	
		return wques.build(q)

	end


	screen.shader_args=""

	screen.camera_fov=0.5
	screen.base_scale=1
	screen.occlusion_scale=1

	screen.setup=function()

		screen.shader_args=""

		screen.loads()

		screen.fbo=framebuffers.create(0,0,32,{ no_uptwopow=true , depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT, gl.FLOAT} } )

		screen.view=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo,
			vz=8192,
			fov=screen.camera_fov,
			cx=0.5,cy=0.5,cz=0.5,
		})

--	we only need RED but dodgy drivers will bork if this is not at least rgb

		screen.fbo_occlusion=framebuffers.create(0,0,0,{ no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })

		screen.view_occlusion=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo_occlusion,
			vz=8192,
			fov=1/2,
			cx=0.5,cy=0.5,cz=0.5,
		})

		screen.fbo_bloom=framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })

		screen.view_bloom=oven.cake.views.create({
			mode="fbo",
			fbo=screen.fbo_bloom,
			vz=8192,
			fov=1/2,
			cx=0.5,cy=0.5,cz=0.5,
		})

		screen.fbo_blur=framebuffers.create(0,0,0,{no_uptwopow=true , texture_format={gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE} })


	end
	
	screen.draw_head=function(scene)

		local w=math.ceil(oven.win.width  * screen.base_scale )
		local h=math.ceil(oven.win.height * screen.base_scale )

		local ow=math.ceil(w * screen.occlusion_scale )
		local oh=math.ceil(h * screen.occlusion_scale )


		if screen.fbo.w ~= w or screen.fbo.h ~= h or screen.fbo_occlusion.w ~= ow or screen.fbo_occlusion.h ~= oh then

			screen.fbo:resize( w , h , 1 )

			screen.fbo_occlusion:resize( ow , oh , 0 )

			local bw=math.ceil(oven.win.width  * 0.25 ) -- should be relative to original size but should also depend on dpi?
			local bh=math.ceil(oven.win.height * 0.25 )

			if bw>w then bw=w end -- need to cap the maximum
			if bh>h then bh=h end

			screen.fbo_bloom:resize( bw , bh , 0 )

		end
		

		screen.view.fov=screen.camera_fov
		screen.view.vz=8192
		screen.view.pz=0

		screen.view_occlusion.fov=screen.camera_fov
		screen.view_occlusion.vz=8192
		screen.view_occlusion.pz=0

		screen.view_bloom.vz=8192
		screen.view_bloom.pz=0


		gl.PushMatrix()
		screen.fbo:bind_frame()
		oven.cake.views.push_and_apply(screen.view)
		gl.state.push(gl.state_defaults)

		gl.state.set({
			[gl.DEPTH_TEST]					=	gl.TRUE,
			[gl.CULL_FACE]					=	gl.TRUE,
		})

		gl.Clear(gl.DEPTH_BUFFER_BIT) -- we promise to draw to the entire screen

	end

	screen.draw_tail=function()

		gl.state.pop()
		oven.cake.views.pop_and_apply()
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		gl.PopMatrix()

	end

	screen.draw=function(scene)

		gl.PushMatrix()

		local t={
			-1,	 1,	0,	0,	1,
			-1,	-1,	0,	0,	0,
			 1,	 1,	0,	1,	1,
			 1,	-1,	0,	1,	0,
		}

--		local opts=screen.shader_args
--		if scene and scene.systems and scene.systems.input and scene.systems.input.tweak_number then opts=opts.."&TWEAK="..scene.systems.input.tweak_number end
--		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_draw?GAMMA=1.5"..opts,function(p)
		
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_draw"),function(p)

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
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		gl.PopMatrix()

	end


	screen.build_occlusion=function(scene)
	
		screen.fbo:mipmap() -- generate mipmaps for depth and texture

		local t={
			-1,	 1,	0,	0,	1,
			-1,	-1,	0,	0,	0,
			 1,	 1,	0,	1,	1,
			 1,	-1,	0,	1,	0,
		}

		gl.PushMatrix()
		oven.cake.views.push_and_apply(screen.view_occlusion)
		gl.state.push(gl.state_defaults)

		screen.fbo_occlusion:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_occlusion"),function(p)

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
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_dark",{DARK=1}),function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_occlusion:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		screen.fbo_occlusion:bind_frame()
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_dark",{DARK=2}),function(p)

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

		gl.PushMatrix()
		oven.cake.views.push_and_apply(screen.view_bloom)
		gl.state.push(gl.state_defaults)

		screen.fbo_bloom:bind_frame()
--		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_bloom_pick",function(p)
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_bloom_pick"),function(p)

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
--		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_blur?BLUR_AXIS=0&BLUR=22",function(p)
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_blur",{BLUR_AXIS=0}),function(p)

				gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
				screen.fbo_bloom:bind_texture()
				gl.Uniform1i( p:uniform("tex"), gl.NEXT_UNIFORM_TEXTURE )
				gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

		end)

		screen.fbo_bloom:bind_frame()
--		oven.cake.canvas.flat.tristrip("rawuv",t,"zone_screen_build_blur?BLUR_AXIS=1&BLUR=22",function(p)
		oven.cake.canvas.flat.tristrip("rawuv",t,screen.get_shader_qs("zone_screen_build_blur",{BLUR_AXIS=1}),function(p)

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

