--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sprites)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

sprites.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return sprites
end

sprites.setup=function()

	sprites.load()

	return sprites
end

sprites.create=function(it,opts)
	it=it or {}
	it.opts=opts
	it.component="sprites"
	it.name=opts.name

	it.sprite_xh=it.opts.char_size and it.opts.char_size[1] or 8
	it.sprite_yh=it.opts.char_size and it.opts.char_size[2] or 8

	it.bitmap_xh=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_yh=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16
	

	it.setup=function(opts)
		it.screen=opts.screen
		
		it.xp=0 -- display x offset 1 is a single char wide
		it.yp=0 -- display y offset 1 is a single char high
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.sprite_xh*it.bitmap_xh , it.sprite_yh*it.bitmap_yh , 1)

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

	end

	it.clean=function()
		if it.bitmap_tex then
			gl.DeleteTexture( it.bitmap_tex )
			it.bitmap_tex=nil
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

--[[
		local x,y,xh,yh=0,0,it.screen.xh,it.screen.yh
		local u,v,uh,vh=0,0,it.screen.xh/it.char_xh,it.screen.yh/it.char_yh
		local t={
			x,		y+yh,	0,	u,		v+vh, 			
			x,		y,		0,	u,		v,
			x+xh,	y+yh,	0,	u+uh,	v+vh, 			
			x+xh,	y,		0,	u+uh,	v,
		}


		flat.tristrip("rawuv",t,"fun_draw_sprites",function(p)

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_map"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.sprites_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_char"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )

			gl.Uniform4f( p:uniform("char_info"), it.char_xh,it.char_yh,it.char_xh*it.bitmap_xh,it.char_yh*it.bitmap_yh )
			gl.Uniform4f( p:uniform("map_info"),  0,0,it.sprites_xh,it.sprites_yh )

		end)
]]

	end

	return it
end

	return sprites
end


