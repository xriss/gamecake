--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Main Good Luck Have Fun system virtual machine management.


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,system)

	system.charmap=oven.rebake("wetgenes.gamecake.fun.charmap")
	system.codemap=oven.rebake("wetgenes.gamecake.fun.codemap")
	system.screen =oven.rebake("wetgenes.gamecake.fun.screen")

	system.config={}
	system.components={}
	
	system.opts=oven.opts.fun or {} -- place your fun system setup in lua/init.lua -> opts.fun={}


system.setup=function()
end
system.clean=function()
end
system.msg=function(m)
end
system.update=function()
end
system.draw=function()
end

	return system
end
