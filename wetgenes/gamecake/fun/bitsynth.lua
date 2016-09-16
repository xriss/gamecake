--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local wpack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")
local wstr=require("wetgenes.string")

local sin=math.sin
local pi=math.pi

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

-- also add quick lookups of notes to bitsynth globals

bitsynth.fillnotes=function(t)
	for n=1,12 do
		for o=1,8 do
			local s=bitsynth.note_names[n]..o
			local f=bitsynth.note2freq(s)
			t[s]=f
			t[s:gsub("#","s")]=f -- also use s for sharp as # is an operator
		end
	end
end

bitsynth.fillnotes(bitsynth) -- we can now use bitsynth.C4 or bitsynth.Cs4 which is easier than bitsynth["C#4"]


-- repeating wave functions
-- put in a value 0-1 and get out a single wave cycle that goes 0 1 0 -1 0 ish depending on wave
bitsynth.fwav={}

bitsynth.fwav.sine=function(t)
	return sin( (t%1) * pi  )
end

bitsynth.fwav.square=function(t)
	if (t%1)>=0.5 then return -1 end 
	return 1
end

bitsynth.fwav.sawtooth=function(t)
	return (((t+0.75)%1)*-2)+1
end

bitsynth.fwav.whitenoise=function(t)
	return (math.random()-0.5)*2 -- probably need a better random function?
end

bitsynth.fwav.triangle=function(t)
	t=t%1
	if     t<0.25 then return t*4
	elseif t<0.50 then return 1-((t-0.25)*4)
	elseif t<0.75 then return (t-0.5)*-4
	else               return ((t-0.75)*4)-1
	end
end


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

-- sv (value)   is sustain value ( 0 to 1 )
-- at (attack)  is time to reach 1
-- dt (decay)   is time to reach sv
-- st (sustain) is time to stay at sv
-- rt (release) is time to reach 0
-- we feed these values into flinear and return the function,length
bitsynth.fadsr=function(sv,at,dt,st,rt)
	if type(sv)=="table" then sv,at,dt,st,rt=unpack(sv) end -- maybe unpack inputs from table
	local t={0,0}
	if at~=0 then t[#t+1]=at          t[#t+1]=1  end -- deal with 0 amounts of time
	if dt~=0 then t[#t+1]=at+dt       t[#t+1]=sv end
	if st~=0 then t[#t+1]=at+dt+st    t[#t+1]=sv end
	if rt~=0 then t[#t+1]=at+dt+st+rt t[#t+1]=0  end
	return bitsynth.flinear( unpack(t) ),at+dt+st+rt
end


-- turn a -1 +1 signal into a frequency signal -1==f1 0==f2 +1==f3
bitsynth.freq_range=function(f1,f2,f3)
	if type(f1)=="table" then f1,f2,f3=unpack(f1) end -- maybe unpack inputs from table
	f1=bitsynth.note2freq(f1)
	f2=bitsynth.note2freq(f2)
	f3=bitsynth.note2freq(f3)
	local f=function(b)
		if b<0 then return f2+((f1-f2)*-b) end
		return f2+((f3-f2)*b)
	end
	return f
end


-- a wave generator, wraps a wave functions and handles wavelengths and phase sync with dynamic frequency change.
bitsynth.gwav=function(ot)

	local it={}
	
	it.fwav=ot.fwav or "sine"
	if type(it.fwav)=="string" then it.fwav=bitsynth.fwav[it.fwav] end
	
	it.sample=0 -- last sample index ( it.sample/bitsynth.samplerate == time in seconds ), this is expected to only ever increase

	it.phase=ot.phase or 0 -- 0 to 1 starting phase of wave, this is auto adjusted as we change the frequency

	it.frequency=bitsynth.C4 -- default frequency of C4
	it.wavelength=bitsynth.samplerate/it.frequency -- and default wavelength

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
	it.set_frequency( bitsynth.note2freq(ot.frequency) or bitsynth.C4 ) -- start at this frequency

-- read a sample, and advance the sample counter by one.
	it.read=function()
		local v=it.fwav((it.sample/it.wavelength)+it.phase)
		it.sample=it.sample+1
		return v
	end

	return it
end


-- generic sound types with a few options
bitsynth.sound={}

-- most basic sound, nothing clever just a waveform and a volume envelope
bitsynth.sound.simple=function(ot)
	local it={}
	
	it.name=ot.name -- remember our name
	
	it.fread=ot.fread or function(it,t) end

	it.gwav=bitsynth.gwav(ot)

	it.fadsr,it.time=bitsynth.fadsr(ot.adsr)

	it.samples=it.time*bitsynth.samplerate
	it.sample=0
	it.read=function()
		local t=it.sample/bitsynth.samplerate

		it.fread(it,t) -- main callback can change settings by time
		
		local e=it.fadsr(t) -- envelope
		local w=it.gwav.read() -- waveform

		it.sample=it.sample+1
		return e*w
	end

	it.rewind=function() it.sample=0 end

	return it
end

-- a simple sound, with its frequency messed with
bitsynth.sound.simple_fm=function(ot)

	assert(type(ot.fm)=="table") -- this must be a table
	
	local it=bitsynth.sound.simple(ot) -- build on top of the simple code

	it.fm_gwav=bitsynth.gwav(ot.fm) -- another wave
	it.fm_frange=ot.fm.frange or function(v,s) return v end -- auto warble between given notes

	if ot.fm.range then it.fm_frange=bitsynth.freq_range(ot.fm.range) end

	it.read=function()
		local t=it.sample/bitsynth.samplerate
		
		it.fread(it,t) -- main callback can change settings by time

		local m=it.fm_gwav.read() -- get the modulation value
		it.gwav.set_frequency( it.fm_frange(m,t) ) -- apply it to the main frequency

		local e=it.fadsr(t) -- envelope
		local w=it.gwav.read() -- waveform

		it.sample=it.sample+1
		return e*w
	end
	
	return it
end


-- render a sound into a data table, the length is controlled by the sound
-- m samples are combined and averaged, so you can set the sample rate higher and subsample it down here?
bitsynth.render=function(it)
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
	return t
end


-- turn a stream of float values into s16 values
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



