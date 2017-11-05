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

function M.bake(oven,overmap)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat
	
overmap.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return overmap
end

overmap.setup=function()

	overmap.load()

	return overmap
end

overmap.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.tiles =assert(it.system.components[opts.tiles  or "tiles" ])
	it.opts=opts
	it.component="overmap"
	it.name=opts.name or it.component

	it.layer=opts.layer or 1
	
	it.sortx=it.opts.sort and it.opts.sort[1] or 1	-- sort
	it.sorty=it.opts.sort and it.opts.sort[2] or 1

	it.px=it.opts.scroll and it.opts.scroll[1] or 0	-- scroll
	it.py=it.opts.scroll and it.opts.scroll[2] or 0

	it.window_px=it.opts.window and it.opts.window[1] or 0
	it.window_py=it.opts.window and it.opts.window[2] or 0
	it.window_hx=it.opts.window and it.opts.window[3] or it.screen.hx
	it.window_hy=it.opts.window and it.opts.window[4] or it.screen.hy

	it.tilemap_hx=it.opts.tilemap_size and it.opts.tilemap_size[1] or 256
	it.tilemap_hy=it.opts.tilemap_size and it.opts.tilemap_size[2] or 256
	
	it.tile_hx=it.opts.tile_size and it.opts.tile_size[1] or it.tiles.tile_hx -- cache the base tile size,
	it.tile_hy=it.opts.tile_size and it.opts.tile_size[2] or it.tiles.tile_hy
	it.tile_hz=it.opts.tile_size and it.opts.tile_size[3] or it.tiles.tile_hy
	
	it.over_hx=it.opts.over_size and it.opts.over_size[1] or (it.tile_hx/2) -- overlap right ( 50% default )
	it.over_hy=it.opts.over_size and it.opts.over_size[2] or (it.tile_hy/2) -- overlap up    ( 50% default )

	it.mode=it.opts.mode or "xy" -- xy for 2d or xz for "fake" 3d

	it.ax=it.opts.add and it.opts.add[1] or 0
	it.ay=it.opts.add and it.opts.add[2] or 0
	it.az=it.opts.add and it.opts.add[3] or 0

	it.setup=function(opts)
		
		it.tilemap_grd =wgrd.create("U8_RGBA", it.tilemap_hx           , it.tilemap_hy           , 1)

		it.tilemap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		
		local t={}
		local phx=(it.tile_hx-it.over_hx)
		local phy=(it.tile_hy-it.over_hy)
		local phz=(it.tile_hz)
		local thx=(it.tile_hx)
		local thy=(it.tile_hy)
		local xoa,xob,xoc=0,it.tilemap_hx-1,1		if it.sortx==-1 then xoa,xob,xoc=it.tilemap_hx-1,0,-1 end
		local yoa,yob,yoc=0,it.tilemap_hy-1,1		if it.sorty==-1 then yoa,yob,yoc=it.tilemap_hy-1,0,-1 end
		for yz=yoa,yob,yoc do
			local y,z=0,0
			if it.mode=="xy" then y=yz end -- 2d
			if it.mode=="xz" then z=-yz end -- 3d
			for x=xoa,xob,xoc do
			
				local bx=phx*x --+phz*it.screen.zx*z
				local by=phy*y --+phz*it.screen.zy*z
				local bz=phz*z

				local l=#t ; for i,v in ipairs{

					bx,		by+thy,	bz,		x,		yz+1,
					bx,		by,		bz,		x,		yz,
					bx+thx,	by,		bz,		x+1,	yz,

					bx,		by+thy,	bz,		x,		yz+1,
					bx+thx,	by,		bz,		x+1,	yz,
					bx+thx,	by+thy,	bz,		x+1,	yz+1,

				} do t[l+i]=v end

			end
		end

		it.gl=flat.array_predraw({fmt="rawuv",data=t,array=gl.TRIANGLES,vb=-1})

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
		it.gl.draw(function(p)

			gl.Uniform2f( p:uniform("projection_zxy"), it.screen.zx,it.screen.zy)
			gl.Uniform3f( p:uniform("modelview_add"), it.ax,it.ay,it.az)

			gl.ActiveTexture(gl.TEXTURE2) gl.Uniform1i( p:uniform("tex_cmap"), 2 )
			gl.BindTexture( gl.TEXTURE_2D , it.colors.cmap_tex )

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_tile"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.tiles.bitmap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_map"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.tilemap_tex )


			gl.Uniform4f( p:uniform("over_info"),	it.tile_hx,
													it.tile_hy,
													0,0)
			gl.Uniform4f( p:uniform("tile_info"),	it.tiles.tile_hx,
													it.tiles.tile_hy,
													it.tiles.hx,
													it.tiles.hy)
			gl.Uniform4f( p:uniform("map_info"), 	0,0,it.tilemap_hx,it.tilemap_hy )

			gl.Uniform4f( p:uniform("color"), 	dl.color[1],dl.color[2],dl.color[3],dl.color[4] )

		end,"fun_draw_overmap")

	end

	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end
	
	return it
end

	return overmap
end


