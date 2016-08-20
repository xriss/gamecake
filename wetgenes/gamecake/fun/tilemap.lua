--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,tilemap)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

tilemap.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return tilemap
end

tilemap.setup=function()

	tilemap.load()

	return tilemap
end

tilemap.create=function(it,opts)
	it.screen=it.system.components.screen -- system will have been passed in
	it.opts=opts
	it.component="tilemap"
	it.name=opts.name
	
	it.px=0
	it.py=0

	it.window_px=it.opts.window and it.opts.window[1] or 0
	it.window_py=it.opts.window and it.opts.window[2] or 0
	it.window_hx=it.opts.window and it.opts.window[3] or it.screen.hx
	it.window_hy=it.opts.window and it.opts.window[4] or it.screen.hy

	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or 8
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or 8

	it.bitmap_hx=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_hy=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16

	it.tilemap_hx=it.opts.tilemap_size and it.opts.tilemap_size[1] or 256
	it.tilemap_hy=it.opts.tilemap_size and it.opts.tilemap_size[2] or 256
	

	it.setup=function(opts)
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.tile_hx*it.bitmap_hx , it.tile_hy*it.bitmap_hy , 1)
		it.tilemap_grd =wgrd.create("U8_RGBA", it.tilemap_hx           , it.tilemap_hy           , 1)

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

		it.tilemap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.REPEAT)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.REPEAT)

	end

	it.clean=function()
		if it.bitmap_tex then
			gl.DeleteTexture( it.bitmap_tex )
			it.bitmap_tex=nil
		end
		if it.tilemap_tex then
			gl.DeleteTexture( it.tilemap_tex )
			it.tilemap_tex=nil
		end
	end

	it.update=function()
	end
	
	it.draw=function()

		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			it.bitmap_grd.width,
			it.bitmap_grd.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			it.bitmap_grd.data )

		gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )	
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			it.tilemap_grd.width,
			it.tilemap_grd.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			it.tilemap_grd.data )



		local x,y,hx,hy=it.window_px , it.window_py , it.window_hx , it.window_hy
		local u,v,hu,hv=it.px/it.tile_hx , it.py/it.tile_hy , hx/it.tile_hx , hy/it.tile_hy
		local t={
			x,		y+hy,	0,	u,		v+hv, 			
			x,		y,		0,	u,		v,
			x+hx,	y+hy,	0,	u+hu,	v+hv, 			
			x+hx,	y,		0,	u+hu,	v,
		}


		flat.tristrip("rawuv",t,"fun_draw_tilemap",function(p)

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_map"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_tile"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )

			gl.Uniform4f( p:uniform("tile_info"), it.tile_hx,it.tile_hy,it.tile_hx*it.bitmap_hx,it.tile_hy*it.bitmap_hy )
			gl.Uniform4f( p:uniform("map_info"),  0,0,it.tilemap_hx,it.tilemap_hy )

		end)

	end

-- if you load a 128x1 font data then the following functions can be used to print 7bit ascii text	
	it.text_px=0
	it.text_py=0
	it.text_hx=it.tilemap_hx
	it.text_hy=it.tilemap_hy

-- replace this function if your font is somewhere else, or you wish to handle more than 128 tiles (utf8)
	it.text_tile=function(c)
		return {(c:byte()),0,0,0}
	end

	it.text_print_tile=function(c,x,y)
		it.tilemap_grd:pixels( x,y, 1,1, it.text_tile(c) )
	end

	it.text_print=function(s,x,y)
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			if x>=it.text_px and y>=it.text_py and x<it.text_px+it.text_hx and y<it.text_py+it.text_hy then
				it.text_print_tile(c,x,y)
			end
			x=x+1
		end

		return x,y
	end

	it.text_scroll=function(x,y)
	end


	return it
end

	return tilemap
end


