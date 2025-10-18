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



	local bfontgrd=require("wetgenes.gamecake.fun.bitdown_font").build_grd(8,16,"r")
		
	local g=wgrd.create(wgrd.FMT_U8_RGBA_PREMULT,8*256,16*1,1)

	for i=0,255 do -- setup base textures for 7bit ascii
		
		local dx=math.floor((i)%256)*8
		local dy=math.floor((i)/256)*16
		g:pixels(dx,dy,8,16,bfontgrd:pixels(i*8,0,8,16,"")) -- splat into grid

	end
	
	wtiles.grdfont8x16=g

	return wtiles
end

function wtiles.update(widget)

--	if widget.data then
--		widget.text=widget.data.str
--	end

	return widget.meta.update(widget)
end

function wtiles.render_lines(widget,lines)

	local g=widget.tilemap_grd
	g:clear(widget.background_tile or 0x00000000)
	for i,v in ipairs(lines) do
		local s=v.s
		if s then
			g:pixels(0,i-1,#s/4,1,s)
		end
	end

end


function wtiles.skin(widget)
		
		wtiles.render_lines(widget,widget.lines or {} )
				
		local x,y,hx,hy=widget.px,widget.py,widget.hx,widget.hy -- it.window_px+dl.dx , it.window_py+dl.dy , it.window_hx , it.window_hy

		local u,v,hu,hv=0,0,hx/8,hy/16 -- it.px/it.tile_hx , it.py/it.tile_hy , hx/it.tile_hx , hy/it.tile_hy

		local ta=gl.apply_modelview( {x    ,y+hy  ,0,1} )
		local tb=gl.apply_modelview( {x    ,y     ,0,1} )
		local tc=gl.apply_modelview( {x+hx ,y+hy  ,0,1} )
		local td=gl.apply_modelview( {x+hx ,y     ,0,1} )

		local t={
			ta[1],	ta[2],	ta[3],	u,		v+hv, 			
			tb[1],	tb[2],	tb[3],	u,		v,
			tc[1],	tc[2],	tc[3],	u+hu,	v+hv, 			
			td[1],	td[2],	td[3],	u+hu,	v,
		}

		
	return function()

--print("FONt DRAW")

--font
		gl.BindTexture( gl.TEXTURE_2D , widget.bitmap_tex )
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			wtiles.grdfont8x16.width,
			wtiles.grdfont8x16.height,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			wtiles.grdfont8x16.data )

--display
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

--colors
		gl.BindTexture( gl.TEXTURE_2D , widget.colormap_tex )
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			256,
			1,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			widget.colormap_grd.data )

		gl.Color(1,1,1,1)

--		local dl={ color={1,1,1,1} , dx=0 , dy=0 }

		flat.tristrip("rawuv",t,"widget_draw_texteditor_tilemap",function(p)

--			gl.Uniform2f( p:uniform("projection_zxy"), it.screen.zx,it.screen.zy)

			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			gl.Uniform1i( p:uniform("tex_tile"), gl.NEXT_UNIFORM_TEXTURE )
			gl.BindTexture( gl.TEXTURE_2D , widget.bitmap_tex )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			gl.Uniform1i( p:uniform("tex_map"), gl.NEXT_UNIFORM_TEXTURE )
			gl.BindTexture( gl.TEXTURE_2D , widget.tilemap_tex )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture( gl.TEXTURE0 + gl.NEXT_UNIFORM_TEXTURE )
			gl.Uniform1i( p:uniform("tex_cmap"), gl.NEXT_UNIFORM_TEXTURE )
			gl.BindTexture( gl.TEXTURE_2D , widget.colormap_tex )
			gl.NEXT_UNIFORM_TEXTURE=gl.NEXT_UNIFORM_TEXTURE+1

			gl.ActiveTexture( gl.TEXTURE0 )

			gl.Uniform4f( p:uniform("tile_info"),	8,
													16,
													256*8,
													1*16 )
			gl.Uniform4f( p:uniform("map_info"), 	0,0,512,256 )

--[[
			for i,v in ipairs{
				0xff336622, 0xff66aa33, 0xff66cccc, 0xff5577cc,
				0xff333366, 0xff442233, 0xff884433, 0xffeeaa99,
				0xffcc3333, 0xffdd7733, 0xffdddd44, 0xff000000,
				0xff444444, 0xff666666, 0xffaaaaaa, 0xffffffff,
			} do
				local n=("0123456789ABCDEF"):sub(i,i)
				gl.Uniform4f( p:uniform("colors_"..n), 	gl.C8(v) )
			end
]]

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
	widget.colormap_tex=gl.GenTexture()

	gl.BindTexture( gl.TEXTURE_2D , widget.bitmap_tex )
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

	gl.BindTexture( gl.TEXTURE_2D , widget.tilemap_tex )
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

	gl.BindTexture( gl.TEXTURE_2D , widget.colormap_tex )
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

	widget.lines={}

	widget.tilemap_grd=wgrd.create(wgrd.FMT_U8_RGBA,512,256,1)
	widget.colormap_grd=wgrd.create(wgrd.FMT_U8_RGBA,256,1,1)
	
	widget.colormap_grd:pixels(0,0,256,1,require("wetgenes.gamecake.fun.bitdown").cmap_swanky32.grd) -- copy swanky32

	return widget
end

return wtiles
end
