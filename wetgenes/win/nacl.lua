--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local nacl={}

local core=require("wetgenes.win.nacl.core")

local wstr=require("wetgenes.string")

--
-- simple debug print function, we wrap the core so it accepts multiple 
-- args and behaves like luas print
--
nacl.print=function(...)
	local t={}
	for i,v in ipairs{...} do
		t[#t+1]=tostring(v)
	end
	core.print(table.concat(t,"\t"))
end
local print=nacl.print



nacl.context=core.context
nacl.swap=core.swap
nacl.time=core.time

nacl.call=core.call
nacl.getURL=core.getURL

nacl.info=function(win,t)
	t.width=640
	t.height=480
end



return nacl
