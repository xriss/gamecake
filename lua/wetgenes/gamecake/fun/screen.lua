--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Screen FBO for rendering to.

local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,screen)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat
	local layouts=cake.layouts
	local views=cake.views

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")


screen.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return screen
end

screen.setup=function()

	screen.load()

	return screen
end

screen.create=function(it,opts)
	it=it or {}
	it.opts=opts or {}
	it.component="screen"
	it.name=opts.name or it.component
	
	it.bloom=it.opts.bloom

	it.scale=it.opts.scale or 3
	
	it.filter=it.opts.filter or "none"
	it.shadow=it.opts.shadow or "none"
	
	it.hx=it.opts.size and it.opts.size[1] or 320
	it.hy=it.opts.size and it.opts.size[2] or 240

	
--	it.drawlist=opts.drawlist

	it.fbo=framebuffers.create(it.hx,it.hy,0)
--	it.lay=layouts.create{parent={x=0,y=0,w=it.hx,h=it.hy}}
	it.view=views.create({
		mode="fbo",
		fbo=it.fbo,
		vx=it.hx,
		vy=it.hy,
		vz=it.hy*4,
	})
	
	it.layers={}
	for i=1,opts.layers do -- create a bunch of layers
		it.layers[i]=framebuffers.create(it.hx,it.hy,1)
	end

-- need another two buffers (no depth) to perform full screen shader fx and generate bloom with
	it.fxbo1=framebuffers.create(it.hx,it.hy,0)
	it.fxbo2=framebuffers.create(it.hx,it.hy,0)
	
	-- clear fbo and prepare for drawing into
	it.draw_into_layer_start=function(idx)

		it.layers[idx]:bind_frame()

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		views.push_and_apply(it.view)

		gl.ClearColor(0,0,0,0)
		gl.DepthMask(gl.TRUE)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		gl.Enable(gl.DEPTH_TEST)
	end


	-- finish drawing
	it.draw_into_layer_finish=function(idx)

		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)

		views.pop_and_apply()

		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()			
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

	end

	-- clear fbo and prepare for drawing into
	it.draw_into_screen_start=function()

		it.fbo:bind_frame()

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		views.push_and_apply(it.view)
--		it.lay_orig=it.lay.apply(nil,nil,0)

		gl.ClearColor(0,0,0,1)
		gl.DepthMask(gl.FALSE)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		gl.Disable(gl.DEPTH_TEST)

	end


	-- finish drawing
	it.draw_into_screen_finish=function()

	--	screen.fbo:mipmap()

		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)

		views.pop_and_apply()
--		it.lay_orig.restore()

		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()			
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

		it.create_bloom()

		gl.DepthMask(gl.TRUE)
	end
	
	-- draw layer fbo, probably with a drop shadow effect
	it.draw_layer=function(idx)
	
		local fbo=it.layers[idx]

		fbo:bind_texture()
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)

			gl.Color(1,1,1,1)

		local r,g,b,a=gl.color_get_rgba()
		local v3=gl.apply_modelview( {fbo.w*-0.0,	fbo.h* 1.0,	0,1} )
		local v1=gl.apply_modelview( {fbo.w*-0.0,	fbo.h*-0.0,	0,1} )
		local v4=gl.apply_modelview( {fbo.w* 1.0,	fbo.h* 1.0,	0,1} )
		local v2=gl.apply_modelview( {fbo.w* 1.0,	fbo.h*-0.0,	0,1} )

		local t={
			v3[1],	v3[2],	v3[3],	0,			0, 			
			v1[1],	v1[2],	v1[3],	0,			fbo.uvh,
			v4[1],	v4[2],	v4[3],	fbo.uvw,	0, 			
			v2[1],	v2[2],	v2[3],	fbo.uvw,	fbo.uvh,
		}


		if it.shadow=="drop" then
			flat.tristrip("rawuv",t,"fun_screen_dropshadow",function(p)
				local v=views.get()
				gl.Uniform4f( p:uniform("siz"), it.fbo.txw				,	it.fbo.txh, 
												it.fbo.uvw/v.hx*v.sx	,	it.fbo.uvh/v.hy*v.sy	)

				gl.Uniform4f( p:uniform("shadow_info"), 1	,	it.shadow_max or 0.4, 
														2	,	it.shadow_min or 0.8	)

			end)
		else
			flat.tristrip("rawuv",t,"raw_tex")
		end

	end

	-- draw screen fbo
	it.draw_screen=function()

--			gl.PushMatrix()

		it.fbo:bind_texture()
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)

		local r,g,b,a=gl.color_get_rgba()
		local v3=gl.apply_modelview( {it.fbo.w*-0.5,	it.fbo.h* 0.5,	0,1} )
		local v1=gl.apply_modelview( {it.fbo.w*-0.5,	it.fbo.h*-0.5,	0,1} )
		local v4=gl.apply_modelview( {it.fbo.w* 0.5,	it.fbo.h* 0.5,	0,1} )
		local v2=gl.apply_modelview( {it.fbo.w* 0.5,	it.fbo.h*-0.5,	0,1} )

		local t={
			v3[1],	v3[2],	v3[3],	0,			0, 			
			v1[1],	v1[2],	v1[3],	0,			it.fbo.uvh,
			v4[1],	v4[2],	v4[3],	it.fbo.uvw,	0, 			
			v2[1],	v2[2],	v2[3],	it.fbo.uvw,	it.fbo.uvh,
		}

		if it.filter=="scanline" then
			flat.tristrip("rawuv",t,"fun_screen_scanline",function(p)
				local v=views.get()
--				local l=layouts.get() -- the size of what we are rendering, so we can calculate display pixel size in uv space (siz.zw)
				gl.Uniform4f( p:uniform("siz"), it.fbo.txw				,	it.fbo.txh, 
												it.fbo.uvw/v.hx*v.sx	,	it.fbo.uvh/v.hy*v.sy	)
			end)
		else
			flat.tristrip("rawuv",t,"raw_tex")
		end

--			gl.PopMatrix()

		if it.bloom then -- bloom on

			gl.Color(it.bloom,it.bloom,it.bloom,0)
			it.fxbo1:bind_texture()
			flat.tristrip("rawuv",t,"raw_tex")
			gl.Color(r,g,b,a)
			
		end

	end

	-- create the bloom 
	it.create_bloom=function()
	
		if not it.bloom then return end
		
		gl.Disable(gl.BLEND)

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()		
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		views.push_and_apply(it.view)
--		it.lay_orig=it.lay.apply(nil,nil,0)

		local v3=gl.apply_modelview( {it.fbo.w*-0,	it.fbo.h* 1,	0,1} )
		local v1=gl.apply_modelview( {it.fbo.w*-0,	it.fbo.h*-0,	0,1} )
		local v4=gl.apply_modelview( {it.fbo.w* 1,	it.fbo.h* 1,	0,1} )
		local v2=gl.apply_modelview( {it.fbo.w* 1,	it.fbo.h*-0,	0,1} )
		local t={
			v3[1],	v3[2],	v3[3],	0,			0, 			
			v1[1],	v1[2],	v1[3],	0,			it.fbo.uvh,
			v4[1],	v4[2],	v4[3],	it.fbo.uvw,	0, 			
			v2[1],	v2[2],	v2[3],	it.fbo.uvw,	it.fbo.uvh,
		}


		it.fxbo1:bind_frame()
		flat.tristrip("rawuv",t,"fun_screen_bloom_pick",function(p)
			it.fbo:bind_texture()
		end)


		it.fxbo2:bind_frame()
		flat.tristrip("rawuv",t,"fun_screen_bloom_blur",function(p)
			it.fxbo1:bind_texture()
			gl.Uniform4f( p:uniform("pix_siz"), it.fbo.uvw/it.fbo.w,0,0,1 )
		end)


		it.fxbo1:bind_frame()
		flat.tristrip("rawuv",t,"fun_screen_bloom_blur",function(p)
			it.fxbo2:bind_texture()
			gl.Uniform4f( p:uniform("pix_siz"), 0,it.fbo.uvh/it.fbo.h,0,1 )
		end)


		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)
		views.pop_and_apply()
--		it.lay_orig.restore()
		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()			
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

		gl.Enable(gl.BLEND)

	end



	return it
end


	return screen
end
