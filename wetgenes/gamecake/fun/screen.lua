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
	it.name=opts.name
	
	it.xh=it.opts.size and it.opts.size[1] or 360
	it.yh=it.opts.size and it.opts.size[2] or 240

	it.fbo=framebuffers.create(it.xh,it.yh,1)
	it.lay=layouts.create{parent={x=0,y=0,w=it.xh,h=it.yh}}

-- need two buffers to generate bloom
--	it.fxbo1=framebuffers.create(it.xh,it.yh,0)
--	it.fxbo2=framebuffers.create(it.xh,it.yh,0)
	
	-- clear fbo and prepare for drawing into
	it.draw_into_start=function()

		it.fbo:bind_frame()

		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()

		it.lay_orig=it.lay.apply(nil,nil,0)

		gl.ClearColor(0,0,0,1)
		gl.DepthMask(gl.TRUE)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)
		gl.DepthMask(gl.FALSE)

	end


	-- finish drawing
	it.draw_into_finish=function()

	--	screen.fbo:mipmap()
		
--		screen.draw_bloom_setup()
--		screen.draw_bloom_blur()

		gl.BindFramebuffer(gl.FRAMEBUFFER, 0.0)

		it.lay_orig.restore()

		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()			
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

	end

	-- draw fbo to the main screen
	it.draw_fbo=function()

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

		flat.tristrip("rawuv",t,"raw_tex")

--			gl.PopMatrix()
		
	end

	return it
end


	return screen
end
