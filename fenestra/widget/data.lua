-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


module("fenestra.widget.data")


-- set number (may trigger hook)
local data_value=function(dat,num)
	if num and num~=dat.num then -- change value
		if num then dat.num=num end
		if dat.num<dat.min then dat.num=dat.min end
		if dat.num>dat.max then dat.num=dat.max end
--		dat.widget:call_hook("value",dat) -- call value hook, which may choose to mod the num some more...
	end
	return dat.num
end



-- a string to put in the handle
local data_get_string=function(dat)
	return math.floor(dat.num).."/"..math.floor(dat.max)
end

-- how wide or tall should the handle be given the size of the parent?
local data_get_size=function(dat,w)
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
local data_get_pos=function(dat,psiz,bsiz)
	if dat.step==0 then -- no snap
		return ((dat.num-dat.min)/(dat.max-dat.min))
	else
		return ((dat.num-dat.min)/(dat.max-dat.min))
	end
end

-- given the parents size and our relative position/size within it
-- update dat.num and return a new position (for snapping)
local data_snap=function(dat,psiz,bsiz,bpos)
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


function new_data(dat)

	local dat=dat or {} -- probably use what is passed in only fill in more values
--	dat.widget=it.widget
--	dat.it=it
--	dat.id=id

	dat.class=dat.class or "number"

-- make a default values and ranges for every possible class
-- this is heavy data...

	dat.lst=dat.lst or {}

	dat.str=dat.str or ""
	dat.str_idx=dat.str_idx or 0

	dat.num=dat.num or 0
	dat.min=dat.min or 0
	dat.max=dat.max or 1
	dat.size=dat.size or 0 -- if 0 then button is auto sized to some value
	dat.step=dat.step or 0 -- if 0 then there is no quantization
	
	
-- setup callback functions

	dat.get_string=data_get_string -- should be moved into value() ?
	dat.get_size=data_get_size
	dat.get_pos=data_get_pos
	
-- get or set the value
	dat.value=data_value

-- work out snapping for scroll bars	
	dat.snap=data_snap

	data_value(dat,dat.num) -- triger value changed/set callbacks
	
	return dat
	
end


