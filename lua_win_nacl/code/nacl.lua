--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local nacl={}

local core=require("wetgenes.nacl.core")
local wstr=require("wetgenes.string")


nacl.PP_INPUTEVENT_TYPE={
[-1]="UNDEFINED",
[0]="MOUSEDOWN",
"MOUSEUP",
"MOUSEMOVE",
"MOUSEENTER",
"MOUSELEAVE",
"WHEEL",
"RAWKEYDOWN",
"KEYDOWN",
"KEYUP",
"CHAR",
"CONTEXTMENU",
"IME_COMPOSITION_START",
"IME_COMPOSITION_UPDATE",
"IME_COMPOSITION_END",
"IME_TEXT",
} for i=0,#nacl.PP_INPUTEVENT_TYPE do nacl.PP_INPUTEVENT_TYPE[ nacl.PP_INPUTEVENT_TYPE[i] ]=i end

--
-- finally export any remaining core functions
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not nacl[n] then -- only if not prewrapped
			nacl[n]=v
		end
	end

end

return nacl

