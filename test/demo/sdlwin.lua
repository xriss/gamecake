--
-- tutorial.lua -- 02 opening a window
--

local SDL	= require "SDL"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local win = assert( SDL.createWindow {
	title	= "02 - Opening a window",	-- optional
	width	= 320,				-- optional
	height	= 320,				-- optional
	flags	= { SDL.window.Resizable , SDL.flags.OpenGL }	-- optional
} )

win:setFullscreen(SDL.window.Desktop)
--win:setFullscreen(SDL.window.Fullscreen)



local rdr = assert( SDL.createRenderer( win, -1) )

	rdr:setDrawColor(0xFF0000)
	rdr:clear()
	rdr:present()
	
	
-- Let the window opened a bit
SDL.delay(5000)


