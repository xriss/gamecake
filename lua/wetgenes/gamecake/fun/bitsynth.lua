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

bitsynth.genes=require("wetgenes.gamecake.fun.bitsynth_genes")
local genes=bitsynth.genes

bitsynth.frequency=48000
bitsynth.oversample=1
bitsynth.samplerate=bitsynth.frequency*bitsynth.oversample-- number of samples per second we will deal with

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
	return (2^((m−69)/12))*(standard or 440)
end

-- convert a freq to a midi note
bitsynth.freq_to_note=function(f,standard)
	return 69+( 12 * ( math.log(f/(standard or 440)) / log_of_2 ) )
end

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


-- wrap a gene into a cycle , pulling out 1 sample at a time
-- provides a way to smoothly stretch a waves length over time
-- wave has its cycle passed in second param
-- so the wave functions can auto regen if it is dynamic
bitsynth.cycle=function(env,gene)
	local it={}
	
	it.idx=0 -- cycle array index
	
	it.sample=0 -- current sample we are rendering
	it.length=1024 -- length in samples , does not have to be an integer
	it.phase=0

	it.env=env -- shared env between waves
	it.gene=gene -- wave generator function or string to lookup in env

-- calling regenerate dynamically can change its own wave
	it.regenerate=function(t)
		local f=it.gene
		while type(f)=="string" do f=it.env[f] end -- auto string
		assert( type(f)=="function" ) -- wave must be function
		it.wave=f(t) or it.wave -- generate wave function, use old wave on nil
		assert( type(it.wave)=="function" ) -- wave must be function
	end
	it.regenerate(0) -- first gene is 0

-- stretch the waves length but stay at the same point in its cycle, using phase.
	it.stretch=function(length)
		if length==it.length then return end -- no change
		local old_phase=((it.sample/it.length)+it.phase)%1 -- this is the current phase position, we want to keep it here when we change frequency
		it.length=length
		local new_phase=(it.sample/it.length)%1 -- this is the unadjusted new base phase for the current sample point in time
		it.phase=(old_phase-new_phase)%1 -- this adjusts phase to what it was before we changed length
		while it.phase<0 do it.phase=it.phase+1 end -- make sure phase is not a negative value
	end

-- returns the next sample and advances sample by 1
	it.next=function()
		it.sample=it.sample+1
		return it.wave( (it.sample/it.length)+it.phase , it )
	end

	return it
end

-- dupe static env input and fill with junk 
-- render a bunch of cycles, defined in env
-- possibly feeding results between each
-- should pass in env.genes[1] env.genes[2] etc to init cycles
-- must pass in env.result for output
bitsynth.render_prepare=function(env)

	env=dupe(env)
	
	-- current time in seconds
	env.time = 0

	-- length of output in samples (how much will be statically rendered)
	env.length = env.length or 48000

	-- base note
	env.note   = env.note   or 60
	
	-- cache sample rate for easy access
	env.samplerate = bitsynth.samplerate

	-- load base gene functions into env
	for n,v in pairs(genes) do
		if type(v)=="function" then
			env[n]=env[n] or v
		end
	end
	
	-- env.lua is a string to init code with inside env
	-- code mis mildly sandboxed, may crash if it wants too
	-- but probably cant escape significantly
	if env.lua then
		sbox.load_and_run( env.lua , env )
	end
	
	env.cycles={}
	env.results={}
	env.notes={}
	
	-- start new cycles
	if env.genes then
		for i=1,#env.genes do
			env.cycles[i]=bitsynth.cycle(env,env.genes[i])
			env.cycles[i].idx=i
			env.results[i]=0 -- init results to 0
		end
	end
	
	return env
end

bitsynth.render_next=function(env)
	for i=1,#env.cycles do		
		-- possibly change wavelength using midi notes
		local note=env.notes[i]
		if note then
			while type(note)=="string" do note=env[note] end -- auto string
			if type(note)=="function" then note=note() end -- auto call
			if type(note)=="number" then
				env.cycles[i].stretch(
					bitsynth.samplerate/bitsynth.note_to_freq(note)
				)
			end
		end
		env.results[i]=env.cycles[i].next() -- remember result
	end

	-- get result
	local result="result"
	while type(result)=="string" do result=env[resuly] end -- auto string
	if type(result)=="function" then result=result() end -- auto call
	if type(result)=="number" then
		return result
	end
	return 0
end

bitsynth.render=function(env)
	-- render to array of numbers
	env.time=0
	local dat={}
	for i=1,env.length do
		env.time=i/bitsynth.samplerate -- time in seconds
		dat[i]=bitsynth.render_next(env)
	end
	return dat
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
bitsynth.task_code=function()
	
	local wire=require("wetgenes.wire")
	local bitsynth=require("wetgenes.gamecake.fun.bitsynth")
	local fats=require("wetgenes.fats")

	local consume=function(data)
		local result={}
		
		if data.action=="bitsynth" then
			
			local env=bitsynth.render_prepare(data.env)
			local floats=bitsynth.render(env)
			local ints=bitsynth.float_to_16bit(tt)
			local words=fats.table_to_int16s(ints)

			result.dat=words -- return 16bit data string from bitsynth
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

