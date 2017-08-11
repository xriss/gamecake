--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local emcc={}

local core=require("wetgenes.win.emcc.core")
local sdl=require("wetgenes.win.sdl")
local wstr=require("wetgenes.string")

-- send message to js (nacl only)
emcc.js_post=function(m,d)
	if type(m)=="table" then
		local s={}
		for n,v in pairs(m) do
			if type(n)=="string" then
				table.insert(n.."="..wstr.url_encode(v)) -- need to escape values...
			end
		end
		s=table.concat(s,"&").."\n"
--		if m[0] then s=s..m[0] end -- and we have a large data chunk after message
		return core.js_post(s,m[0])
	end
	return core.js_post(m,d) -- just a string
end

emcc.show=function(it,view)
	return sdl.show(it,view)
end

--
-- export all core functions not wrapped above
--
for n,v in pairs(core) do -- check the core
	if type(v)=="function" then -- only functions
		if not emcc[n] then -- only if not prewrapped
			emcc[n]=v
		end
	end
end

--
-- export all sdl functions not wrapped above
--
for n,v in pairs(sdl) do -- check the core
	if type(v)=="function" then -- only functions
		if not emcc[n] then -- only if not prewrapped
			emcc[n]=v
		end
	end
end

return emcc
