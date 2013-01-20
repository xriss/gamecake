--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local linux={}

local core=require("wetgenes.win.linux.core")



-- key names are given in raw OS flavour,
-- this maps these raw names to more generic names
-- well actually I've decided to standardise on the windows vkey codes since
-- thats the most popular amongst all the platfroms suported (blame NaCl)
linux.generic_keymap={
	["grave"]="`",
--	["backspace"]="back",
}

function linux.keymap(key)
	key=key:lower()
	return linux.generic_keymap[key] or key
end


linux.msg=function(w)
	local m=core.msg(w)
	if m then
		if m.keyname then -- run it through our keymap probably just force it to lowercase.
			m.keyname=linux.keymap(m.keyname)
		end
	end
	return m
end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not linux[n] then -- only if not prewrapped
			linux[n]=v
		end
	end
end



return linux
