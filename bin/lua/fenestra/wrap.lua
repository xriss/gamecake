

local print=print

local core = require("fenestra.core")
local console = require("fenestra.console")
local data = require("fenestra.data")
local avatar = require("fenestra.avatar")
local font = require("fenestra.font")
local widget = require("fenestra.widget")

module("fenestra.wrap")

--
-- Call win to get a unique table full of functions associated
-- with lots of tasty up values for easy use
--
-- local win=require("fenestra.wrap").win()
--
function win(into_hwnd)

local win={}

	function win.setup(g)
	
		win._g=g -- the global table
		win.into_hwnd=into_hwnd

		win.core = core.setup(win)
		
		win.core_data = core.data.setup(win.core)
		
		win.data = data.setup(win)
		
		win.core_ogl = core.ogl.setup(win.core)
		
		win.console = console.setup(win)		
		win.restore_print=win.console.replace_print(g)
		
		win.avatar = avatar.setup(win)
		
		win.font_debug=font.setup(win,"debug")
		
		win.widget=widget.setup(win,{font=win.font_debug})
		
		return win
	end

	function win.clean()
		
		win.widget.clean()
		
		win.font_debug.clean()

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

function win.load(...)					return core.data.load(					win.core_data,	...) end
	
	return win
end

