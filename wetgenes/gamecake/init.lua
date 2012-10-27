-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple 2D/3D game framework that targets GLESv2 on android/nacl/linux/windows/raspberrypi systems

module("wetgenes.gamecake")

local wcanvas =require("wetgenes.gamecake.canvas")
local wbuffers=require("wetgenes.gamecake.buffers")
local wimages =require("wetgenes.gamecake.images")
local wsheets =require("wetgenes.gamecake.sheets")
local wsounds =require("wetgenes.gamecake.sounds")
local wfonts  =require("wetgenes.gamecake.fonts")

local grd=require("wetgenes.grd")


function bake(opts)

	local cake={}
	
	opts.cake=cake -- insert cake into opts
	cake.opts=opts -- and opts into cake


--	get gl and win from state if we need them
	opts.gl=opts.gl or (opts.state and opts.state.gl)
	opts.win=opts.win or (opts.state and opts.state.win)

	if opts.win.flavour=="nacl" then -- nacl hacks
--		cake.opts.disable_sounds=true
	end

	if opts.win.flavour=="android" then -- android hacks
--		cake.opts.disable_sounds=true
	end

	if opts.win.flavour=="windows" then -- windows hacks
		cake.opts.disable_sounds=true
	end

	if opts.win.flavour=="raspi" then -- raspi hacks
		cake.opts.disable_sounds=true
	end

-- cache stuffs	
	cake.state=opts.state
	cake.gl=opts.gl
	cake.win=opts.win
	
	cake.mods={} -- for use in our baked require
	
	
	if cake.gl then cake.gl.GetExtensions() end
	
		
cake.setup = function()
print("CAKE SETUP")
	cake.sounds.setup()
end

cake.clean = function()
	cake.sounds.clean()
end

cake.start = function()
print("cakestart")
	cake.buffers.start()
	cake.images.start()
	cake.sheets.start()
	cake.fonts.start()
	cake.sounds.start()
end

cake.stop = function()
print("cakestop")
	cake.fonts.stop()
	cake.sheets.stop()
	cake.images.stop()
	cake.buffers.stop()
	cake.sounds.stop()
	if cake.gl.forget then -- any programs will need to be recompiled
		cake.gl.forget()
	end
end

	
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
	

-- and add in some extra simple sugar

	local images=cake.images
	local canvas=cake.canvas
	local sounds=cake.sounds

-- simple wrapper for bliting by id/name
cake.blit = function(id,cx,cy,ix,iy,w,h)
	canvas.blit(images.get(id),cx,cy,ix,iy,w,h)
end

-- simple wrapper for beeping by id/name
cake.beep = function(id)
	sounds.beep(sounds.get(id))
end

	cake.setup()
	
	return cake
end

