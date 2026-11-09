--
-- (C) 2026 Kriss@XIXs.com
--

local tau=math.pi*2

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local genes=M

local bitsynth=require("wetgenes.gamecake.fun.bitsynth")

-- smooth clip a float of any value to between -1 and +1
-- waves will be deformed progressively and never reach +1-1
-- uses n=n^2/(n^2+0.25) to squish
-- signal is untouched until 0.5 then gets squished
-- this will square off louder waves but is not as instantly harsh as clipping
genes.smooth_clip=function(n)
	if n<0 then
		if n>-0.5 then return n end -- linear
		return -n*n/(n*n+0.25) -- squish
	else
		if n<0.5 then return n end -- linear
		return  n*n/(n*n+0.25) -- squish
	end
end
local smooth_clip=genes.smooth_clip

-- blend between a and b by w clamped.
-- w is a wave so -1 is a and +1 is b
genes.blend=function(a,b,w)
	local f=(w*0.5)+0.5
	if f<0 then f=0 end
	if f>1 then f=1 end
	return a*(1-f) + b*f
end
local blend=genes.blend

-- mix between a and b by f clamped.
-- f is a fraction so 0 is a and 1 is b
genes.mix=function(a,b,f)
	if f<0 then f=0 end
	if f>1 then f=1 end
	return a*(1-f) + b*f
end
local mix=genes.mix

-- create a delay filter, tlen is size of delay buffer
-- fout is function that samples the buffer and produces an output
-- read is function that samples the buffer at given time
genes.gene_filter=function(tlen,fout)
	local buf={}
	local len=math.ceil(tlen*bitsynth.sample_rate)
	if len<1 then len=1 end
	for i=1,len do buf[i]=0 end -- empty
	local base=0
	-- single sample with tween
	local get=function(t) -- t is time from now into the past
		local f=t*bitsynth.sample_rate
		local i=math.floor(f)
		f=f-i
		if i<0 then i=0 end
		if i>(len-1) then i=(len-1) end
--		return buf[ 1+((base+i)%len) ]
		local j=i+1
		if j>(len-1) then j=(len-1) end
		local a=buf[ 1+((base+i)%len) ]
		local b=buf[ 1+((base+j)%len) ]
		return b*f + a*(1-f)
	end
	-- box sample
	local box=function(l,s) -- s box start, l length of box
		if not s then s=0 end
		local s=math.floor(s*bitsynth.sample_rate)
		local l=math.ceil(l*bitsynth.sample_rate)
		if s<0 then l=l+s s=0 end
		if s>(len-1) then s=(len-1) end
		if l<1 then l=1 end
		if l>(len-s) then l=len-s end
		local a=0
		for i=s,s+l-1 do
			a=a+buf[ 1+((base+i)%len) ]
		end
		return a/l
	end
	return function(v)
		if v then -- optional push into front of buffer
			base=(base+len-1)%len
			buf[1+base]=v
		end
		return fout(get,box) -- get output
	end
end
local gene_filter=genes.gene_filter

-- utility and wave/gene functions
-- gene functions create and return a wave function usually of the requested note
-- wave functions take (time,note) and return a "wave"
-- time must not be negative. it starts at 0 and advances.
-- if note is provided then the wave will be adjusted to this new length
-- whilst maintaining the current phase


-- create a wave duty function, this squishes a waves time
-- default values are all 0.5 and can be dragged left to 0 or right to 1
-- first number is middle of wave ( main duty )
-- second (optional) number is middle of left part of wave
-- third (optional) number is middle of right part of wave
genes.gene_duty=function(b,a,c,p)
	if not p then p=0 end -- adjust phase eg 0.25 makes sin become cos
	-- defaults to middle of each section, half on half off
	if not b then b=0.5 end
	if not a then a=0.5 end
	if not c then c=0.5 end
	local recalc=function()
		-- convert into wave time
		local ta=b*a
		local tb=b
		local tc=1-((1-b)*c)
		-- clamp
		if ta<0 then ta=0 end ; if ta>1 then ta=1 end
		if tb<0 then tb=0 end ; if tb>1 then tb=1 end
		if tc<0 then tc=0 end ; if tc>1 then tc=1 end
	end
	recalc()
	return function(t,na,nb,nc)
	
		-- adjust duty
		if na and na~=a then
			a=na
			if nb and nb~=b then b=nb end
			if nc and nc~=c then c=nb end
			recalc()
		elseif nb and nb~=b then
			b=nb
			if nc and nc~=c then c=nb end
			recalc()
		elseif nc and nc~=c then
			c=nb
			recalc()
		end

		t=t%1
		if t==0 then return 0 end -- catch 0 and 1 prevents possible divide by 0
		if     t<=ta then -- ta cant be 0
			return ((((t)/(ta))*0.25)+p)%1
		elseif t<=tb then -- tb cant be ta
			return ((((t-ta)/(tb-ta))*0.25)+0.25+p)%1
		elseif t<=tc then -- tc cant be tb
			return ((((t-tb)/(tc-tb))*0.25)+0.5+p)%1
		elseif t<1  then -- tc cant be 1
			return ((((t-tc)/(1 -tc))*0.25)+0.75+p)%1
		end
		return (1+p)%1 -- catch 1
	end
end
local gene_duty=genes.gene_duty

-- turn time into a ramp that increments at the given note
-- so 1hz would be unchanged, 2hz twice as fast etc
-- dynamic changes to the ramp frequency will maintain phase only
genes.gene_note=function(note)
	note=note or bitsynth.note_1hz
	local phase=0
	local freq=bitsynth.note_to_freq(note)
	return function(t,n)
		if n and n~=note then -- new note, lock phase at time t
			local new_freq=bitsynth.note_to_freq(n)
			local old_phase=(t*freq+phase)%1 -- current phase
			local new_phase=(t*new_freq)%1 -- new base phase
			phase=(1+old_phase-new_phase)%1 -- fix phase
			freq=new_freq -- new wavelength
			note=n  -- new note
		end
		t=t*freq+phase
		return t
	end
end
local gene_note=genes.gene_note

-- combine note and duty returning a 0-1 phase only
genes.gene_note_duty=function(note,gb,ga,gc)
	local ramp=gene_note(note)
	local duty=gene_duty(gb,ga,gc)
	return function(t,note,gb,ga,gc)
		return duty( ramp(t,note) , gb,ga,gc )
	end
end
local gene_note_duty=genes.gene_note_duty


-- at (attack)  is time to reach 1
genes.gene_attack=function(at)
	if at<=0 then return function() return 1 end end
	return function(t)
		if t<=0 then return 0 end
		if t<=at then return (t/at) end
		return 1
	end
end
local gene_attack=genes.gene_attack


-- st (sustain) is time to stay at 1
-- rt (release) is time to reach 0
-- times are lengths not absolutes
genes.gene_release=function(st,rt)
	return function(t)
		if t<=st then return 1 end
		if t<=st+rt then return 1-((t-st)/rt) end
		return 0
	end
end
local gene_release=genes.gene_release

-- dt is decay time to half amplitude, call with time to get a decay 
-- 0 or less removes decay
genes.gene_decay=function(dt)
	if dt<=0 then return function() return 1 end end
	return function(t)
		return 1 / (2^(t/(dt)))
	end
end
local gene_decay=genes.gene_decay

-- use env vars to build adsr and provide a sustain_off function
genes.env_adsr=function(env)
	env.total_length=env.sustain_length+env.release_length+env.filter_length
	local attack=gene_attack( env.attack_length )
	local decay=gene_decay( env.decay_length )
	local sustain_length=env.sustain_length
	local release=gene_release( sustain_length , env.release_length )
	return function(t) return attack(t) * decay(t) * release(t) end ,
	function(t)
		env.sustain_length=t or env.time
		env.total_length=env.sustain_length+env.release_length+env.filter_length
		if sustain_length~=env.sustain_length then
			sustain_length=env.sustain_length
			release=gene_release( sustain_length , env.release_length )
		end
	end
end
local env_adsr=genes.env_adsr


-- create a simple rnd function, no input just next in sequence out
genes.gene_randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1 inclusive
		n=(n*75)%65537
		return (n-1)/65535
	end
end
local gene_randomise=genes.gene_randomise


genes.gene_rez=function(rez) -- reduce resolution of input to 2*rez+1 values
	return function(t)
		return math.floor( (t*rez) + 0.5 )/rez
	end
end
local gene_rez=genes.gene_rez

genes.rnds={} -- cache
do
	local rnd=genes.gene_randomise(437)
	for i=1,65536 do -- cache 64k set sequence
		genes.rnds[i]=rnd()
	end
end


--[[

	    *        *
	   *  *    * *
	*  *  *   * * *
	*  *  *   *   *
	 *  * * * *   *
	 *    *  *     *
	      * 

 cache a big chunk of random and index it

]]
 -- steps is the number of values in a white noise wave cycle
genes.wave_whitenoise=function(t,steps,seed)
	local idx=1+(math.floor((t*(steps or 2))+(seed or 0))%65536)
	return (genes.rnds[idx]*2)-1
end
local wave_whitenoise=genes.wave_whitenoise

genes.gene_whitenoise=function(note,steps,seed)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_whitenoise( ramp(t,note) ,steps,seed )
	end
end

genes.gene_duty_whitenoise=function(note,steps,seed,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note,b,a,c)
		local r=ramp(t,note)
		return wave_whitenoise( math.floor(r) + duty( r , b,a,c) , steps,seed )
	end
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
genes.wave_sin=function(t)
	return math.sin( t*tau  )
end
local wave_sin=genes.wave_sin

genes.gene_sin=function(note)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_sin( ramp(t,note) )
	end
end

genes.gene_duty_sin=function(note,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note)
		return wave_sin( duty( ramp(t,note) , b,a,c) )
	end
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
genes.wave_triangle=function(t)
	t=t%1
	if     t<0.25 then return t*4
	elseif t<0.50 then return 1-((t-0.25)*4)
	elseif t<0.75 then return (t-0.5)*-4
	else               return ((t-0.75)*4)-1
	end
end
local wave_triangle=genes.wave_triangle

genes.gene_triangle=function(note)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_triangle( ramp(t,note) )
	end
end

genes.gene_duty_triangle=function(note,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note)
		return wave_triangle( duty( ramp(t,note) , b,a,c) )
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
genes.wave_sawtooth=function(t)
	return (((t+0.5)%1)*2)-1
end
local wave_sawtooth=genes.wave_sawtooth

genes.gene_sawtooth=function(note)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_sawtooth( ramp(t,note) )
	end
end

genes.gene_duty_sawtooth=function(note,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note)
		return wave_sawtooth( duty( ramp(t,note) , b,a,c) )
	end
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
genes.wave_toothsaw=function(t)
	return ((t%1)*-2)+1
end
local wave_toothsaw=genes.wave_toothsaw

genes.gene_toothsaw=function(note)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_toothsaw( ramp(t,note) )
	end
end

genes.gene_duty_toothsaw=function(note,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note)
		return wave_toothsaw( duty( ramp(t,note) , b,a,c) )
	end
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
genes.wave_square=function(t)
	if (t%1)>=0.5 then return -1 end 
	return 1
end
local wave_square=genes.wave_square

genes.gene_square=function(note)
	local ramp=gene_note(note)
	return function(t,note)
		return wave_square( ramp(t,note) )
	end
end

genes.gene_duty_square=function(note,b,a,c,p)
	local ramp=gene_note(note)
	local duty=gene_duty(b,a,c,p)
	return function(t,note)
		return wave_square( duty( ramp(t,note) , b,a,c) )
	end
end
