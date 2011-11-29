-- copy all globals into locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


module("fenestra.widget.data")






-- set dat from def table
local data_set=function(dat,def)

	if not def then return dat end -- short circuit
	
	if def.min then dat.min=def.min end
	if def.max then dat.max=def.max end
	if def.num then dat.num=def.num end
	if def.size then dat.size=def.size end
	if def.step then dat.step=def.step end
	
	return dat
end


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
	return dat.num.."/"..dat.max
end

-- how wide or tall should the handle be given the size of the parent?
local data_get_size=function(dat,w)
	if dat.size==0 then
		if dat.min==dat.max then return w end -- fullsize		
		return w/4 -- else some room to scroll
	else
		return w*( dat.size / dat.max-dat.min )
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

	local dat=dat or {}
--	dat.widget=it.widget
--	dat.it=it
--	dat.id=id
	dat.min=dat.min or 0
	dat.num=dat.num or 0
	dat.max=dat.max or 1
	dat.size=dat.size or 0 -- if 0 then button is auto sized to some value
	dat.step=dat.step or 0 -- if 0 then there is no quantization
	
	dat.get_string=data_get_string
	dat.get_size=data_get_size
	dat.set=data_set
	dat.value=data_value
	dat.snap=data_snap
	
	return dat
	
end


