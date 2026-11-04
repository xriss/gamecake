--
-- (C) 2016 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local sbox=require("wetgenes.sandbox")

local kissfft=require("wetgenes.kissfft.core")
local wpack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth=M
M.old=require("wetgenes.gamecake.fun.bitsynth_old") -- the bad old ways

bitsynth.samplerate=48000

bitsynth.genes=require("wetgenes.gamecake.fun.bitsynth_genes")
local genes=bitsynth.genes


--                     01   02   03   04   05   06   07   08   09   10   11   12
bitsynth.note_names={ "C", "C#","D", "D#","E", "F", "F#","G", "G#","A", "A#","B"}
for i=1,12 do
	local s=bitsynth.note_names[i]
	local c=s:sub(1,1)
	if #s==2 then -- is sharp
		bitsynth.note_names[ c:lower() ]=i -- lowercase
		bitsynth.note_names[ c.."#"    ]=i
		bitsynth.note_names[ c.."s"    ]=i
	else
		bitsynth.note_names[ c      ]=i -- uppercase
		bitsynth.note_names[ c.."-" ]=i
		bitsynth.note_names[ c.."_" ]=i
	end
end
-- cache log of 2 as a constant
local log_of_2=math.log(2)

-- midi notes to freq, based on 69 is A4 is 440 ( C4 is 60 )
bitsynth.note_to_freq=function(m,standard)
	return (2^((m-69)/12))*(standard or 440)
end

-- convert a freq to a midi note
bitsynth.freq_to_note=function(f,standard)
	return 69+( 12 * ( math.log(f/(standard or 440)) / log_of_2 ) )
end

bitsynth.note_1hz=bitsynth.freq_to_note(1)

-- convert a string, eg "C4" into a note
bitsynth.to_note=function(s,o)

	if type(s)~="string" then -- we only convert strings
		return s -- otherwise return number we are given
	end 

	if not o then -- look for octave
		o=tonumber(s:sub(-1))
		if o then
			s=s:sub(1,-2)
		else
			o=tonumber(s:sub(1,1))
			if o then
				s=s:sub(2)
			end
		end
	end
	if not o then o=4 end -- default octave
	assert(type(o)=="number") -- unknown octave name

	local n=bitsynth.note_names[ s ]
	assert(type(n)=="number") -- unknown note name

	return (n-1)+((o+1)*12) -- return midi note
end

bitsynth.to_freq=function(s,o)
	local m=bitsynth.to_note(s,o)
	return bitsynth.note_to_freq(m)
end

-- deep copy table a into b (or a new table) and return b
local dupe; dupe=function(a,b)
	b=b or {}
	for n,v in pairs(a) do
		if type(v)=="table" then	b[n]=dupe(v)
		else						b[n]=v
		end
	end
	return b
end
bitsynth.dupe=dupe


bitsynth.base_id=0
bitsynth.next_id=function()
	
	-- wrap ids, lets pretend it might happen
	if bitsynth.base_id >= 0xffffffffffff then bitsynth.base_id=0 end
	
	bitsynth.base_id=bitsynth.base_id+1

	return bitsynth.base_id
end

bitsynth.env_init=function(env)

	-- a clean new env, copied from input
	env=dupe(env)
	env.env=env -- self access from out of scope functions
	
	-- asign an ID
	env.ID=bitsynth.next_id()

	-- auto load base gene functions into env
	for n,v in pairs(genes) do
		if type(v)~="table" then -- do not copy tables
			env[n]=env[n] or v
		end
	end
	-- and copy some bitsynth basics 
	env.samplerate   = bitsynth.samplerate
	env.note_1hz     = bitsynth.note_1hz
	env.note_to_freq = bitsynth.note_to_freq
	env.freq_to_note = bitsynth.freq_to_note

	-- current time in seconds
	env.time = 0

	-- env.lua is a string to init code with inside env
	-- when called this must reset the sound so the env can be reused
	if env.lua then
		-- code is mildly sandboxed, may crash if it wants too
		-- but probably cant escape significantly
		sbox.load_and_run( env.lua , env )
	end
	
	return env
end


-- dupe static env input and fill with junk 
bitsynth.sound_init=function(env)

	env=bitsynth.env_init(env) -- dupe and basic init

	-- length of output in seconds
	env.sustain_length = env.sustain_length or 1	-- may be dynamic 
	env.release_length = env.release_length or 0	-- note off release time
	env.delay_length   = env.delay_length   or 0	-- optional delay filter
	-- if a sound is dynamically held then sustain_length will start as math.huge
	-- and change to the current time when released

	-- base note, dynamic input, for this sound
	env.note   = env.note   or 60
	
	return env
end


bitsynth.sound_next=function(env)

	-- get next sample
	env.time=env.time+(1/bitsynth.samplerate)
	env.sample=env.get_sample()
	return env.sample
end

bitsynth.sound_render=function(env)
	-- render to array of numbers with smooth_clip applied
	-- keep wave under 0.5 to prevent distortion
	env.time=0
	local length=env.sustain_length + env.release_length + env.delay_length
	local dat={}
	for i=1,math.ceil(length*bitsynth.samplerate) do
		-- get next sample and
		-- auto apply smooth_clip ( env may alter smooth_clip )
		dat[i]=env.smooth_clip( bitsynth.sound_next(env) )
	end
	return dat
end

bitsynth.line_init=function(env)

	env=bitsynth.env_init(env) -- dupe and basic init

	return env
end

bitsynth.line_next=function(env)

	return 0
end

bitsynth.stream_init=function(env)

	env=bitsynth.env_init(env) -- dupe and basic init
	
	-- stream buffer packet size
	env.packet_length=env.packet_length or 2048

	-- lists of active sounds etc keyed by ID
	env.sounds=env.sounds or {}
	env.lines=env.lines or {}

	return env
end

bitsynth.stream_next=function(env)

	-- advance playing sounds
	for n,sound in pairs(env.sounds or {}) do
		bitsynth.sound_next(sound)
		local length=sound.sustain_length + sound.release_length + sound.delay_length
		if sound.time > length then
			env.sounds[n]=nil -- finished
		end
	end

	-- get next sample
	env.time=env.time+(1/bitsynth.samplerate)
	env.sample=env.get_sample()
	return env.sample
end

bitsynth.tracker_init=function(env)

	env=bitsynth.env_init(env) -- dupe and basic init

	return env
end

bitsynth.tracker_next=function(env)

end


-- turn a stream of float values into s16 values, changes input data and returns it
-- scale defaults to 0x7fff but you could use a slightly lower value to reduce clipping
bitsynth.float_to_16bit=function(t,scale)
	scale=scale or 0x7fff
	local ht=#t
	for i=1,ht do
		local v=t[i]
		v=math.floor(v*scale)
		if v>0x7fff then v=0x7fff elseif v<-0x7fff then v=-0x7fff end
		t[i]=v
	end
	return t
end

-- turn a stream of float values into u8 values, changes input data and returns it
-- scale defaults to 0x7f but you could use a slightly lower value to reduce clipping
bitsynth.float_to_8bit=function(t,scale)
	scale=scale or 0x7f
	local ht=#t
	for i=1,ht do
		local v=t[i]
		v=math.floor(v*scale)
		if v>0x7f then v=0x7f elseif v<-0x7f then v=-0x7f end
		t[i]=v+0x80
	end
	return t
end


--[[#lua.wetgenes.gamecake.fun.bitsynth.task_code

task function for handling bitsynth rendering.

]]
bitsynth.render_code=function()
	
	local wire=require("wetgenes.wire")
	local bitsynth=require("wetgenes.gamecake.fun.bitsynth")
	local fats=require("wetgenes.fats")

	local consume=function(data)
		local result={}
		
		if data.action=="render" then
			
			local env=bitsynth.sound_init(data.env)
			local floats=bitsynth.sound_render(env)
			local ints=bitsynth.float_to_16bit(floats)
			local words=fats.table_to_int16s(ints)
			result.buf=words -- return 16bit data string from bitsynth

		end
	
		return result
	end

	 -- this named fifo will have been created before this thread
	local fifo = wire.fifo(wire_tasks_name)

	-- loop until our thread is asked to halt
	while wire.active( wire.thread_handle ) do
		local memo = fifo:pull()
		
		if memo then

			-- this will print a TRACEBACK on error but keep going
			local ok,result = xpcall( function() return consume( memo.data ) end , TRACEBACK )

			if ok then
				memo.result=result -- consume gave us a result to reply with
			else
				memo.result={ fail=(result or "fail") } -- reply with fail reason
			end
			memo:reply()
			memo:remove()

		else -- no memo, sleep a bit
		
			fifo:wait(1/16) -- slow response

		end

	end
	-- we have gracefully halted so cleanup and return

end


--[[#lua.wetgenes.gamecake.fun.bitsynth.stream_code

stream function for handling bitsynth streaming.

]]
bitsynth.stream_code=function()
	
	local wire=require("wetgenes.wire")
	local bitsynth=require("wetgenes.gamecake.fun.bitsynth")
	local fats=require("wetgenes.fats")
	
	local stream -- main stream data

	local consume=function(data)
		local result={}
		
		if     data.action=="tracker" then -- control tracker

		elseif data.action=="stream"  then -- control stream

			if data.env then
PRINT("START STREAM")
				stream=bitsynth.stream_init(data.env)
				stream.packet_time=wire.time()
			end

		elseif data.action=="line"    then -- control lines

		elseif data.action=="sound"   then -- control sounds

			if not stream then return result end

			local sound
			if data.env then -- new sound
				sound=bitsynth.sound_init(data.env)
			end
			if sound then
				stream.sounds[sound.ID]=sound
				result.sound_id=sound.ID
			end

		end
	
		return result
	end
	
	-- advance the current stream and send data if it is time
	local update_stream=function()
	
		local update_time=wire.time()
		if stream.packet_time <= update_time then -- send new packet

			local floats={}
			for i=1,stream.packet_length do
				floats[i]=bitsynth.stream_next(stream)
			end
			stream.packet_time=stream.packet_time+(stream.packet_length/bitsynth.samplerate)
			
			local ints=bitsynth.float_to_16bit(floats)
			local words=fats.table_to_int16s(ints)
			local memo = wire.memo({
				fifo=wire.fifo( stream.fifo_name ),
				data={
					action="stream",
					buf=words,
				},
			}):send()
			
			update_time=wire.time()-update_time
			local tpacket=math.floor(update_time*bitsynth.samplerate)
--			if tpacket>=stream.packet_length then
				PRINT("bitsynth took "..tpacket.." to make "..stream.packet_length)
--			end
		end
	end

	 -- this named fifo will have been created before this thread
	local fifo = wire.fifo(wire_tasks_name)

	-- loop until our thread is asked to halt
	while wire.active( wire.thread_handle ) do
		local memo = fifo:pull()
		
		if memo then

			-- this will print a TRACEBACK on error but keep going
			local ok,result = xpcall( function() return consume( memo.data ) end , TRACEBACK )

			if ok then
				memo.result=result -- consume gave us a result to reply with
			else
				memo.result={ fail=(result or "fail") } -- reply with fail reason
			end
			memo:reply()
			memo:remove()

		else -- no memo, sleep a bit
		
			fifo:wait(1/1024) -- fast response

		end
		
		if stream then
			xpcall( update_stream , TRACEBACK )
		end

	end
	-- we have gracefully halted so cleanup and return

end

