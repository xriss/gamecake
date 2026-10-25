--
-- (C) 2016 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local sin=math.sin
local pi=math.pi

local kissfft=require("wetgenes.kissfft.core")
local wpack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth=M


bitsynth.frequency=48000
bitsynth.oversample=1
bitsynth.samplerate=bitsynth.frequency*bitsynth.oversample-- number of samples per second we will deal with

--                     01   02   03   04   05   06   07   08   09   10   11   12
bitsynth.note_names={ "C", "C#","D", "D#","E", "F", "F#","G", "G#","A", "A#","B"}
for i=0,12 do
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
bitsynth.midi2freq=function(m)
	return (2^((m−69)/12))*440
end

-- convert a freq to a midi note
bitsynth.freq2midi=function(f)
	return 69+( 12 * ( math.log(f/440) / log_of_2 ) )
end

-- convert a note string, eg C4 into a midi note
bitsynth.note2midi=function(s,o)

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
	if not o then 0=4 end -- default octave
	assert(type(o)=="number") -- unknown octave name

	local n=bitsynth.note_names[ s ]
	assert(type(n)=="number") -- unknown note name

	return (n-1)+((o+1)*12) -- return midi note
end

bitsynth.note2freq=function(s,o)
	local m=bitsynth.note2midi(s,o)
	return bitsynth.midi2freq(m)
end

-- also add quick lookups of notes to bitsynth globals

--[[
bitsynth.fillnotes=function(t)
	for n=1,12 do
		for o=1,8 do
			local s1=bitsynth.note_names[n]..o
			local s2=s1:gsub("#","s")
			local s3=s1:gsub("#",""):lower()
			local f=bitsynth.note2freq(s1)
			t[s1]=f -- the default -> C#4
			t[s2]=f -- also use s for sharp as # is an operator -> Cs4
			if s1~=s2 then -- this is a sharp
				t[s3]=f -- lowercase means sharp -> c4
			end
		end
	end
end

bitsynth.fillnotes(bitsynth) -- we can now use bitsynth.C4 or bitsynth.Cs4 or bitsynth.c4 which is easier than bitsynth["C#4"]
]]

-- create a rnd function
bitsynth.randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1
		n=(n*75)%65537
		return (n-1)/65536
	end
end

bitsynth.rnds={}
bitsynth.rnd=bitsynth.randomise(437) -- a global random sequence
for i=1,65536 do -- cache 64k set sequence
	bitsynth.rnds=bitsynth.rnd()
end

-- repeating wave functions
-- put in a value 0-1 and get out a single wave cycle that goes 0 1 0 -1 0 ish depending on wave
bitsynth.fwav={}

-- wave functions that begin with f, return a wave function built with args
bitsynth.fwav.f=function(f) -- auto lookup f if it is not already a function
	if type(f)=="string" then f=bitsynth.fwav[f] end -- lookup function
	if type(f)=="table" then f=bitsynth.fwav[f[1]](select(2,unpack(f))) end -- call wrapper
	return f
end

bitsynth.fwav.frez=function(rez,f) -- reduce resolution of f function to 2*rez+1 values
	f=bitsynth.fwav.f(f)
	return function(t)
		return math.floor( (f(t)*rez) + 0.5 )/rez
	end
end

bitsynth.fwav.fscale=function(scale,ff) -- scale wave
	local f=bitsynth.fwav.f(ff)
	return function(t)
		return f(t)*scale
	end
end

bitsynth.fwav.fadd=function(...) -- add all given waveforms
	local fs={}
	for i,v in ipairs({...}) do
		fs[i]=bitsynth.fwav.f(v)
	end
	return function(t)
		local n=0
		for i=1,#fs do
			n=n+(fs[i])(t)
		end
		return n
	end
end

bitsynth.fwav.fmul=function(...) -- multiply all given waveforms together
	local fs={}
	for i,v in ipairs({...}) do
		fs[i]=bitsynth.fwav.f(v)
	end
	return function(t)
		local n=1
		for i=1,#fs do
			n=n*(fs[i])(t)
		end
		return n
	end
end

bitsynth.fwav.zero=function(t)
	return 0
end

bitsynth.fwav.one=function(t)
	return 1
end

bitsynth.fwav.minus=function(t)
	return -1
end

--[[

	   ***
	 **   **
	*       *
	*       *       *
			*       *
			 **   **
			   ***

]]
bitsynth.fwav.sine=function(t)
	return sin( (t%1) * pi*2  )
end

--[[
	    *
	   * *
	  *   * 
	 *     * 
	*       *       *
			 *     *
			  *   *
			   * *
	            *
]]
bitsynth.fwav.triangle=function(t)
	t=t%1
	if     t<0.25 then return t*4
	elseif t<0.50 then return 1-((t-0.25)*4)
	elseif t<0.75 then return (t-0.5)*-4
	else               return ((t-0.75)*4)-1
	end
end

--[[

		  ***
		**  *
	  **    *
	**      *      **
			*    **  
			*  **    
			***

]]
bitsynth.fwav.sawtooth=function(t)
	return (((t+0.5)%1)*2)-1
end

--[[

	***
	*  **
	*    ** 
	*      ***      *
			  **    * 
				**  * 
				  ***

]]
bitsynth.fwav.toothsaw=function(t)
	return ((t%1)*-2)+1
end

--[[

	*********
	*       *
	*       *
	*       *       *
			*       *
			*       *
			*********

]]
bitsynth.fwav.square=function(t)
	if (t%1)>=0.5 then return -1 end 
	return 1
end

--[[
		  *     *
	  *   *     *
	  *  *** *  ** *
	* *  * ***  ** * *
	 * ***  * **  * *
	   *    * **  *
			* *
]]
bitsynth.fwav.fwhitenoise=function(steps,seed) -- steps is the number of values in a white noise wave cycle
	return function(t)
		local idx=1+(math.floor((t*steps)+seed)%65536)
		return bitsynth.rnds[idx]
	end
end
bitsynth.fwav.whitenoise=bitsynth.fwav.fwhitenoise(16,437) -- default whitenoise

-- create a wave duty function, this squishes a waves time
-- numbers are mid point and can be dragged left to 0 or right to 1
-- first number is middle of wave ( main duty )
-- second (optional) number is middle of left part of wave
-- second (optional) number is middle of right part of wave
bitsynth.fwav.fduty=function(b,a,c)
	-- defaults to middle of each section, half on half off
	if not b then b=0.5 end
	if not a then a=0.5 end
	if not c then c=0.5 end
	-- convert into wave time
	local ta=b*a
	local tb=b
	local tc=1-((1-b)*c)
	-- clamp
	if ta<0 then ta=0 end ; if ta>1 then ta=1 end
	if tb<0 then tb=0 end ; if tb>1 then tb=1 end
	if tc<0 then tc=0 end ; if tc>1 then tc=1 end

	local f=function(t)
		t=t%1
		if t==0 then return 0 end -- catch 0 and 1 prevents possible divide by 0
		if     t<=ta then -- ta cant be 0
			return (((t)/(ta))*0.25)
		elseif t<=tb then -- tb cant be ta
			return (((t-ta)/(tb-ta))*0.25)+0.25
		elseif t<=tc then -- tc cant be tb
			return (((t-tb)/(tc-tb))*0.25)+0.5
		elseif t<1  then -- tc cant be 1
			return (((t-tc)/(1 -tc))*0.25)+0.75
		end
		return 1 -- catch 1
	end
	return f
end

-- create an envelope function, could be used for waves as well if you wish but is rather slow with lots of points
-- pass in a {t1,v1,t2,v2,t3,v3,...} table of points and we will linear interpolate between the values
-- must be at least one point of data and each time value must be higher than the previous
-- returns function of the following type -> value=function(time)
bitsynth.fwav.flinear=function(...)
	local tt={...}
	local idx_max=#tt -- remember length
	local tmin,vmin=tt[1],tt[2]
	local tmax,vmax=tt[idx_max-1],tt[idx_max]
	local f=function(t)
		if t<=tmin then return vmin end
		if t>=tmax then return vmax end
		for i=3,idx_max-1,2 do -- simple search
			local t2=tt[i]
			local t1=tt[i-2]
			if t1<=t and t2>=t then -- found it
				local v2=tt[i+1]
				local v1=tt[i-1]
				if t1==t2 then return v1 end -- avoid /0
				local s=(t-t1)/(t2-t1) -- 0==v1 1==v2
				local v=v1+((v2-v1)*s) -- linear blend
				return v
			end
		end
		return 0
	end
	
	return f
end


-- sv (value)   is sustain value ( 0 to 1 )
-- at (attack)  is time to reach 1
-- dt (decay)   is time to reach sv
-- st (sustain) is time to stay at sv
-- rt (release) is time to reach 0
-- we feed these values into flinear and return the function,length
bitsynth.fadsr=function(sv,at,dt,st,rt)
	if type(sv)=="table" then sv,at,dt,st,rt=unpack(sv) end -- maybe unpack inputs from table
	local t={}
	if at~=0 then						t[#t+1]=0           t[#t+1]=0      -- start at 0
										t[#t+1]=at          t[#t+1]=1      -- attack
	elseif dt~=0 then					t[#t+1]=0           t[#t+1]=1	   -- start at 1
	elseif st==0 then					t[#t+1]=at+dt+st    t[#t+1]=sv end -- sustain level

	if st~=0 then						t[#t+1]=at+dt       t[#t+1]=sv     -- sustain start
										t[#t+1]=at+dt+st    t[#t+1]=sv end -- sustain end
						
	if rt~=0 then						t[#t+1]=at+dt+st+rt t[#t+1]=0  end -- release

	return bitsynth.flinear( unpack(t) ),at+dt+st+rt
end
bitsynth.fwav.fadsr=bitsynth.fadsr -- expose to wave generator


bitsynth.funcs={} -- reference by string and args


-- a wave generator, wraps a wave functions and handles wavelengths and phase sync with dynamic frequency change.
bitsynth.gwav=function(ot)

	local it={}
	
	it.fwav=bitsynth.fwav.f(ot.fwav or "sine") -- auto lookup function from string
	
	it.reset=function()
		it.sample=0 -- last sample index ( it.sample/bitsynth.samplerate == time in seconds ), this is expected to only ever increase
		it.frequency=0
		it.wavelength=1
		it.phase=0
	end
	it.reset()

	if ot.duty then -- duty adjust
		if type(ot.duty)=="table" then
			it.duty=bitsynth.duty(unpack(ot.duty))
		else
			it.duty=bitsynth.duty(ot.duty)
		end
	end

-- adjust the note frequency but keep the wave at a stable point in its cycle.
	it.set_note=function(note)
		local old_phase=((it.sample/it.wavelength)+it.phase)%1 -- this is the current phase position, we want to keep it here when we change frequency
		it.note=note -- midi number, probably an int but may not be
		it.frequency=bitsynth.midi2freq(note)
		it.wavelength=bitsynth.samplerate/it.frequency
		local new_phase=(it.sample/it.wavelength)%1 -- this is the unadjusted new base phase for the current sample point in time
		it.phase=(old_phase-new_phase)%1 -- this adjusts it to what it was before
		while it.phase<0 do it.phase=it.phase+1 end -- make sure phase is not a negative value
	end
	it.basenote=ot.note or 60
	it.set_note( it.basenote ) -- start at this midi note

-- read a sample, and advance the sample counter by one.
	it.read=function()
		local t=(it.sample/it.wavelength)+it.phase
		if it.duty then
			t=math.floor(t)+it.duty(t) -- adjust t by duty
		end
		local v=it.fwav(t)
		it.sample=it.sample+1
		return v
	end

	return it
end


-- prepare a generator to render from the options table (ot)
-- most basic sound, nothing clever just a waveform and a volume envelope with optional modulation
bitsynth.prepare=function(ot)
	local it={ot=ot}
	
-- return a bound custom function or generic one from within bitsynth.funcs
	local fbind=function(f)
		local t=type(f)
		local c=function(n,...)
			if not bitsynth.funcs[n] then return end -- did not find function
			return bitsynth.funcs[n](...) -- bind it
		end
		if     t=="function" then return f(it)
		elseif t=="table"    then return c(unpack(f))
		end
	end
	
	it.name=ot.name -- remember our name
	it.raw=ot.raw -- remember this raw table of options

	it.volume=ot.volume or 1 -- render volume
	
	it.gwav=bitsynth.gwav(ot) -- base wave

	if ot.fadsr then
		it.fadsr,it.time=fbind(ot.fadsr) -- custom function
	else 
		it.fadsr,it.time=bitsynth.fadsr(ot.adsr) -- or data table
	end
	
	it.samples=it.time*bitsynth.samplerate
	it.sample=0
	it.read=function()

		-- using frequency modulation?
		for i,v in ipairs(it) do -- update sub waves and ourselves
			if v.update then v.update(v) end
		end
		if it.update then
			it.update(it) -- main callback
		end

		local e=it.fadsr( it.sample/bitsynth.samplerate ) -- envelope
		local w=it.gwav.read() -- waveform

		it.sample=it.sample+1
		return e*w*it.volume
	end

	it.rewind=function() it.sample=0 end

	for i,v in ipairs(ot) do -- init sub waves
		it[i]=bitsynth.gwav(ot[i])
		it[i].idx=i
		it[i].parent=it
	end

	return it
end

-- render a sound into a data table, the length is controlled by the sound
-- m samples are combined and averaged, so you can set the sample rate higher and subsample it down here?
bitsynth.render=function(it)

	if not it.gwav then it=bitsynth.prepare(it) end -- auto prepare if no wave generator

	local m=bitsynth.oversample
	it.rewind()
	local t={}
	if m==1 then -- simple 1:1 samples
		for i=1,it.samples do t[i]=it.read() end
	else
		for i=1,it.samples/m do -- average m samples for each output value
			local v=0
			for j=1,m do v=v+it.read() end
			t[i]=v/m
		end
	end

-- apply eq to sample data
	if it.ot.eq and it.ot.eq[1] and it.ot.eq[2] then
		local oldlen=#t
		local feq=bitsynth.flinear(unpack(it.ot.eq))
		local tf={} ; for i=1,1025 do tf[i]=feq( (i-0.5)*48000/2048) end -- filter
		local ff,fl=wpack.save_array(tf,"f32")
		local tt={{},{}}
		local ti=1
		for i=0,#t-1,1024 do -- fft chunks
			local ta={}
			for j=1,2048 do ta[j]=t[i+j] or 0 end
			assert(kissfft.filter(2048,ta,ff))
			for j=1,2048 do tt[ti][i+j]=ta[j] end -- save into odd/even buffers
			ti=(ti%2)+1 -- toggle odd/even buffers
		end
		for i=1,#t do
			local ff=i/1024
			local fi=math.floor(ff)
			ff=ff-fi
			if (fi%2)==1 then ff=1-ff end
			if i<=1024 then ff=1 end-- special first chunk
			t[i]= (tt[1][i] or 0)*ff + (tt[2][i] or 0)*(1-ff) -- blend
		end
	end
	
	return t
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

lanes task function for handling bitsynth rendering.

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

