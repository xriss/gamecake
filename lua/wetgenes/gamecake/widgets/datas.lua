--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- handle widgets data values
--


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wdatas)
wdatas=wdatas or {}

function wdatas.new_datas(datas)

	local datas=datas or {} -- probably use what is passed in only fill in more values


print("DATA SETUP")	
	return datas
	
end

return wdatas
end

