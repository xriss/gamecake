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

function M.bake(oven,tilemap)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat
	
	tilemap.text=oven.rebake("wetgenes.gamecake.fun.tilemap_text") -- require text sub module

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
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.tiles =assert(it.system.components[opts.tiles  or "tiles" ])
	it.opts=opts
	it.component="tilemap"
	it.name=opts.name or it.component

	it.autosize=it.opts.autosize

	it.layer=opts.layer or 1
	
	it.px=it.opts.scroll and it.opts.scroll[1] or 0	-- scroll
	it.py=it.opts.scroll and it.opts.scroll[2] or 0

	it.window_px=it.opts.window and it.opts.window[1] or 0
	it.window_py=it.opts.window and it.opts.window[2] or 0
	it.window_hx=it.opts.window and it.opts.window[3] or it.screen.hx
	it.window_hy=it.opts.window and it.opts.window[4] or it.screen.hy

	it.tilemap_hx=it.opts.tilemap_size and it.opts.tilemap_size[1] or 256
	it.tilemap_hy=it.opts.tilemap_size and it.opts.tilemap_size[2] or 256
	
--	it.tilemap_hx=2^math.ceil( math.log(it.tilemap_hx)/math.log(2) ) -- force power of 2?
--	it.tilemap_hy=2^math.ceil( math.log(it.tilemap_hy)/math.log(2) )
	
	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or it.tiles.tile_hx -- cache the tile size, or allow it to change per map
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or it.tiles.tile_hy
	
	it.setup=function(opts)
		
		it.tilemap_grd =wgrd.create("U8_RGBA", it.tilemap_hx           , it.tilemap_hy           , 1)

		it.tilemap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

		it.dirty(true)

	end

	it.clean=function()
		if it.tilemap_tex then
			gl.DeleteTexture( it.tilemap_tex )
			it.tilemap_tex=nil
		end
	end

	it.update=function()
	end
	
	it.draw=function()

		if it.dirty() then
		
			it.dirty(false)

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

		end

		gl.Color(1,1,1,1)

		local dl={ color={1,1,1,1} , dx=0 , dy=0 }

		local x,y,hx,hy=it.window_px+dl.dx , it.window_py+dl.dy , it.window_hx , it.window_hy
		local u,v,hu,hv=it.px/it.tile_hx , it.py/it.tile_hy , hx/it.tile_hx , hy/it.tile_hy
		local t={
			x,		y+hy,	0,	u,		v+hv, 			
			x,		y,		0,	u,		v,
			x+hx,	y+hy,	0,	u+hu,	v+hv, 			
			x+hx,	y,		0,	u+hu,	v,
		}


		flat.tristrip("rawuv",t,"fun_draw_tilemap",function(p)

			gl.Uniform2f( p:uniform("projection_zxy"), it.screen.zx,it.screen.zy)

			gl.ActiveTexture(gl.TEXTURE2) gl.Uniform1i( p:uniform("tex_cmap"), 2 )
			gl.BindTexture( gl.TEXTURE_2D , it.colors.cmap_tex )

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_tile"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.tiles.bitmap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_map"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )


			gl.Uniform4f( p:uniform("tile_info"),	it.tile_hx,
													it.tile_hy,
													it.tiles.hx,
													it.tiles.hy )
			gl.Uniform4f( p:uniform("map_info"), 	0,0,it.tilemap_hx,it.tilemap_hy )

			gl.Uniform4f( p:uniform("color"), 	dl.color[1],dl.color[2],dl.color[3],dl.color[4] )

		end)

	end

	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end
	
-- add text functions
	tilemap.text.inject(it,opts)

	return it
end

	return tilemap
end


