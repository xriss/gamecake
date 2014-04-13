--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")

local linux={}

local core=require("wetgenes.win.linux.core")


linux.msg=function(w)
	local m=core.msg(w)
	return m
end

linux.send_intent=function(s)
	local e=wstr.url_encode(s)
	os.execute("xdg-open \"https://twitter.com/intent/tweet?text="..e.."\"")
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
