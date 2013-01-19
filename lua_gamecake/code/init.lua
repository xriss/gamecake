-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- gamecake lua is a simple 2D/3D game framework that targets GLESv2 on android/nacl/linux/windows/raspberrypi systems

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(state,cake)

	state.cake=cake
		
	cake.setup = function()
		cake.sounds.setup()
	end

	cake.clean = function()
		cake.sounds.clean()
	end

	cake.start = function()
		cake.buffers.start()
		cake.images.start()
		cake.sheets.start()
		cake.fonts.start()
		cake.sounds.start()
	end

	cake.stop = function()
		cake.fonts.stop()
		cake.sheets.stop()
		cake.images.stop()
		cake.buffers.stop()
		cake.sounds.stop()
		if cake.gl.forget then -- any programs will need to be recompiled
			cake.gl.forget()
		end
	end

	cake.update = function()
		cake.sounds.update()
	end


	cake.gles 	 = state.rebake("wetgenes.gamecake.gles") -- initalise gles and manage our shaders
	cake.buffers = state.rebake("wetgenes.gamecake.buffers") -- generic buffer memory is now complex thanks to retardroid
	cake.images  = state.rebake("wetgenes.gamecake.images") -- we will need to load some images
	cake.sheets  = state.rebake("wetgenes.gamecake.sheets") -- we will need to manage some sprite sheets
	cake.fonts   = state.rebake("wetgenes.gamecake.fonts") -- we will need to load some fonts
	cake.sounds  = state.rebake("wetgenes.gamecake.sounds") -- we will need to load some sounds
	cake.canvas  = state.rebake("wetgenes.gamecake.canvas") -- a canvas contains current drawing state and functions
	


-- finally add in some extra simple sugar

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

-- and things must be setup before we return
	cake.setup()
	
	return cake
end

