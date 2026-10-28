--
-- (C) 2026 Kriss@XIXs.com
--

local tau=math.pi*2


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local genes=M

-- genes is functions that create and return another function
-- returned functions take a signal (time) and return a signal (wave)
-- with a wavelength of 1

genes.auto=function(gene) -- auto lookup of function by name with optional params
	if type(gene)=="table" then -- with args
		local ff=genes[gene[1]]
		if ff then
			return ff(select(2,unpack(gene)))
		end
	else
		local ff=genes[gene]
		if ff then return ff() end
	end
	return gene
end

genes.copy=function(gene) -- merge into base
	if type(gene)=="table" then
		local ret={}
		for i=1,#gene do
			ret[i]=genes.copy(gene[i])
		end
		return ret
	else
		return gene
	end
end

genes.merge=function(base,change,vars) -- merge into base
	local tbase,tchange=type(base),type(change)
	if tchange == "nil"    then return base   end -- no change
	if tchange == "string" then -- check vars recursively
		if vars then
			v=change
			while type(v)=="string" do
				v=it.vars[v]
			end
			return v or change
		end
		return change
	end
	if tchange ~= "table"  then return change end -- single change
	if tbase   ~= "table"  then base={base}   end -- promote to table
	local i=0 ; repeat -- and merge tables
		i=i+1 
		base[i] = genes.merge( base[i] , change[i] , vars )
	until not base[i]
	return base
end

-- create a simple rnd function, no input just next in sequence out
genes.randomise=function(seed)
	local n=seed -- set the seed
	return function() -- get the next number as a value between 0 and 1
		n=(n*75)%65537
		return (n-1)/65536
	end
end


genes.rez=function(rez) -- reduce resolution of input to 2*rez+1 values
	return function(t)
		return math.floor( (t*rez) + 0.5 )/rez
	end
end

genes.scale=function(scale) -- scale input
	return function(t)
		return t*scale
	end
end

genes.offset=function(offset) -- offset input
	return function(t)
		return t+offset
	end
end

genes.chain=function(...) -- chain all functions
	local fs={}
	for i,v in ipairs({...}) do
		fs[i]=genes.auto(v)
	end
	return function(t)
		local n=t
		for i=1,#fs do
			n=(fs[i])(n)
		end
		return n
	end
end

genes.add=function(...) -- add all functions
	local fs={}
	for i,v in ipairs({...}) do
		fs[i]=genes.auto(v)
	end
	return function(t)
		local n=0
		for i=1,#fs do
			n=n+(fs[i])(t)
		end
		return n
	end
end

genes.mul=function(...) -- multiply all functions
	local fs={}
	for i,v in ipairs({...}) do
		fs[i]=genes.auto(v)
	end
	return function(t)
		local n=1
		for i=1,#fs do
			n=n*(fs[i])(t)
		end
		return n
	end
end



-- create a wave duty function, this squishes a waves time
-- numbers are mid point and can be dragged left to 0 or right to 1
-- first number is middle of wave ( main duty )
-- second (optional) number is middle of left part of wave
-- second (optional) number is middle of right part of wave
genes.duty=function(b,a,c)
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
genes.linear=function(...)
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
genes.adsr=function(sv,at,dt,st,rt)
	if type(sv)=="table" then sv,at,dt,st,rt=unpack(sv) end -- maybe unpack inputs from table
	local t={}
	if at~=0 then						t[#t+1]=0           t[#t+1]=0      -- start at 0
										t[#t+1]=at          t[#t+1]=1      -- attack
	elseif dt~=0 then					t[#t+1]=0           t[#t+1]=1	   -- start at 1
	elseif st==0 then					t[#t+1]=at+dt+st    t[#t+1]=sv end -- sustain level

	if st~=0 then						t[#t+1]=at+dt       t[#t+1]=sv     -- sustain start
										t[#t+1]=at+dt+st    t[#t+1]=sv end -- sustain end
						
	if rt~=0 then						t[#t+1]=at+dt+st+rt t[#t+1]=0  end -- release

	return genes.linear( unpack(t) ),at+dt+st+rt
end



genes.rnds={} -- cache
do
	local rnd=genes.randomise(437)
	for i=1,65536 do -- cache 64k set sequence
		genes.rnds[i]=rnd()
	end
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
genes.whitenoise=function(steps,seed) -- steps is the number of values in a white noise wave cycle
	return function(t)
		local idx=1+(math.floor((t*steps)+seed)%65536)
		return genes.rnds[idx]
	end
end


genes.zero=function()
	return function()
		return 0
	end
end

genes.one=function()
	return function()
		return 1
	end
end

genes.minus=function()
	return function()
		return -1
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
genes.sine=function()
	return function(t)
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
genes.triangle=function()
	return function(t)
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
genes.sawtooth=function()
	return function(t)
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
genes.toothsaw=function()
	return function(t)
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
genes.square=function()
	return function(t)
		if (t%1)>=0.5 then return -1 end 
		return 1
	end
end


