--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdata)
wdata=wdata or {}


wdata.call_hook_later=function(dat,hook)
	if not dat.master then return wdata.call_hook(dat,hook) end -- can not defer without master
	local hooks=dat.hooks
	local type_hooks=type(hooks)
	if type_hooks=="function" then -- master function
		return dat.master.later_append( hooks , hook , dat )
	elseif type_hooks=="table" and hooks[hook] then -- or table of functions
		return dat.master.later_append( hooks[hook] , dat )
	end
end

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
wdata.data_value=function(dat,val,force)

	if dat.class=="number" or dat.class=="list" then
--print(dat,val,force)
		if val then
			if type(val)=="string" then val=dat:tonumber(val) end -- auto convert from string to number
			if val*0~=val*0 then val=0 end -- remove inf or nan values?
		end 
		local old=dat.num
		if dat.min and val and val<dat.min then val=dat.min end
		if dat.max and val and val>dat.max then val=dat.max end
		if ( val and val~=dat.num ) or force then -- change value
			dat.num=val or dat.num
		end
		if old~=dat.num or force then
			dat.str=dat:tostring(dat.num) -- cache on change
			if not force and type(force)=="boolean" then -- set force to false to disable hook
			else
				dat:call_hook_later("value") -- call value hook, which may choose to mod the num some more...
				if dat.master then dat.master.dirty_by_data(dat) end
			end
		end
		return dat.num
	else
		if not force and type(force)=="boolean" then -- set force to false to disable hook
			dat.str=val or dat.str
		else
			if (val and val~=dat.str ) or force  then -- change value
				dat.str=val or dat.str
				dat:call_hook_later("value") -- call value hook, which may choose to mod the num some more...
				if dat.master then dat.master.dirty_by_data(dat) end
			end
		end
		return dat.str
	end
end

-- adjust number (may trigger hook)
wdata.data_inc=function(dat,step)
	step=step or dat.step
	if step==0 then step=1 end
	dat:value(dat.num+step)
end
-- adjust number (may trigger hook)
wdata.data_dec=function(dat,step)
	step=step or dat.step
	if step==0 then step=1 end
	dat:value(dat.num-step)
end



-- get a string from the number
wdata.data_tostring=function(dat,num)
	return tostring(num or dat:value() )
end
wdata.data_tostring_from_list=function(dat,num)
	local d=dat.list[ num or dat:value() ]
	return d and d.str
end

-- get a number from the string
wdata.data_tonumber=function(dat,str)
	return tonumber(str or dat:value())
end
wdata.data_tonumber_from_list=function(dat,str)
	str=str or dat:value()
	for i,v in ipairs(dat.list) do
		if v.str==str then return i end
	end
	return nil
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
wdata.data_get_pos=function(dat,psiz,bsiz,reverse)
	if reverse then
		if dat.step==0 then -- no snap
			return (1-((dat.num-dat.min)/(dat.max-dat.min)))*(psiz-bsiz)
		else
			return (1-((dat.num-dat.min)/(dat.max-dat.min)))*(psiz-bsiz)
		end
	else
		if dat.step==0 then -- no snap
			return ((dat.num-dat.min)/(dat.max-dat.min))*(psiz-bsiz)
		else
			return ((dat.num-dat.min)/(dat.max-dat.min))*(psiz-bsiz)
		end
	end
end

-- given the parents size and our relative position/size within it
-- update dat.num and return a new position (for snapping)
wdata.data_snap=function(dat,psiz,bsiz,bpos,reverse)

--print("minmax",dat.min,dat.max)

	if dat.step==0 then -- no snap
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move

		local f=bpos/(psiz-bsiz)
		if reverse then f=1-f end
		dat:value(dat.min+((dat.max-dat.min)*f))
		
		return bpos
		
	else
	
		if dat.max==dat.min then dat:value(dat.min) return 0 end -- do not move
		
		local f=bpos/(psiz-bsiz)
		if reverse then f=1-f end
		local n=math.floor(0.5+(((dat.max-dat.min)*f)/dat.step))

		dat:value(dat.min+(n*dat.step))
		
		if reverse then
			return math.floor((psiz-bsiz)*(1-((dat.num-dat.min)/(dat.max-dat.min))))
		else
			return math.floor((psiz-bsiz)*((dat.num-dat.min)/(dat.max-dat.min)))
		end
	end
end


function wdata.new_data(dat)

	local dat=dat or {} -- probably use what is passed in only fill in more values

	dat.class=dat.class or "number" -- could also be a "string" or "list"

-- make default values and ranges for every possible class
-- this is very heavy data...

	dat.list=dat.list or {} -- list of possible values {{ num=1,str="hash1"},{ num=2,str="hash2"},...}
	
	dat.str_idx=dat.str_idx or 0
	dat.str_select=dat.str_select or 0

	dat.min=dat.min or 0 -- not negative by default
	dat.max=dat.max or (2^48) -- a big old number
	dat.size=dat.size or 0 -- if 0 then button is auto sized to some value
	dat.step=dat.step or 0 -- if 0 then there is no quantization
	
	if dat.class=="list" then -- build from .list
		dat.min=1
		dat.max=#dat.list
		dat.step=1
		dat.tostring=dat.tostring or wdata.data_tostring_from_list
		dat.tonumber=dat.tonumber or wdata.data_tonumber_from_list
	end

	
-- setup callback functions

	dat.call_hook=wdata.call_hook
	dat.call_hook_later=wdata.call_hook_later

	dat.tostring=dat.tostring or wdata.data_tostring -- convert number value to string (possible custom format)
	dat.tonumber=dat.tonumber or wdata.data_tonumber -- convert string value to number (possible custom format)

	dat.get_size=wdata.data_get_size
	dat.get_pos=wdata.data_get_pos
	
-- get or set the value
	dat.value=wdata.data_value
	dat.inc=wdata.data_inc
	dat.dec=wdata.data_dec

-- work out snapping for scroll bars	
	dat.snap=wdata.data_snap
	
	if dat.str and not dat.num then dat.num=dat:tonumber(dat.str) end
	if dat.num and not dat.str then dat.str=dat:tostring(dat.num) end

	dat.str=dat.str or ""
	dat.num=dat.num or 0

--	if dat.class=="number" then
--		dat:value(dat.num,true) -- triger value changed/set callbacks?
--	else
--		dat:value(dat.str,true) -- triger value changed/set callbacks
--	end
	
	return dat
	
end

return wdata
end

