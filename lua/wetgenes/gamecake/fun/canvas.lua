--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wgrdpaint=require("wetgenes.grdpaint")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,canvas)

	local gl=oven.gl
	local cake=oven.cake
	local flat=cake.canvas.flat

canvas.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return canvas
end

canvas.setup=function()

	canvas.load()

	return canvas
end

canvas.create=function(it,opts)
	it.screen=assert(it.system.components[opts.screen or "screen"]) -- find linked components by name
	it.colors=assert(it.system.components[opts.colors or "colors"])
	it.opts=opts
	it.component="canvas"
	it.name=opts.name or it.component
	
	it.hx=it.opts.size and it.opts.size[1] or it.screen.hx
	it.hy=it.opts.size and it.opts.size[2] or it.screen.hy
	
	it.px=it.opts.scroll and it.opts.scroll[1] or 0	-- scroll
	it.py=it.opts.scroll and it.opts.scroll[2] or 0

	it.layer=opts.layer or 0

	it.window_px=it.opts.window and it.opts.window[1] or 0
	it.window_py=it.opts.window and it.opts.window[2] or 0
	it.window_hx=it.opts.window and it.opts.window[3] or it.screen.hx
	it.window_hy=it.opts.window and it.opts.window[4] or it.screen.hy

	it.setup=function(opts)
		
		it.grd  =wgrd.create("U8_INDEXED", it.hx , it.hy , 1)
		it.grdcanvas=wgrdpaint.canvas(it.grd)

		it.tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.tex )
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)
		
		it.dirty(true)

	end

	it.clean=function()
		if it.tex then
			gl.DeleteTexture( it.tex )
			it.tex=nil
		end
	end

	it.update=function()
	end
	
	it.draw=function()

-- update the bitmap, but do not draw anything
		if it.dirty() then
			it.dirty(false)
			
			gl.BindTexture( gl.TEXTURE_2D , it.tex )	
			gl.TexImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RED,			-- some drivers do not like luminance and want red others demand luminance...
				it.grd.width,
				it.grd.height,
				0,
				gl.RED,
				gl.UNSIGNED_BYTE,
				it.grd.data )
		end

		gl.Color(1,1,1,1)

		local dl={ color={1,1,1,1} , dx=0 , dy=0 }

		local x,y,hx,hy=it.window_px+dl.dx , it.window_py+dl.dy , it.window_hx , it.window_hy
		local u,v,hu,hv=it.px , it.py , hx , hy
		local t={
			x,		y+hy,	0,	u,		v+hv, 			
			x,		y,		0,	u,		v,
			x+hx,	y+hy,	0,	u+hu,	v+hv, 			
			x+hx,	y,		0,	u+hu,	v,
		}


		gl.DepthMask(gl.FALSE)
		flat.tristrip("rawuv",t,"fun_draw_canvas",function(p)

			gl.Uniform2f( p:uniform("projection_zxy"), it.screen.zx,it.screen.zy)

			gl.ActiveTexture(gl.TEXTURE1) gl.Uniform1i( p:uniform("tex_cmap"), 1 )
			gl.BindTexture( gl.TEXTURE_2D , it.colors.cmap_tex )

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_canvas"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , it.tex )


			gl.Uniform4f( p:uniform("canvas_info"), 	0,0,it.hx,it.hy )

			gl.Uniform4f( p:uniform("color"), 	dl.color[1],dl.color[2],dl.color[3],dl.color[4] )

		end)
		gl.DepthMask(gl.TRUE)

	end

	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end

	it.fill         = function(...) return it.grdcanvas.fill(...)         end
	it.color        = function(...) return it.grdcanvas.color(...)        end
	it.brush        = function(...) return it.grdcanvas.brush(...)        end
	it.brush_handle = function(...) return it.grdcanvas.brush_handle(...) end
	it.clear        = function(...) return it.grdcanvas.clear(...)        end
	it.plot         = function(...) return it.grdcanvas.plot(...)         end
	it.line         = function(...) return it.grdcanvas.line(...)         end
	it.box          = function(...) return it.grdcanvas.box(...)          end
	it.circle       = function(...) return it.grdcanvas.circle(...)       end
	it.set_font     = function(...) return it.grdcanvas.set_font(...)     end
	it.text         = function(...) return it.grdcanvas.text(...)         end

	return it
end

	return canvas
end


