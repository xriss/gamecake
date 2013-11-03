--
-- (C) 2013 Kriss@XIXs.com
--
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

