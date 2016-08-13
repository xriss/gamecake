--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,charmap)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

charmap.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return charmap
end

charmap.setup=function()

	charmap.load()

	return charmap
end

charmap.create=function(it,opts)
	it.screen=it.system.components.screen -- system will have been passed in
	it.opts=opts
	it.component="charmap"
	it.name=opts.name
	
	it.xp=0
	it.yp=0

	it.window_xp=it.opts.window and it.opts.window[1] or 0
	it.window_yp=it.opts.window and it.opts.window[2] or 0
	it.window_xh=it.opts.window and it.opts.window[3] or it.screen.xh
	it.window_yh=it.opts.window and it.opts.window[4] or it.screen.yh

	it.char_xh=it.opts.char_size and it.opts.char_size[1] or 8
	it.char_yh=it.opts.char_size and it.opts.char_size[2] or 8

	it.bitmap_xh=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_yh=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16

	it.charmap_xh=it.opts.charmap_size and it.opts.charmap_size[1] or 256
	it.charmap_yh=it.opts.charmap_size and it.opts.charmap_size[2] or 256
	

	it.setup=function(opts)
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.char_xh*it.bitmap_xh , it.char_yh*it.bitmap_yh , 1)
		it.charmap_grd =wgrd.create("U8_RGBA", it.charmap_xh           , it.charmap_yh           , 1)

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

		it.charmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.charmap_tex )	
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
		if it.charmap_tex then
			gl.DeleteTexture( it.charmap_tex )
			it.charmap_tex=nil
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

		gl.BindTexture( gl.TEXTURE_2D , it.charmap_tex )	
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			it.charmap_grd.width,
			it.charmap_grd.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			it.charmap_grd.data )



		local x,y,xh,yh=it.window_xp , it.window_yp , it.window_xh , it.window_yh
		local u,v,uh,vh=it.xp/it.char_xh , it.yp/it.char_yh , xh/it.char_xh , yh/it.char_yh
		local t={
			x,		y+yh,	0,	u,		v+vh, 			
			x,		y,		0,	u,		v,
			x+xh,	y+yh,	0,	u+uh,	v+vh, 			
			x+xh,	y,		0,	u+uh,	v,
		}


		flat.tristrip("rawuv",t,"fun_draw_charmap",function(p)

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_map"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.charmap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_char"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )

			gl.Uniform4f( p:uniform("char_info"), it.char_xh,it.char_yh,it.char_xh*it.bitmap_xh,it.char_yh*it.bitmap_yh )
			gl.Uniform4f( p:uniform("map_info"),  0,0,it.charmap_xh,it.charmap_yh )

		end)

	end

-- if you load a 128x1 font data then the following functions can be used to print 7bit ascii text	
	it.text_xp=0
	it.text_yp=0
	it.text_xh=it.charmap_xh
	it.text_yh=it.charmap_yh

-- replace this function if your font is somewhere else, or you wish to handle more than 128 chars (utf8)
	it.text_char=function(c)
		return {(c:byte()),0,0,0}
	end

	it.text_print_char=function(c,x,y)
		it.charmap_grd:pixels( x,y, 1,1, it.text_char(c) )
	end

	it.text_print=function(s,x,y)
		for c in s:gmatch("([%z\1-\127\194-\244][\128-\191]*)") do
			if x>=it.text_xp and y>=it.text_yp and x<it.text_xp+it.text_xh and y<it.text_yp+it.text_yh then
				it.text_print_char(c,x,y)
			end
			x=x+1
		end

		return x,y
	end

	it.text_scroll=function(x,y)
	end


	return it
end

	return charmap
end


