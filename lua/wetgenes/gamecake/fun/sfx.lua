--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- A copper like background, uses a glsl fragment program to fill the screen,

local wzips=require("wetgenes.zips")

local bitsynth=require("wetgenes.gamecake.fun.bitsynth")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,sfx)

	local cake=oven.cake
	local sounds=cake.sounds -- the gamecake sound interface


sfx.load=function()
	return sfx
end

sfx.setup=function()

	sfx.load()

	return sfx
end

sfx.create=function(it,opts)
	it.opts=opts
	it.component="sfx"
	it.name=opts.name or it.component

	it.bitsynth=bitsynth

	it.render=function(ot) -- render a bitsynth sound
		local t=bitsynth.render(ot)
		t=bitsynth.float_to_16bit(t)
		return sounds.load_wavtab( t, ot.name , bitsynth.frequency )
	end

	it.get=function(name)
		return assert(sounds.get(name))
	end

	it.play=function(name,pitch,gain) -- play a previously rendered sound, pitch and gain are scalars so default to 1
		local d=it.get(name)
		local op,og=d.pitch,d.gain
		d.pitch,d.gain=pitch or 1,gain or 1
		sounds.beep( d )
		d.pitch,d.gain=op,og
	end
	
	it.update=function() -- nothing to do, maybe add a tracker later?
	end
	
	it.draw=function() -- nothing to do
	end

	return it
end


	return sfx
end
