-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.buffers")
base=require(...)

local grd=require("wetgenes.grd")

function bake(opts)

	local buffers={}
	
	buffers.cake=opts.cake
	buffers.gl=opts.gl
	
	local gl=buffers.gl
	local cake=buffers.cake
	
	buffers.data={}

buffers.create = function(start,stop)

	local v={}
	
	v[0]=gl.GenBuffer()
	
	v.start=start	-- remember start / stop functions
	v.stop=stop -- probably dont need a stop function

	if v.start then
		v:start(buffers) -- and call start now
	end
	
	return v
end


buffers.start = function()
	for v,n in pairs(buffers.data) do
		v[0]=gl.GenBuffer()
		if v.start then
			v:start(buffers)
		end
	end
end

buffers.stop = function()
	for v,n in pairs(buffers.data) do
		gl.DeleteBuffer(v[0])
		v[0]=nil
		if v.stop then
			v:stop(buffers)
		end
	end

end


	return buffers
end




