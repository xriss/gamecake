--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")

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


linux.icon=function(w,g)

	assert(g:convert(wgrd.U8_RGBA)) -- make sure it is this format
	local argb=g:pixels(0,0,g.width,g.height)
	for i=0,g.width*g.height*4-1,4 do argb[i+1],argb[i+3]=argb[i+3],argb[i+1] end -- swap reg and blue
	core.rawicon(w,pack.save_array({g.width,g.height},"u32",0,2)..pack.save_array(argb,"u8",0,#argb))

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
