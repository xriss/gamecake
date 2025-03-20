--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this contains mild emulation of the old gl fixed pipeline, such that we can run some code in gles2 and above

local tardis=require("wetgenes.tardis")
local wzips=require("wetgenes.zips")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,gles)

	if not oven.gl then -- need a gles2 wrapped in glescode
	
		oven.gl=require("glescode").create( require("gles") , gles )
		
		oven.gl.GetExtensions()
		oven.gl.probe_all()
		
		oven.gl.MatrixMode(oven.gl.PROJECTION)
		oven.gl.MatrixMode(oven.gl.MODELVIEW)

	end

	local gl=oven.gl
	
	local filename="lua/"..(M.modname):gsub("%.","/")..".glsl"
	gl.shader_sources( assert(wzips.readfile(filename),"file not found: "..filename) , filename )

	gl.camera=tardis.M4() -- no transform default camera for 2d
	gl.uniforms[ "camera"     ] = function(u) gl.UniformMatrix4f(u, gl.camera ) end
	gl.uniforms[ "modelview"  ] = function(u) gl.UniformMatrix4f(u, gl.matrix(gl.MODELVIEW) ) end
	gl.uniforms[ "projection" ] = function(u) gl.UniformMatrix4f(u, gl.matrix(gl.PROJECTION) ) end
	gl.uniforms[ "color"      ] = function(u) gl.Uniform4f(u, gl.cache.color ) end

	return oven.gl

end
