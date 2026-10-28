--
-- (C) 2016 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


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
bitsynth.note_to_freq=function(m)
	return (2^((m−69)/12))*440
end

-- convert a freq to a midi note
bitsynth.freq_to_note=function(f)
	return 69+( 12 * ( math.log(f/440) / log_of_2 ) )
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

-- wrap a gene into a cycle , pulling out 1 sample at a time
-- provides a way to smoothly stretch a waves length over time
-- and adjust the wave by changing gene parameters if needed
bitsynth.cycle=function(gene,vars)
	local it={}
	
	it.note=60 -- C4 default
	it.index=0
	-- length in samples , does not have to be an integer
	it.length=bitsynth.samplerate/bitsynth.note_to_freq(it.note)
	it.phase=0

	it.gene=genes.copy(gene) -- take a copy
	it.wave=genes.auto(it.gene)
	it.vars=vars

-- merge gene changes and change the wave
	it.regenerate=function(gene,vars)
		it.gene=genes.merge(it.gene,gene,vars or it.vars)
		it.wave=genes.auto(it.gene)
	end
	it.regenerate(gene,it.vars)

-- stretch the waves length but stay at the same point in its cycle, using phase.
	it.stretch=function(length)
		if length==it.length then return end
		local old_phase=((it.index/it.length)+it.phase)%1 -- this is the current phase position, we want to keep it here when we change frequency
		it.length=length
		local new_phase=(it.index/it.length)%1 -- this is the unadjusted new base phase for the current sample point in time
		it.phase=(old_phase-new_phase)%1 -- this adjusts phase to what it was before we changed length
		while it.phase<0 do it.phase=it.phase+1 end -- make sure phase is not a negative value
	end

-- returns the next sample and advances index by 1
	it.next=function()
		it.index=it.index+1
		return it.wave( (it.index/it.length)+it.phase )
	end

	return it
end

-- render a bunch of cycles, possibly feeding results between each using vars
-- should pass in it.genes and it.vars and it.length to configure
bitsynth.render=function(it)

	it=it or {}
	
	it.length=it.length or 48000
	
	it.vars=vars or {}
	if not it.vars.result then it.vars.result="result1" end
	it.genes=it.genes or {}
	it.cycles={}
	for i=1,#it.genes do
		it.cycles[i]=bitsynth.cycle(it.genes[i],it.vars)
	end
	
	it.next=function()
		for i=1,#it.genes do
			it.cycles[i].regenerate(it.genes[i],it.vars)
			local result="result"..i
			local note="note"..i
			while type(note)=="string" do
				note=it.vars[note]
			end
			if type(note)=="number" then
				-- add base note so we can have generic pitch bends
				note=note+it.cycles[i].note
				local length=bitsynth.samplerate/bitsynth.note_to_freq(note)
				it.cycles[i].stretch(length)
			end
			it.vars[result]=it.cycles[i].next() -- remember result
		end
		local result="result"
		while type(result)=="string" do -- pick result from result1 etc
			result=it.vars[result]
		end
		return result
	end

	return it
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
			
			local it=bitsynth.prepare(data.ot)
			local tt=bitsynth.render(it)
			local tab=bitsynth.float_to_16bit(tt)
			local dat=fats.table_to_int16s(tab)

			result.dat=dat -- return 16bit data string from bitsynth
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

