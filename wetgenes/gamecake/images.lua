-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.images")

base=require(...)
meta={}
meta.__index=base

local wgrd=require("wetgenes.grd")



function create(opts)

	local images={}
	setmetatable(images,meta)
	
	images.cake=opts.cake
	
	images.grds={}
	
	
	images.prefix=opts.prefix or "art/out"
	images.postfix=opts.postfix or ".png"
	

	return images
end


--
-- load a single image, and make it easy to lookup by the given id
--
load=function(images,name,id)

	local fname=images.prefix..name..images.postfix
	
	local grd=assert(wgrd.create(fname,"png")):convert("U8_ARGB_PREMULT")

	images.grds[id]=grd

end

--
-- load many images from id=filename table
--
loads=function(images,tab)

	for i,v in pairs(tab) do
	
		if type(i)=="number" then -- use name twice
			images:load(v,v)
		else
			images:load(v,i)
		end
		
	end

end




