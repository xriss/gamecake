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

-- blend between a and b by w clamped.
-- w is a wave so -1 is a and +1 is b
genes.blend=function(a,b,w)
	local f=(w*0.5)+0.5
	if f<0 then f=0 end
	if f>1 then f=1 end
	return a*(1-f) + b*f
end

-- mix between a and b by f clamped.
-- f is a fraction so 0 is a and 1 is b
genes.mix=function(a,b,f)
	if f<0 then f=0 end
	if f>1 then f=1 end
	return a*(1-f) + b*f
end

-- create a delay filter, tlen is size of delay buffer
-- fout is function that samples the buffer and produces an output
-- read is function that samples the buffer at given time
genes.gene_delay=function(tlen,fout)
	local buf={}
	local len=math.ceil(tlen*bitsynth.samplerate)
	if len<1 then len=1 end
	for i=1,len do buf[i]=0 end -- empty
	local base=0
	local get=function(t) -- t is time from now into the past
		local f=t*bitsynth.samplerate
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
	return function(v)
		base=(base+len-1)%len
		buf[1+base]=v
		return fout(get) -- get output
	end
end

-- utility and wave/gene functions
-- gene functions create and return a wave function
-- wave functions take (time) and return a wave (probably)
-- time must not be negative.
-- the wavelength is normally 1
-- time may be scaled to get the wave length we want out


-- create a wave duty function, this squishes a waves time
-- default values are all 0.5 and can be dragged left to 0 or right to 1
-- first number is middle of wave ( main duty )
-- second (optional) number is middle of left part of wave
-- second (optional) number is middle of right part of wave
genes.gene_duty=function(b,a,c)
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

-- sv (value)   is sustain value ( 0 to 1 )
-- at (attack)  is time to reach 1
-- dt (decay)   is time to reach sv
-- times are lengths not absolutes
genes.gene_attack=function(sv,at,dt)
	return function(t)
		if t<=0 then return 0 end
		if t<=at then return (t/at) end
		if t<=at+dt then return 1-(((t-at)/dt)*(1-sv)) end
		return sv
	end
end


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

-- use dynamic env value for release, as sustain must be changed by streaming
genes.env_release=function(env,sn,rn)
	sn=sn or "sustain_length"
	rn=rn or "release_length"
	return function(t)
		return (genes.gene_release(	env[sn] , env[rn] ))(t)
	end
end

-- combine attack and release together to get a full adsr signal
genes.gene_ladsr=function(sv,at,dt,st,rt)
	local a=genes.gene_attack(sv,at,dt)
	local b=genes.gene_release(st,rt)
	return function(t) return a(t)*b(t) end , st+rt
end

-- create a simple rnd function, no input just next in sequence out
genes.gene_randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1 inclusive
		n=(n*75)%65537
		return (n-1)/65535
	end
end


genes.gene_rez=function(rez) -- reduce resolution of input to 2*rez+1 values
	return function(t)
		return math.floor( (t*rez) + 0.5 )/rez
	end
end

genes.rnds={} -- cache
do
	local rnd=genes.gene_randomise(437)
	for i=1,65536 do -- cache 64k set sequence
		genes.rnds[i]=rnd()
	end
end

-- cache a bit chunk of random and index it

genes.gene_whitenoise=function(steps,seed) -- steps is the number of values in a white noise wave cycle
	steps=steps or 2 -- two gives a square wave cycle for the given frequency
	seed=seed or 0
	return function(t)
		local idx=1+(math.floor((t*steps)+seed)%65536)
		return (genes.rnds[idx]*2)-1
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
genes.gene_sine=function(s)
	s=s or 1
	return function(t)
		t=t*s
		return math.sin( (t%1) * tau  )
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
genes.gene_triangle=function(s)
	s=s or 1
	return function(t)
		t=t*s
		t=t%1
		if     t<0.25 then return t*4
		elseif t<0.50 then return 1-((t-0.25)*4)
		elseif t<0.75 then return (t-0.5)*-4
		else               return ((t-0.75)*4)-1
		end
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
genes.gene_sawtooth=function(s)
	s=s or 1
	return function(t)
		t=t*s
		return (((t+0.5)%1)*2)-1
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
genes.gene_toothsaw=function(s)
	s=s or 1
	return function(t)
		t=t*s
		return ((t%1)*-2)+1
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
genes.gene_square=function(s)
	s=s or 1
	return function(t)
		t=t*s
		if (t%1)>=0.5 then return -1 end 
		return 1
	end
end


