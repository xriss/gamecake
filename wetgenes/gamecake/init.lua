-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple 2D game framework that uses GLES or CPU renders to a bitmap.
-- It bears a passing resemblence to gamecake js.



local win=win

module("wetgenes.gamecake")

local wcanvas=require("wetgenes.gamecake.canvas")
local wimages=require("wetgenes.gamecake.images")
local wsounds=require("wetgenes.gamecake.sounds")

local grd=require("wetgenes.grd")

base=require(...)
meta={}
meta.__index=base

function bake(opts)

	local cake={}
	setmetatable(cake,meta)
	
	opts.cake=cake -- insert cake into opts
	cake.opts=opts -- and opts into cake
	cake.gl=opts.gl
	
	cake.canvas_fmt=opts.canvas_fmt or grd.FMT_U16_RGB_565
	cake.images_fmt=opts.images_fmt or grd.FMT_U16_RGBA_4444_PREMULT
	
	
	cake.canvas=wcanvas.bake(opts) -- we will need a canvas to draw too
	cake.images=wimages.bake(opts) -- we will need to load some images
	cake.sounds=wsounds.bake(opts) -- we will need to load some sounds

	return cake
end

setup = function(cake)
	cake.sounds:setup()
end

clean = function(cake)
	cake.sounds:clean()
end

start = function(cake)
	cake.canvas:start()
	cake.images:start()
	cake.sounds:start()
end

stop = function(cake)
	cake.canvas:stop()
	cake.images:stop()
	cake.sounds:stop()
end

blit = function(cake,id,name,cx,cy,ix,iy,w,h)
	cake.canvas:blit(cake.images:get(id,name),cx,cy,ix,iy,w,h)
end

beep = function(cake,id,name)
	cake.sounds:beep(cake.sounds:get(id,name))
end

-- draw a prebuilt texture using win and opengl functions
-- do not call if you do not have fenestra and a global win setup.
gldraw = function(cake)
	if win then
		local gl=require("gl")
		local t=assert(win.tex( cake.canvas.grd ))
		t:draw()--{max=gl.NEAREST,min=gl.NEAREST})
		t:clean()
	end
end



--
-- build a simple field of view projection matrix designed to work in 2d or 3d and keep the numbers
-- easy for 2d positioning.
--
-- setting aspect to 640,480 and fov of 1 would mean at a z depth of 240 (which is y/2) then your view area would be
-- -320 to +320 in the x and -240 to +240 in the y.
--
-- fov is a tan like value (a view size inverse scalar) so 1 would be 90deg, 0.5 would be 45deg and so on
--
-- the depth parameter is only used to limit the range of the zbuffer so it covers 0 to depth
--
-- The following would be a reasonable default for a 640x480 screen.
--
-- build_project23d(640,480,0.5,1024)
--
-- then at z=((480/2)/0.5)=480 we would have one to one pixel scale...
-- the total view area volume from there would be -320 +320 , -240 +240 , -480 +(1024-480)
--
-- screen needs to contain width and height of the display which we use to work
-- out where to place our view such that it is always visible and keeps its aspect.
--
function build_project23d(screen,width,height,fov,depth)

	local aspect=height/width
	
	local m={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- defaults
	local f=depth
	local n=1

	local screen_aspect=(screen.height/screen.width)
		
	if (screen_aspect > (aspect) ) 	then 	-- fit width to screen
	
		m[1] = ((aspect)*1)/fov
		m[6] = -((aspect)/screen_aspect)/fov
		
		screen.xs=1
		screen.ys=-screen_aspect/aspect
	else									-- fit height to screen
	
		m[1] = screen_aspect/fov
		m[6] = -1/fov
		
		screen.xs=aspect/screen_aspect
		screen.ys=-1
	end
	
	
	m[11] = -(f+n)/(f-n)
	m[12] = -1

	m[15] = -2*f*n/(f-n)
	
	return m
end




