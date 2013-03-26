-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdata)
wdata=wdata or {}


wdata.call_hook=function(dat,hook)
	local hooks=dat.hooks
	local type_hooks=type(hooks)
	if type_hooks=="function" then -- master function
		return hooks(hook,dat)
	elseif type_hooks=="table" and hooks[hook] then -- or table of functions
		return hooks[hook](dat)
	end
end

-- set number (may trigger hook)
wdata.data_value=function(dat,val)
	if dat.class=="number" then
		if val then val=tonumber(val) end -- auto convert from string
		if val and val~=dat.num then -- change value
			dat.num=val
			if dat.min and dat.num<dat.min then dat.num=dat.min end
			if dat.max and dat.num>dat.max then dat.num=dat.max end
			dat.str=dat:get_string() -- cache on change
			dat:call_hook("value") -- call value hook, which may choose to mod the num some more...
		end
		return dat.num
	else
		if val and val~=dat.str then -- change value
			dat.str=val
			dat:call_hook("value") -- call value hook, which may choose to mod the num some more...
		end
		return dat.str
	end
end



-- a string to put in the handle
wdata.data_get_string=function(dat)
	if dat.class=="number" then return tostring(dat.num) end
	return dat.str
end

-- how wide or tall should the handle be given the size of the parent?
wdata.data_get_size=function(dat,w)
	local ret=16
	if dat.min==dat.max then
		ret=w					-- fullsize
	elseif dat.size==0 then
		ret=w/4					-- some random room to scroll
	else
		ret=w*dat.size			-- use the given size
	end
	if ret<16 then ret=16 end
	if ret>w then ret=w end
	return ret
end


-- get display pos, given the size of the parent and our size?
wdata.data_get_pos=function(dat,psiz,bsiz)
	if dat.step==0 then -- no snap
		return ((dat.num-dat.min)/(dat.max-dat.min))
	else
		return ((dat.num-dat.min)/(dat.max-dat.min))
	end
end

-- given the parents size and our relative position/size within it
-- update dat.num and return a new position (for snapping)
wdata.data_snap=function(dat,psiz,bsiz,bpos)

--print("minmax",dat.min,dat.max)

	if dat.step==0 then -- no snap
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move

		local f=bpos/(psiz-bsiz)
		dat:value(dat.min+((dat.max-dat.min)*f))
		
		return bpos
		
	else
	
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		local n=math.floor(0.5+(((dat.max-dat.min)*f)/dat.step))

		dat:value(dat.min+(n*dat.step))
		
		return math.floor((psiz-bsiz)*((dat.num-dat.min)/(dat.max-dat.min)))
	end
end


function wdata.new_data(dat)

	local dat=dat or {} -- probably use what is passed in only fill in more values
--	dat.widget=it.widget
--	dat.it=it
--	dat.id=id

	dat.class=dat.class or "number" -- could also be a "string"

-- make default values and ranges for every possible class
-- this is very heavy data...

	dat.lst=dat.lst or {}

	dat.str=dat.str or ""
	dat.str_idx=dat.str_idx or 0

	dat.num=dat.num or 0
	dat.min=dat.min or 0 -- not negative by default
	dat.max=dat.max or (2^48) -- a big old number
	dat.size=dat.size or 0 -- if 0 then button is auto sized to some value
	dat.step=dat.step or 0 -- if 0 then there is no quantization
	
	
-- setup callback functions

	dat.call_hook=wdata.call_hook

	dat.get_string=wdata.data_get_string -- should be moved into value() ?
	dat.get_size=wdata.data_get_size
	dat.get_pos=wdata.data_get_pos
	
-- get or set the value
	dat.value=wdata.data_value

-- work out snapping for scroll bars	
	dat.snap=wdata.data_snap

	dat:value(dat,dat.num) -- triger value changed/set callbacks
	
	return dat
	
end

return wdata
end

