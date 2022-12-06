--
-- (C) 2022 Kriss@XIXs.com and released under the MIT license,
-- see http://opensource.org/licenses/MIT for full license text.
--
local	coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=
		coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Generate textures using GLSL in a shadertoy like harness

local wzips=require("wetgenes.zips")
local wques=require("wetgenes.ques")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,texgenes)

	local gl=oven.gl
	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

texgenes.load=function()
	if texgenes.loaded then return texgenes end

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )
	local filename="lua/"..(M.modname):gsub("%.","/").."_sketches.glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	texgenes.loaded=true
	return texgenes
end


texgenes.render=function(opts)

	texgenes.load() -- make sure we are loaded glsl

	opts            = opts            or {}
	opts.width      = opts.width      or 256
	opts.height     = opts.height     or 256
	opts.scale      = opts.scale      or 1
	opts.shadername = opts.shadername or "texgenes_test"
	opts.qs         = opts.qs         or {}

	local qs={[0]=opts.shadername}
	if string.find(opts.shadername, "?") then wques.parse(opts.shadername,qs) end -- options from string
	for n,v in pairs(opts.qs) do qs[n]=v end -- overide options from table
	local shadername = wques.build(qs) -- rebuild string

	local fbo=framebuffers.create( opts.width*opts.scale , opts.height*opts.scale , 0 )
	
	fbo:render_start()

	gl.state.set({
		[gl.BLEND]						=	gl.FALSE,
		[gl.CULL_FACE]					=	gl.FALSE,
		[gl.DEPTH_TEST]					=	gl.FALSE,
		[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
	})

	local v1=gl.apply_modelview( {fbo.w*-0,	fbo.h* 1,	0,1} )
	local v2=gl.apply_modelview( {fbo.w*-0,	fbo.h*-0,	0,1} )
	local v3=gl.apply_modelview( {fbo.w* 1,	fbo.h* 1,	0,1} )
	local v4=gl.apply_modelview( {fbo.w* 1,	fbo.h*-0,	0,1} )
	local t={
		v1[1],	v1[2],	v1[3],	0,			0, 			
		v2[1],	v2[2],	v2[3],	0,			fbo.w,
		v3[1],	v3[2],	v3[3],	fbo.h,		0, 			
		v4[1],	v4[2],	v4[3],	fbo.h,		fbo.w,
	}

	oven.cake.canvas.flat.tristrip("rawuv",t,shadername,function(p)

		gl.Uniform3f( p:uniform("iResolution"), fbo.w,fbo.h,0 )
		gl.Uniform1f( p:uniform("iTime"), 0 )

--		gl.UniformMatrix4f( p:uniform("iCamera"), show.cam )
--		gl.UniformMatrix4f( p:uniform("iCamera2D"), show.cam2d )

		gl.Uniform4f( p:uniform("iMouse"), 0 , 0 , 0 , 0 )

		if opts.callback then opts.callback(p) end
	end)

	fbo:render_stop()
	local g=fbo:download()
	fbo:clean()
	
	if opts.scale~=1 then
		g:scale(opts.width,opts.height,1)
	end
	
	return g

end


	return texgenes
end
