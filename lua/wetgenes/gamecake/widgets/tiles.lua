--
-- (C) 2015 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd=require('wetgenes.grd')
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wtiles)
wtiles=wtiles or {}

local gl=oven.gl
local cake=oven.cake
local canvas=cake.canvas
local flat=canvas.flat

wtiles.loads=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return wtiles
end

function wtiles.update(widget)

--	if widget.data then
--		widget.text=widget.data.str
--	end

	return widget.meta.update(widget)
end

--function wtiles.draw(widget)
--	return widget.meta.draw(widget)
--end

function wtiles.skin(widget)
		print("skin1111")
	return function()

		gl.BindTexture( gl.TEXTURE_2D , widget.tilemap_tex )
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			widget.tilemap_grd.width,
			widget.tilemap_grd.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			widget.tilemap_grd.data )

		gl.Color(1,1,1,1)

--		local dl={ color={1,1,1,1} , dx=0 , dy=0 }

		local x,y,hx,hy=0,0,256,256 -- it.window_px+dl.dx , it.window_py+dl.dy , it.window_hx , it.window_hy
		local u,v,hu,hv=0,0,256,256 -- it.px/it.tile_hx , it.py/it.tile_hy , hx/it.tile_hx , hy/it.tile_hy
		local t={
			x,		y+hy,	0,	u,		v+hv, 			
			x,		y,		0,	u,		v,
			x+hx,	y+hy,	0,	u+hu,	v+hv, 			
			x+hx,	y,		0,	u+hu,	v,
		}


		flat.tristrip("rawuv",t,"widget_draw_texteditor_tilemap",function(p)

--			gl.Uniform2f( p:uniform("projection_zxy"), it.screen.zx,it.screen.zy)

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_tile"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , widget.bitmap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_map"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , widget.tilemap_tex )


			gl.Uniform4f( p:uniform("tile_info"),	8,
													16,
													256,
													256 )
			gl.Uniform4f( p:uniform("map_info"), 	0,0,256,256 )

--			gl.Uniform4f( p:uniform("color"), 	dl.color[1],dl.color[2],dl.color[3],dl.color[4] )

		end)


	end
end


function wtiles.setup(widget,def)
--	local it={}
--	widget.button=it
	widget.class="tiles"

	widget.pan_px=0
	widget.pan_py=0

	widget.hx_max=widget.hx
	widget.hy_max=widget.hy

	widget.key=wtiles.key
	widget.mouse=wtiles.mouse
	widget.update=wtiles.update
	widget.draw=wtiles.draw
	widget.skin=wtiles.skin
	
	widget.color=widget.color or 0 -- presume we want to be visible
	
	widget.bitmap_tex=gl.GenTexture()
	widget.tilemap_tex=gl.GenTexture()

	widget.bitmap_grd=wgrd.create(wgrd.U8_RGBA,256,256,1)
	widget.tilemap_grd=wgrd.create(wgrd.U8_RGBA,256,256,1)


	return widget
end

return wtiles
end
