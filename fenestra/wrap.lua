

local print=print

local core = require("fenestra.core")
local console = require("fenestra.console")
local data = require("fenestra.data")
local avatar = require("fenestra.avatar")
local font = require("fenestra.font")
local widget = require("fenestra.widget")

local gl=require("gl")

module("fenestra.wrap")

--
-- Call win to get a unique table full of functions associated
-- with lots of tasty up values for easy use
--
-- local win=require("fenestra.wrap").win()
--
function win(opts)
opts=opts or {}

local win={}

	function win.setup(g)
	
		win._g=g -- the global table
		win.into_hwnd=opts.into_hwnd

-- open window of this height		
		win.width=opts.width
		win.height=opts.height

		win.core = core.setup(win)
		
		win.core_data = core.data.setup(win.core)
		
		win.data = data.setup(win)
		
		win.core_ogl = core.ogl.setup(win.core)
		
		win.console = console.setup(win)		
		win.restore_print=win.console.replace_print(g)
		
		win.avatar = avatar.setup(win)
		
		win.font_base=font.setup(win,"base")
		win.font_sans=font.setup(win,"sans")
		win.font_debug=win.font_base -- old name, do not use...
		
		win.widget=widget.setup(win,{font=win.font_sans})
		
		return win
	end

	function win.clean()
		
		win.widget.clean()
		
		win.font_sans.clean()
		win.font_base.clean()

		win.avatar.clean()
		
		win.restore_print()
		win.console.clean()
		
		core.ogl.clean(win.core_ogl)
		
		win.data.clean()
		
		core.data.clean(win.core_data)
		
		core.clean(win.core)

	end

	function win.xox(xox_info)

		local core=win.xox_setup(xox_info)
		local xox=win.xox_get(core)
		xox.info=xox_info
		xox.core=core
		
		function xox.clean()
			return win.xox_clean(core)
		end
		
		function xox.draw()
			return win.xox_draw(core)
		end

		function xox.set()
			return win.xox_set(core,xox)
		end
		
		return xox
	end
	
	function win.xsx(xsx_info)

		local core=win.xsx_setup(xsx_info)
		local xsx=win.xsx_get(core)
		xsx.info=xsx_info
		xsx.core=core
		
		function xsx.clean()
			return win.xsx_clean(core)
		end
		
		function xsx.draw(f)
			return win.xsx_draw(core,f)
		end
		
		function xsx.set()
			return win.xsx_set(core,xsx)
		end
		
		return xsx

	end


	function win.fbo(width,height,depth)
	
		depth=depth or 0
		
		local core=win.fbo_setup(width,height,depth)
		local fbo={}
		fbo.core=core
		fbo.width=width
		fbo.height=height
		fbo.depth=depth
		
		function fbo.clean(fbo)
			return win.fbo_clean(core)
		end
		
		function fbo.bind(fbo)
			return win.fbo_bind(core)
		end

		function fbo.texture(fbo)
			return win.fbo_texture(core)
		end

		function fbo.draw(fbo)
			win.fbo_texture(core)
			gl.Disable(gl.CULL_FACE)
			gl.Enable(gl.TEXTURE_2D)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
			gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA) --fbo has premultipliedalpha
			gl.Begin(gl.QUADS)
				gl.Color({1,1,1,1})
				gl.TexCoord(0, 0) gl.Vertex(fbo.width*-0.5, fbo.height*-0.5)
				gl.TexCoord(1, 0) gl.Vertex(fbo.width* 0.5, fbo.height*-0.5)
				gl.TexCoord(1, 1) gl.Vertex(fbo.width* 0.5, fbo.height* 0.5)
				gl.TexCoord(0, 1) gl.Vertex(fbo.width*-0.5, fbo.height* 0.5)
			gl.End()
		end

		return fbo

	end

	function win.tex(grd) -- this takes a copy of the given grd
	
		local core=win.tex_setup(grd)
		local tex={}
		tex.core=core
		tex.width=grd.width
		tex.height=grd.height
		tex.depth=grd.depth
		
		grd=nil -- do not keep any references
		
		function tex.clean(tex)
			return win.tex_clean(core)
		end
		
		function tex.bind(tex)
			return win.tex_bind(core)
		end

		function tex.draw(tex)
			win.tex_bind(core)
			gl.Disable(gl.CULL_FACE)
			gl.Enable(gl.TEXTURE_2D)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
			gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
			gl.Begin(gl.QUADS)
				gl.Color({1,1,1,1})
				gl.TexCoord(0, 0) gl.Vertex(tex.width*-0.5, tex.height* 0.5)
				gl.TexCoord(1, 0) gl.Vertex(tex.width* 0.5, tex.height* 0.5)
				gl.TexCoord(1, 1) gl.Vertex(tex.width* 0.5, tex.height*-0.5)
				gl.TexCoord(0, 1) gl.Vertex(tex.width*-0.5, tex.height*-0.5)
			gl.End()
		end

		return tex

	end

	
-- return window x,y pos transformed into the viewspace of project23d if you provided h/w to that function
	function win.mouse23d(w,h,x,y)
	
		local hx=win.width/2
		local hy=win.height/2

		local tx,ty
		
		if win.height/(win.width or 1) > (h/w) then -- deal with new "smart" viewport sizeing
		
			tx=(w/ 2)*(x-hx)/hx
			ty=(w/-2)*(hy-y)/hx
			
		else
		
			tx=(h/ 2)*(x-hx)/hy
			ty=(h/-2)*(hy-y)/hy

		end

		return tx,ty
	end

function win.choose_file(...)			return core.choose_file(				win.core,		...) end

-- default key and mouse functions just send data to the debug console	
function win.keypress(...)				return win.console.keypress(							...) end
function win.mouse(...)					return win.console.mouse(								...) end

function win.msg(...)					return core.msg(						win.core,		...) end
function win.time(...)					return core.time(						win.core,		...) end

function win.getwin(...)				return core.getwin(						win.core,		...) end
function win.setwin(...)				return core.setwin(						win.core,		...) end

function win.get(...)					return core.ogl.get(					win.core_ogl,	...) end
function win.set(...)					return core.ogl.set(					win.core_ogl,	...) end
function win.begin(...)					return core.ogl.begin(					win.core_ogl,	...) end
function win.clip2d(...)				return core.ogl.clip2d(					win.core_ogl,	...) end
function win.project23d(...)			return core.ogl.project23d(				win.core_ogl,	...) end
function win.swap(...)					return core.ogl.swap(					win.core_ogl,	...) end

function win.target(...)				return core.ogl.target(					win.core_ogl,	...) end
function win.readpixels(...)			return core.ogl.readpixels(				win.core_ogl,	...) end


function win.debug_begin(...)			return core.ogl.debug_begin(			win.core_ogl,	...) end
function win.debug_end(...)				return core.ogl.debug_end(				win.core_ogl,	...) end
function win.debug_print(...)			return core.ogl.debug_print(			win.core_ogl,	...) end
function win.debug_print_alt(...)		return core.ogl.debug_print_alt(		win.core_ogl,	...) end
function win.debug_rect(...)			return core.ogl.debug_rect(				win.core_ogl,	...) end
function win.debug_polygon_begin(...)	return core.ogl.debug_polygon_begin(	win.core_ogl,	...) end
function win.debug_polygon_vertex(...)	return core.ogl.debug_polygon_vertex(	win.core_ogl,	...) end
function win.debug_polygon_end(...)		return core.ogl.debug_polygon_end(		win.core_ogl,	...) end


function win.flat_begin(...)			return core.ogl.flat_begin(				win.core_ogl,	...) end
function win.flat_end(...)				return core.ogl.flat_end(				win.core_ogl,	...) end
function win.flat_font(...)				return core.ogl.flat_font(				win.core_ogl,	...) end
function win.flat_print(...)			return core.ogl.flat_print(				win.core_ogl,	...) end
function win.flat_measure(...)			return core.ogl.flat_measure(			win.core_ogl,	...) end
function win.flat_which(...)			return core.ogl.flat_which(				win.core_ogl,	...) end
function win.flat_fits(...)				return core.ogl.flat_fits(				win.core_ogl,	...) end
function win.flat_rect(...)				return core.ogl.debug_rect(				win.core_ogl,	...) end
function win.flat_polygon_begin(...)	return core.ogl.debug_polygon_begin(	win.core_ogl,	...) end
function win.flat_polygon_vertex(...)	return core.ogl.debug_polygon_vertex(	win.core_ogl,	...) end
function win.flat_polygon_end(...)		return core.ogl.debug_polygon_end(		win.core_ogl,	...) end


function win.draw_cube(...)				return core.ogl.draw_cube(				win.core_ogl,	...) end

function win.xox_setup(...)				return core.ogl.xox_setup(				win.core_ogl,	...) end
function win.xox_clean(...)				return core.ogl.xox_clean(				win.core_ogl,	...) end
function win.xox_draw(...)				return core.ogl.xox_draw(				win.core_ogl,	...) end
function win.xox_get(...)				return core.ogl.xox_get(				win.core_ogl,	...) end
function win.xox_set(...)				return core.ogl.xox_set(				win.core_ogl,	...) end

function win.xsx_setup(...)				return core.ogl.xsx_setup(				win.core_ogl,	...) end
function win.xsx_clean(...)				return core.ogl.xsx_clean(				win.core_ogl,	...) end
function win.xsx_draw(...)				return core.ogl.xsx_draw(				win.core_ogl,	...) end
function win.xsx_get(...)				return core.ogl.xsx_get(				win.core_ogl,	...) end
function win.xsx_set(...)				return core.ogl.xsx_set(				win.core_ogl,	...) end

function win.fbo_setup(...)				return core.ogl.fbo_setup(				win.core_ogl,	...) end
function win.fbo_clean(...)				return core.ogl.fbo_clean(				win.core_ogl,	...) end
function win.fbo_bind(...)				return core.ogl.fbo_bind(				win.core_ogl,	...) end
function win.fbo_texture(...)			return core.ogl.fbo_texture(			win.core_ogl,	...) end

function win.tex_setup(...)				return core.ogl.tex_setup(				win.core_ogl,	...) end
function win.tex_clean(...)				return core.ogl.tex_clean(				win.core_ogl,	...) end
function win.tex_bind(...)				return core.ogl.tex_bind(				win.core_ogl,	...) end

function win.load(...)					return core.data.load(					win.core_data,	...) end
	
	return win
end

