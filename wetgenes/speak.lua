--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wzips=require("wetgenes.zips")

local speak={}

local core=require("wetgenes.speak.core")

for n,f in pairs(core) do speak[n]=f end


speak.setup=function()

	speak.slt_data=assert(wzips.readfile("data/wvoices/cmu_us_slt/model_data.bin"))
	speak.slt_idxs=assert(wzips.readfile("data/wvoices/cmu_us_slt/model_data.idx"))

print("speak loaded ",#speak.slt_data,#speak.slt_idxs)

	core.setup(speak.slt_data,speak.slt_idxs)
end


-- safe to call setup now?
speak.setup()

return speak
