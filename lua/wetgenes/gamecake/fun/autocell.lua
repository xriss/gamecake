--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,autocell)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
	
autocell.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return autocell
end

autocell.setup=function()

	autocell.load()

	return autocell
end

autocell.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.tiles =assert(it.system.components[opts.tiles  or "tiles" ])
	it.opts=opts
	it.component="autocell"
	it.name=opts.name or it.component
	
	it.px=0
	it.py=0

	it.window_px=it.opts.window and it.opts.window[1] or 0
	it.window_py=it.opts.window and it.opts.window[2] or 0
	it.window_hx=it.opts.window and it.opts.window[3] or it.screen.hx
	it.window_hy=it.opts.window and it.opts.window[4] or it.screen.hy

	it.autocell_hx=it.opts.autocell_size and it.opts.autocell_size[1] or 256
	it.autocell_hy=it.opts.autocell_size and it.opts.autocell_size[2] or 256
	
	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or it.tiles.tile_hx -- cache the tile size, or allow it to change per map
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or it.tiles.tile_hy
	
	it.layer=opts.layer or 1

	it.shader_step_name=opts.shader_step_name or "fun_step_autocell"
	it.shader_draw_name=opts.shader_draw_name or "fun_draw_autocell"

	it.setup=function(opts)
		
		it.autocell_grd =wgrd.create("U8_RGBA", it.autocell_hx           , it.autocell_hy           , 1)
		it.frames={}
		it.frame=1
		it.frames[1]=framebuffers.create(it.hx,it.hy,0) -- no depth, just rgba
		it.frames[2]=framebuffers.create(it.hx,it.hy,0)

		for i,v in pairs( it.frames or {} ) do
			v:bind_texture()
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		end

		it.dirty(true)

	end

	it.clean=function()
		for i,v in pairs( it.frames or {} ) do
			v:clean()
		end
	end

-- this performs a single cellular step from one fbo to the other
	it.update=function()

		gl.ActiveTexture(gl.TEXTURE0)
		it.frames[it.frame]:bind_texture()

	end
	
	it.draw=function()

		if it.dirty() then -- force a replacement of the current cell fbo image data (ie at the start)
		
			it.dirty(false)

			it.frames[it.frame]:bind_texture()
			gl.TexImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RGBA,
				it.autocell_grd.width,
				it.autocell_grd.height,
				0,
				gl.RGBA,
				gl.UNSIGNED_BYTE,
				it.autocell_grd.data )

		end

		gl.Color(1,1,1,1)

--		for i,dl in ipairs(it.drawlist) do
		local dl={ color={1,1,1,1} , dx=0 , dy=0 }
		

			local x,y,hx,hy=it.window_px+dl.dx , it.window_py+dl.dy , it.window_hx , it.window_hy
			local u,v,hu,hv=it.px/it.tile_hx , it.py/it.tile_hy , hx/it.tile_hx , hy/it.tile_hy
			local t={
				x,		y+hy,	0,	u,		v+hv, 			
				x,		y,		0,	u,		v,
				x+hx,	y+hy,	0,	u+hu,	v+hv, 			
				x+hx,	y,		0,	u+hu,	v,
			}


			flat.tristrip("rawuv",t,it.shader_draw_name,function(p)

				gl.ActiveTexture(gl.TEXTURE2) gl.Uniform1i( p:uniform("tex_cmap"), 2 )
				gl.BindTexture( gl.TEXTURE_2D , it.colors.cmap_tex )

				gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_tile"), 1 )
				gl.BindTexture( gl.TEXTURE_2D , it.tiles.bitmap_tex )

				gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_map"), 0 )
				it.frames[it.frame]:bind_texture()


				gl.Uniform4f( p:uniform("tile_info"),	it.tile_hx,
														it.tile_hy,
														it.tiles.hx,
														it.tiles.hy )
				gl.Uniform4f( p:uniform("map_info"), 	0,0,it.autocell_hx,it.autocell_hy )

				gl.Uniform4f( p:uniform("color"), 	dl.color[1],dl.color[2],dl.color[3],dl.color[4] )

			end)
			
--		end

	end

	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end
	
	return it
end

	return autocell
end


