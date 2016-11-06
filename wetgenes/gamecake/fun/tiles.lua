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

function M.bake(oven,tiles)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

tiles.load=function()

--	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
--	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return tiles
end

tiles.setup=function()

	tiles.load()

	return tiles
end

tiles.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.opts=opts
	it.component="tiles"
	it.name=opts.name or it.component
	
	it.names={}
	it.tiles={}
	
	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or 8
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or 8

	it.bitmap_hx=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_hy=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16
	
	it.hx=it.tile_hx*it.bitmap_hx
	it.hy=it.tile_hy*it.bitmap_hy
	
--	it.hx=2^math.ceil( math.log(it.hx)/math.log(2) ) -- force power of 2?
--	it.hy=2^math.ceil( math.log(it.hy)/math.log(2) )


	it.drawtype=opts.drawtype or "first"

	it.setup=function(opts)
		
		it.bitmap_grd  =wgrd.create("U8_RGBA", it.hx , it.hy , 1)

		it.bitmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.bitmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		
		it.dirty(true)

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

-- update the bitmap, but do not draw anything
		if it.dirty() then
			it.dirty(false)
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
		end

	end

-- optionally keep track of bitmap tiles with names	
	it.set_tile_name=function(tile,name,data)
		if not name then name=string.format("tile_%04X",tile) end
		it.names[name]=tile
		it.tiles[tile]=data

	end
-- upload all the named tiles previously set with set_tile_name
	it.upload_tiles=function()
		bitdown.pixtab_tiles( it.tiles, it.colors.cmap , it )
	end
	
	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end

	return it
end

	return tiles
end


