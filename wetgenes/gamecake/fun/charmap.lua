--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,charmap)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

charmap.load=function()

	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
print(filename)
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	return charmap
end

charmap.setup=function()

	charmap.load()

	return charmap
end

charmap.create=function(it,opts)
	it=it or {}
	it.opts=opts

	it.char_xh=it.opts.char_size and it.opts.char_size[1] or 8
	it.char_yh=it.opts.char_size and it.opts.char_size[2] or 8

	it.bitmap_xh=it.opts.bitmap_size and it.opts.bitmap_size[1] or 16
	it.bitmap_yh=it.opts.bitmap_size and it.opts.bitmap_size[2] or 16

	it.charmap_xh=it.opts.charmap_size and it.opts.charmap_size[1] or 256
	it.charmap_yh=it.opts.charmap_size and it.opts.charmap_size[2] or 256
	
	it.bitmap_grd  =wgrd.create("U8_RGBA", it.char_xh*it.bitmap_xh , it.char_yh*it.bitmap_yh , 1)
	it.charmap_grd =wgrd.create("U8_RGBA", it.charmap_xh           , it.charmap_yh           , 1)

	return it
end

	return charmap
end


