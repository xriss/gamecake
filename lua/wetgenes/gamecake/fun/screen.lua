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

	it.zx=it.opts.zxy and it.opts.zxy[1] or 0 -- fake 3d projection
	it.zy=it.opts.zxy and it.opts.zxy[2] or 0

	it.fbo=framebuffers.create(it.hx,it.hy,0)
	it.view=views.create({
		mode="fbo",
		fbo=it.fbo,
		vx=it.hx,
		vy=it.hy,
		vz=it.hy*4,
	})
	
	it.layers={}

	local opts_layers=opts.layers or 1 -- temporary layer info list
	
	if type(opts_layers)=="number" then -- create table of given length
		local t={}
		for i=1,opts_layers do t[i]={} end
		opts_layers=t
	end

	for i,v in ipairs(opts_layers) do -- create a bunch of layers
		local layer={}
		it.layers[i]=layer
		
		layer.idx=i
		
		layer.shadow=v.shadow or it.shadow or "none"

		layer.clip_px=v.clip and v.clip[1] or 0 -- layer display area in screen space
		layer.clip_py=v.clip and v.clip[2] or 0
		layer.clip_hx=v.clip and v.clip[3] or it.hx
		layer.clip_hy=v.clip and v.clip[4] or it.hy

		layer.px=v.scroll and v.scroll[1] or 0	-- layer scroll
		layer.py=v.scroll and v.scroll[2] or 0

		layer.hx=v.size and v.size[1] or it.hx -- layer size
		layer.hy=v.size and v.size[2] or it.hy
		
		layer.fbo=framebuffers.create(layer.hx,layer.hy,1)

		layer.view=views.create({
			mode="fbo",
			fbo=layer.fbo,
			vx=layer.hx,
			vy=layer.hy,
			vz=layer.hy*4,
		})
		
		layer.hooks={} -- user managable list of gl drawing callback functions (lowlevel hacks)

	end

-- need another two buffers (no depth) to perform full screen shader fx and generate bloom with
	it.fxbo1=framebuffers.create(it.hx,it.hy,0)
	it.fxbo2=framebuffers.create(it.hx,it.hy,0)
	
	it.screen_resize_view=function()
		if it.system.opts.hxhy=="best" then -- auto resize
		
			local view=views.get()
			local opts=it.system.opts
			local hx,hy=opts.hx,opts.hy

			local faa=view.hx/view.hy
			local haa=opts.xx/opts.yy

			if haa > faa then -- fit aspect
				hx=opts.xx
				hy=math.floor((opts.xx/faa)/2)*2 -- force even number
			else
				hy=opts.yy
				hx=math.floor((opts.yy*faa)/2)*2 -- force even number
			end
			
			-- sanity
			if hx<opts.xx then hx=opts.xx end -- no less than minimum size
			if hy<opts.yy then hy=opts.yy end
			if hx>opts.xx*2 then hx=opts.xx*2 end -- no more than double minimum size
			if hy>opts.yy*2 then hy=opts.yy*2 end
			hx=math.floor(hx)	-- snap
			hy=math.floor(hy)

			it.system.screen_resize(hx,hy) -- resize all

			view.vx=hx
			view.vy=hy
			view.vz=hy*4

		end

	end

	it.screen_resize=function(hx,hy)
		if hx~=it.hx or hy~=it.hy then -- new size
		
			it.hx=hx
			it.hy=hy
			it.view.vx=hx
			it.view.vy=hy
			it.view.vz=hy*4
			it.fbo:resize(hx,hy,0)
			it.fxbo1:resize(hx,hy,0)
			it.fxbo2:resize(hx,hy,0)

			for i,l in ipairs(it.layers) do
				l.hx=hx
				l.hy=hy
				l.clip_hx=hx
				l.clip_hy=hy
				l.view.vx=hx
				l.view.vy=hy
				l.view.vz=hy*4
				l.fbo:resize(hx,hy,1)
			end
		
		end
	end

	it.clean=function()
		if it.fxbo1 then
			framebuffers.clean(it.fxbo1)
			it.fxbo1=nil
		end
		if it.fxbo2 then
			framebuffers.clean(it.fxbo2)
			it.fxbo2=nil
		end
		for _,layer in pairs(it.layers) do
			if layer.fbo then
				framebuffers.clean(layer.fbo)
				layer.fbo=nil
			end
		end
	end
	
	-- clear fbo and prepare for drawing into
	it.draw_into_layer_start=function(idx)

		it.layers[idx].fbo:bind_frame()

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		views.push_and_apply(it.layers[idx].view)

		gl.ClearColor(0,0,0,0)
		gl.state.set({
			[gl.DEPTH_WRITEMASK]			=	gl.TRUE,
			[gl.DEPTH_TEST]					=	gl.TRUE,
		})
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
	end


	-- extra drawing functions called while layer fbo is bound after main components
	it.hooks=function(idx)

		local layer=it.layers[idx]
		
		for i,v in ipairs(layer.hooks) do
		
			v(layer)
			
		end

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

		gl.ClearColor(0,0,0,1)
		gl.DepthMask(gl.FALSE)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		gl.Disable(gl.DEPTH_TEST)

	end


	-- finish drawing
	it.draw_into_screen_finish=function()

		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)

		views.pop_and_apply()

		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()			
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

		it.create_bloom()

		gl.DepthMask(gl.TRUE)
	end
	
	-- draw layer fbo, probably with a drop shadow effect
	it.draw_layer=function(idx)
	
		local layer=assert(it.layers[idx])
		local fbo=layer.fbo
		
		local fpx=layer.clip_px
		local fpy=layer.clip_py
		local fhx=(layer.clip_px+layer.clip_hx)
		local fhy=(layer.clip_py+layer.clip_hy)

		fbo:bind_texture()
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)

		gl.Color(1,1,1,1)

		local r,g,b,a=gl.color_get_rgba()
		local v3=gl.apply_modelview( {fpx,	fhy,	0,1} )
		local v1=gl.apply_modelview( {fpx,	fpy,	0,1} )
		local v4=gl.apply_modelview( {fhx,	fhy,	0,1} )
		local v2=gl.apply_modelview( {fhx,	fpy,	0,1} )

		local t={
			v3[1],	v3[2],	v3[3],	(layer.px+fpx)*fbo.uvw/layer.hx,	(layer.py+fpy)*fbo.uvh/layer.hy,
			v1[1],	v1[2],	v1[3],	(layer.px+fpx)*fbo.uvw/layer.hx,	(layer.py+fhy)*fbo.uvh/layer.hy,
			v4[1],	v4[2],	v4[3],	(layer.px+fhx)*fbo.uvw/layer.hx,	(layer.py+fpy)*fbo.uvh/layer.hy,
			v2[1],	v2[2],	v2[3],	(layer.px+fhx)*fbo.uvw/layer.hx,	(layer.py+fhy)*fbo.uvh/layer.hy,
		}


		if layer.shadow=="drop" then
			flat.tristrip("rawuv",t,"fun_screen_dropshadow",function(p)

				gl.Uniform2f( p:uniform("projection_zxy"), 0,0)

				local v=views.get()
				gl.Uniform4f( p:uniform("siz"), fbo.txw				,	fbo.txh, 
												fbo.uvw/v.hx*v.sx	,	fbo.uvh/v.hy*v.sy	)

				gl.Uniform4f( p:uniform("shadow_info"), 1	,	it.shadow_max or 0.4, 
														2	,	it.shadow_min or 0.8	)

			end)
		else
			flat.tristrip("rawuv",t,"raw_tex")
		end

	end

	-- draw screen fbo
	it.draw_screen=function()

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
				gl.Uniform2f( p:uniform("projection_zxy"), 0,0)
				local v=views.get()
				gl.Uniform4f( p:uniform("siz"), it.fbo.txw				,	it.fbo.txh, 
												it.fbo.uvw/v.hx*v.sx	,	it.fbo.uvh/v.hy*v.sy	)
				local view=views.get()
				gl.Uniform4f( p:uniform("vsiz"), view.hx/view.sx , view.hy/view.sy , it.fbo.w , it.fbo.h )
			end)
		else
			flat.tristrip("rawuv",t,"raw_tex")
		end

		if it.bloom then -- bloom on

			gl.Color(it.bloom,it.bloom,it.bloom,0)
			it.fxbo1:bind_texture()
			flat.tristrip("rawuv",t,"raw_tex")
			gl.Color(r,g,b,a)
			
		end

		it.screen_resize_view()
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
			gl.Uniform2f( p:uniform("projection_zxy"), 0,0)
			it.fbo:bind_texture()
		end)


		it.fxbo2:bind_frame()
		flat.tristrip("rawuv",t,"fun_screen_bloom_blur",function(p)
			gl.Uniform2f( p:uniform("projection_zxy"), 0,0)
			it.fxbo1:bind_texture()
			gl.Uniform4f( p:uniform("pix_siz"), it.fbo.uvw/it.fbo.w,0,0,1 )
		end)


		it.fxbo1:bind_frame()
		flat.tristrip("rawuv",t,"fun_screen_bloom_blur",function(p)
			gl.Uniform2f( p:uniform("projection_zxy"), 0,0)
			it.fxbo2:bind_texture()
			gl.Uniform4f( p:uniform("pix_siz"), 0,it.fbo.uvh/it.fbo.h,0,1 )
		end)


		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)
		views.pop_and_apply()
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
