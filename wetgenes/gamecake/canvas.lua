-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require





module("wetgenes.gamecake.canvas")

base=require(...)
meta={}
meta.__index=base

local wgrd=require("wetgenes.grd")


function create(opts)

	local canvas={}
	setmetatable(canvas,meta)
	
	canvas.cake=opts.cake
	
	canvas.grd=assert(wgrd.create(canvas.cake.grd_fmt,
		opts.width or 320,
		opts.height or 240,
		1))

	return canvas
end

