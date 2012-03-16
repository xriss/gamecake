-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple 2D game framework that uses GLES or CPU renders to a bitmap.
-- It bears a passing resemblence to gamecake js.



local win=win

module("wetgenes.gamecake")

local wcanvas=require("wetgenes.gamecake.canvas")
local wimages=require("wetgenes.gamecake.images")
local wsounds=require("wetgenes.gamecake.sounds")
local wscreen=require("wetgenes.gamecake.screen")
local wfonts =require("wetgenes.gamecake.fonts")

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
	cake.fonts = wfonts.bake(opts) -- we will need to load some fonts
	cake.sounds=wsounds.bake(opts) -- we will need to load some sounds
	cake.screen=wscreen.bake(opts) -- how to display a canvas

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
	cake.fonts:start()
	cake.sounds:start()
end

stop = function(cake)
	cake.canvas:stop()
	cake.images:stop()
	cake.fonts:stop()
	cake.sounds:stop()
end

blit = function(cake,id,name,cx,cy,ix,iy,w,h)
	cake.canvas:blit(cake.images:get(id,name),cx,cy,ix,iy,w,h)
end

beep = function(cake,id,name)
	cake.sounds:beep(cake.sounds:get(id,name))
end

-- draw a prebuilt texture using win and opengl functions
-- do not call if you do not have fenestra and a global win setup?
-- probably need to remove this
gldraw = function(cake)
	if win then
		local gl=require("gl")
		local t=assert(win.tex( cake.canvas.grd ))
		t:draw()--{max=gl.NEAREST,min=gl.NEAREST})
		t:clean()
	end
end

