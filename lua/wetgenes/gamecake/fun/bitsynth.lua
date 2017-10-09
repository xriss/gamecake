--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local sin=math.sin
local pi=math.pi

local kissfft=require("kissfft.core")
local wpack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth=M


bitsynth.frequency=48000
bitsynth.oversample=1
bitsynth.samplerate=bitsynth.frequency*bitsynth.oversample-- number of samples per second we will deal with

bitsynth.note_names={ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}
bitsynth.note_freq={
{ [4]=261.63 },
{ [4]=277.18 },
{ [4]=293.66 },
{ [4]=311.13 },
{ [4]=329.63 },
{ [4]=349.23 },
{ [4]=369.99 },
{ [4]=392.00 },
{ [4]=415.30 },
{ [4]=440.00 },
{ [4]=466.16 },
{ [4]=493.88 },
}
for i,v in ipairs(bitsynth.note_freq) do -- build other octaves
	v[1]=v[4]/8	v[2]=v[4]/4	v[3]=v[4]/2	v[5]=v[4]*2	v[6]=v[4]*4	v[7]=v[4]*8	v[8]=v[4]*16
end
bitsynth.note_freq_add={}
for i=1,11 do
	bitsynth.note_freq_add[i]={}
	for j=1,8 do
		bitsynth.note_freq_add[i][j]=bitsynth.note_freq[i+1][j] - bitsynth.note_freq[i][j]
	end
end
bitsynth.note_freq_add[12]={}
for j=1,7 do
	bitsynth.note_freq_add[12][j]=bitsynth.note_freq[1][j+1] - bitsynth.note_freq[12][j]
end
bitsynth.note_freq_add[12][8]=0

-- use the above tables to turn a frequency into the closest note name
bitsynth.freq2note=function(freq)
	for i=1,8 do
		if freq<=bitsynth.note_freq[12][i] + (bitsynth.note_freq_add[12][i]/2) then
			for j=12,2,-1 do
				if freq>bitsynth.note_freq[j-1][i] + (bitsynth.note_freq_add[j-1][i]/2) then
					return bitsynth.note_names[j]..i
				end
			end
			return sound.note_names[1]..i
		end
	end
end

-- use the above tables to turn a frequency into the closest note name
bitsynth.note2freq=function(s,n)
	if type(s)~="string" then return s end -- we only convert strings, otherwise return what we are given
	
	if not n then n=tonumber(s:sub(-1)) s=s:sub(1,-2) end -- allows "C",1 or "C1",nil

	s=s:upper() -- note names are uppercase
	for i=1,#bitsynth.note_names do
		if s==bitsynth.note_names[i] then s=i break end
	end
	assert(type(s)=="number") -- unknown note name
	return assert(bitsynth.note_freq[s][n]) -- check the range is ok as we return
end

-- get ratio of two notes, second note defaults to C4
bitsynth.note2ratio=function(s1,s2)
	local v1=bitsynth.note2freq(s1)
	local v2=bitsynth.note2freq(s2 or bitsynth.C4)
	return v1/v2
end

-- also add quick lookups of notes to bitsynth globals

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

-- create a rnd function
local randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1
		n=(n*75)%65537
		return (n-1)/65536
	end
end
bitsynth.rnd=randomise(437) -- a global random sequence

-- repeating wave functions
-- put in a value 0-1 and get out a single wave cycle that goes 0 1 0 -1 0 ish depending on wave
bitsynth.fwav={}

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
	local seeds={[0]=(seed or 437)} -- remember seed for *each* cycle
	local getseed=function(t)
		if not seeds[t] then -- need to work it out
			local fillfrom=0
			if seeds[t-1] then
				fillfrom=t-1 -- have previous seed
			else
				for j=t,0,-1 do -- find closest seed we have
					if seeds[j] then fillfrom=j break end
				end
			end
			for i=fillfrom,t do -- fill in missing seeds
				if not seeds[i] then
					local n=seeds[i-1]
					for j=1,steps do n=(n*75)%65537 end
					seeds[i]=n
				end
			end
		end
		return seeds[t]
	end
	return function(t)
		local n=getseed(math.floor(t))
		for j=0,math.floor((t%1)*steps) do n=(n*75)%65537 end
		return ((n-1)/32768)-1
	end
end
bitsynth.fwav.whitenoise=bitsynth.fwav.fwhitenoise(16,437) -- default whitenoise



-- attempt at pink noise... a tad expensive and not as versatile as white
--[[
bitsynth.fwav.fpinknoise=function(steps,layers,seed) -- layers is the number of white noise samples we merge
	local f=bitsynth.fwav.fwhitenoise(steps,seed)
	return function(t)
		local n=0
		for i=1,layers do n=n+f(t*math.pow(2,i-1)) end
		return n/layers
	end
end
bitsynth.fwav.pinknoise=bitsynth.fwav.fpinknoise(16,8,437)
]]

-- create an envelope function, could be used for waves as well if you wish but is rather slow with lots of points
-- pass in a {t1,v1,t2,v2,t3,v3,...} table of points and we will linear interpolate between the values
-- must be at least one point of data and each time value must be higher than the previous
-- returns function of the following type -> value=function(time)
bitsynth.flinear=function(...)
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
				local s=(t-t1)/(t2-t1) -- 0==v1 1==v2
				local v=v1+((v2-v1)*s) -- linear blend
				return v
			end
		end
	end
	
	return f
end
bitsynth.fwav.flinear=bitsynth.flinear -- expose to wave generator


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

bitsynth.funcs={} -- reference by string and args

-- turn a -1 +1 signal into a frequency signal -1==f1 0==f2 +1==f3 + t*t1
bitsynth.funcs.freq_range=function(f1,f2,f3,t1)
	if type(f1)=="table" then f1,f2,f3,t1=unpack(f1) end -- maybe unpack inputs from table
	f1=bitsynth.note2freq(f1)
	f2=bitsynth.note2freq(f2)
	f3=bitsynth.note2freq(f3)
	t1=t1 or 0
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+t*t1 end
		return f2+((f3-f2)*m)+t*t1
	end
	return f
end


-- a wave generator, wraps a wave functions and handles wavelengths and phase sync with dynamic frequency change.
bitsynth.gwav=function(ot)

	local it={}
	
	it.fwav=bitsynth.fwav.f(ot.fwav or "sine") -- auto lookup function from string
	
	it.sample=0 -- last sample index ( it.sample/bitsynth.samplerate == time in seconds ), this is expected to only ever increase
	it.duty=ot.duty or 0.5

-- adjust the frequency but keep the wave at a stable point.
	it.set_frequency=function(f)
		if f<0 then f=-f end
		if f==0 then
			it.frequency=0
			it.wavelength=1
			it.phase=0
		else
			local old_phase=((it.sample/it.wavelength)+it.phase)%1 -- this is the current phase position, we want to keep it here when we change frequency
			it.frequency=f
			it.wavelength=bitsynth.samplerate/it.frequency 
			local new_phase=(it.sample/it.wavelength)%1 -- this is the unadjusted new base phase for the current sample point in time
			it.phase=(old_phase-new_phase)%1 -- this adjusts it to what it was before
			while it.phase<0 do it.phase=it.phase+1 end -- make sure phase is not a negative value
		end
	end
	it.set_frequency(0) -- reset
	it.set_frequency( bitsynth.note2freq(ot.frequency) or bitsynth.C4 ) -- start at this frequency
-- read a sample, and advance the sample counter by one.
	it.read=function()
		local t=(it.sample/it.wavelength)+it.phase
		if it.duty~=0.5 then -- only if not default duty
			local ta=math.floor(t)
			local tb=(t-ta)
			if tb<it.duty then 		tb=      tb *(0.5/   it.duty)	-- i think we are /0 safe here...
			else					tb=1-((1-tb)*(0.5/(1-it.duty)))	end
			t=ta+tb
		end
		local v=it.fwav(t)
		it.sample=it.sample+1
		return v
	end

	return it
end


-- prepare a generator to render from the options table (ot)
-- most basic sound, nothing clever just a waveform and a volume envelope with optional FM
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
	
	-- we call a function to return a bound function or use a default empty one
	it.fread=fbind(ot.fread) or function(t) end

	it.gwav=bitsynth.gwav(ot) -- base wave

	if ot.fadsr then
		it.fadsr,it.time=fbind(ot.fadsr) -- custom function
	else 
		it.fadsr,it.time=bitsynth.fadsr(ot.adsr) -- or data table
	end
	
	if ot.fm then -- fm table flags fm actions

		it.fm_gwav=bitsynth.gwav(ot.fm) -- run another wave as fm input

		-- we call a function to return a bound function or use a default empty one
		it.fm_ffreq=fbind(ot.fm.ffreq) or function(v,s) return it.gwav.frequency end
		
	end


	it.samples=it.time*bitsynth.samplerate
	it.sample=0
	it.read=function()
		local t=it.sample/bitsynth.samplerate
		
		it.fread(t) -- main callback can change other settings by time

		-- using frequency modulation?
		if it.fm_gwav then
			local m=it.fm_gwav.read() -- get the modulation value
			it.gwav.set_frequency( it.fm_ffreq(m,t) ) -- function decides how it changes the frequency
		end

		local e=it.fadsr(t) -- envelope
		local w=it.gwav.read() -- waveform

		it.sample=it.sample+1
		return e*w*it.volume
	end

	it.rewind=function() it.sample=0 end

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


