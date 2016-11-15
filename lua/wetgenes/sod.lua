--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local sod={}

local core=require("wetgenes.sod.core")

local meta={__index=sod}

sod.create=function()

	local sd={}
	setmetatable(sd,meta)
	
	sd[0]=assert( core.create() )
	
	core.info(sd[0],sd)
	return sd
end

sod.destroy=function(sd)
	core.destroy(sd[0])
end


sod.load=function(sd,name)
	assert(core.load_file(sd[0],name))
	core.info(sd[0],sd)
	return sd
end

sod.load_file=function(sd,name)
	assert(core.load_file(sd[0],name))
	core.info(sd[0],sd)
	return sd
end

sod.load_data=function(sd,data)
	assert(core.load_data(sd[0],data))
	core.info(sd[0],sd)
	return sd
end

sod.dynap_st16=core.dynap_st16

return sod
