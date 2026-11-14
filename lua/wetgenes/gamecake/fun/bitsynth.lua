--
-- (C) 2026 Kriss@XIXs.com
--

local sbox=require("wetgenes.sandbox")

local kissfft=require("wetgenes.kissfft.core")
local wpack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth=M
M.old=require("wetgenes.gamecake.fun.bitsynth_old") -- the bad old ways

bitsynth.sample_rate=48000
bitsynth.sample_time=1/bitsynth.sample_rate
bitsynth.packet_size=2048

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

bitsynth.env_init=function(...)

	-- a clean new env, copied from input
	local env={}
	for _,it in pairs{...} do dupe(it,env) end -- right replaces left
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
	env.sample_rate  = bitsynth.sample_rate
	env.sample_time  = bitsynth.sample_time
	env.packet_size  = bitsynth.packet_size
	env.note_1hz     = bitsynth.note_1hz
	env.note_to_freq = bitsynth.note_to_freq
	env.freq_to_note = bitsynth.freq_to_note

	-- current time in seconds, will be set by controlling code
	env.time = 0

	-- env.lua is a string to init code with inside env
	-- when called this must reset the sound so the env can be reused
	-- arguments may be passed in via uppercase vars in the env
	if env.lua then
		-- code is mildly sandboxed, may crash if it wants too
		-- but probably cant escape significantly
		sbox.load_and_run( env.lua , env )
	end
	
	return env
end


-- dupe static env input and fill with junk 
bitsynth.sound_init=function(...)

	local sound=bitsynth.env_init(...) -- dupe and basic init
	
	return sound
end

bitsynth.sound_render=function(sound)
	-- render to array of numbers with smooth_clip applied
	-- keep wave under 0.5 to prevent distortion
	local length=sound.total_length or 0
	local dat={}
PRINT("RENDER",sound.total_length)
	sound.time=0
	for i=1,math.ceil(length*sound.sample_rate) do
		-- get next sample ( we control time )
		dat[i]=sound.sample_next()
		sound.time=sound.time+sound.sample_time
	end
	return dat
end

bitsynth.stream_init=function(...)

	local stream=bitsynth.env_init(...) -- dupe and basic init
	
	return stream
end

bitsynth.tracker_init=function(...)

	local tracker=bitsynth.env_init(...) -- dupe and basic init

	return tracker
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


-- old code hacks
bitsynth.prepare=function(ot)
	local it={ot=ot}
DUMP(it)
	it.name=ot.name -- remember our name
	return ot
end
bitsynth.render=function(it)
	local floats={0,0,0,0}
	return floats
end
bitsynth.note2freq=function(n,o)
	local m=bitsynth.to_note(n,o)
	return bitsynth.note_to_freq(m)
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
			
			local sound=bitsynth.sound_init(unpack(data.envs))
			local floats=bitsynth.sound_render(sound)
			local ints=bitsynth.float_to_16bit(floats)
			local words=fats.table_to_int16s(ints)
			result.words=words -- return 16bit data string from bitsynth

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
				stream.time=wire.time()
			end

		elseif data.action=="sound"   then -- control sounds

			if not stream then return result end

			local sound
			if data.envs then -- new sound
				sound=bitsynth.sound_init(unpack(data.envs))
				sound=stream.play_sound(sound)
			end
			if sound then
				result.sound_id=sound.ID
			end

		end
	
		return result
	end
	
	-- advance the current stream and send data if it is time
	local update_stream=function()
	
		local update_time=wire.time()
		if stream.time <= update_time then -- send new packet

			local floats=stream.sample_packet()			
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
			local tpacket=math.floor(update_time*bitsynth.sample_rate)
			if tpacket>=stream.packet_size then
				PRINT("bitsynth took "..tpacket.." to make "..stream.packet_size)
			end
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

