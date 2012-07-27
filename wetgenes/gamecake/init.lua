-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple 2D game framework that uses GLES or CPU renders to a bitmap.
-- It bears a passing resemblence to gamecake js.


module("wetgenes.gamecake")

local wcanvas =require("wetgenes.gamecake.canvas")
local wbuffers=require("wetgenes.gamecake.buffers")
local wimages =require("wetgenes.gamecake.images")
local wsheets =require("wetgenes.gamecake.sheets")
local wsounds =require("wetgenes.gamecake.sounds")
local wfonts  =require("wetgenes.gamecake.fonts")

local grd=require("wetgenes.grd")

base=require(...)
meta={}
meta.__index=base

function bake(opts)

	local cake={}
	setmetatable(cake,meta)
	
	opts.cake=cake -- insert cake into opts
	cake.opts=opts -- and opts into cake

--	get gl and win from state if we need them
	opts.gl=opts.gl or (opts.state and opts.state.gl)
	opts.win=opts.win or (opts.state and opts.state.win)

-- cache stuffs	
	cake.state=opts.state
	cake.gl=opts.gl
	cake.win=opts.win
	
	if cake.gl then cake.gl.GetExtensions() end
	
--	cake.bake_canvas=function() return wcanvas.bake(opts)	end -- a canvas generator
	
	
	cake.buffers = wbuffers.bake(opts) -- generic buffer memory is now complex thanks to retardroid
	cake.images  =  wimages.bake(opts) -- we will need to load some images
	cake.sheets  =  wsheets.bake(opts) -- we will need to manage some sprite sheets
	cake.fonts   =   wfonts.bake(opts) -- we will need to load some fonts
	cake.sounds  =  wsounds.bake(opts) -- we will need to load some sounds

	cake.canvas  =  wcanvas.bake(opts) -- a canvas contains current drawing state and functions
	
	if cake.state then -- squirt some shortcuts into the state such as
		cake.state.cake=cake -- the cake upon which we rest
		cake.state.canvas=cake.canvas -- the default canvas to draw upon
	end
	
	return cake
end

setup = function(cake)
	if not cake.opts.disable_sounds then
--		cake.sounds:setup()
	end
end

clean = function(cake)
	if not cake.opts.disable_sounds then
--		cake.sounds:clean()
	end
end

start = function(cake)
print("cakestart")
	cake.buffers:start()
	cake.images:start()
	cake.sheets:start()
	cake.fonts:start()
	if not cake.opts.disable_sounds then
--		cake.sounds:start()
	end
end

stop = function(cake)
print("cakestop")
	cake.fonts:stop()
	cake.sheets:stop()
	cake.images:stop()
	cake.buffers:stop()
	if not cake.opts.disable_sounds then
--		cake.sounds:stop()
	end
	if cake.gl.forget then -- any programs will need to be recompiled
		cake.gl.forget()
	end
end

-- wrapper for bliting by id/name
blit = function(cake,id,name,cx,cy,ix,iy,w,h)
	cake.canvas:blit(cake.images:get(id,name),cx,cy,ix,iy,w,h)
end
