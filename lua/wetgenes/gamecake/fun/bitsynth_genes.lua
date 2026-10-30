--
-- (C) 2026 Kriss@XIXs.com
--

local tau=math.pi*2

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local genes=M

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
		if t<=av then return (t/at) end
		if t<=av+dt then return sv+(((t-at)/dt)*(1-sv)) end
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

-- combine attack and release together to get a full adsr signal
genes.gene_adsr=function(sv,at,dt,st,rt)
	local a=genes.gene_attack(sv,at,dt)
	local r=genes.gene_release(st,rt)
	return function(t) return a(t)*b(t) end
end

-- create a simple rnd function, no input just next in sequence out
genes.gene_randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1
		n=(n*75)%65537
		return (n-1)/65536
	end
end


genes.gene_rez=function(rez) -- reduce resolution of input to 2*rez+1 values
	return function(t)
		return math.floor( (t*rez) + 0.5 )/rez
	end
end

genes.rnds={} -- cache
do
	local rnd=genes.randomise(437)
	for i=1,65536 do -- cache 64k set sequence
		genes.rnds[i]=rnd()
	end
end

-- cache a bit chunk of random and index it

genes.gene_whitenoise=function(steps,seed) -- steps is the number of values in a white noise wave cycle
	return function(t)
		local idx=1+(math.floor((t*steps)+seed)%65536)
		return genes.rnds[idx]
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
genes.wave_sine=function(t)
	return math.sin( (t%1) * tau  )
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


