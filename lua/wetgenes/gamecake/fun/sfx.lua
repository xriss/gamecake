--
-- (C) 2016 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- A copper like background, uses a glsl fragment program to fill the screen,

local wire=require("wetgenes.wire")

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

	local env={ -- bitsynth stream options table
		fifo_name="fun_bitsynth",
		lua=[=[

time=0
get_sample = function()

	local a=0

	for n,sound in pairs(sounds or {}) do
		a=a+(sound.sample or 0)
	end

	return a

end

]=],
	}

	local qq=cake.sounds.queues[1]
	qq:stream_bitsynth(env)

	local memo = wire.memo({
		fifo=wire.fifo("bitstream"),
		data={
			action="stream",
			env=env,
		},
	}):send()

	return sfx
end

sfx.create=function(it,opts)
	it.opts=opts
	it.component="sfx"
	it.name=opts.name or it.component

	it.bitsynth=bitsynth

	it.render=function(env) -- render a bitsynth sound
		local memo = wire.memo({
			fifo=wire.fifo("bitrender"),
			data={
				action="render",
				env=env, -- bitsynth options table
			},
		}):send()
		local result=function()
--PRINT("memo",memo.result)
			if memo.result then
				if memo.result.fail then
					PRINT(memo.result.fail)
					return "" --fail
				end
				return memo.result.buf
			end
		end
		return sounds.lazyload_wavtab( result, env.name , bitsynth.frequency )
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
	
	it.stream_sound=function(env)
		local memo = wire.memo({
			fifo=wire.fifo("bitstream"),
			data={
				action="sound",
				env=env,
			},
		}):send()
	end
	
	it.update=function() -- nothing to do, maybe add a tracker later?
	end
	
	it.draw=function() -- nothing to do
	end

	return it
end


	return sfx
end
