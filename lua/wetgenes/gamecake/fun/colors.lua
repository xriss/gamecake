--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wgrd =require("wetgenes.grd")
local wpack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,colors)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

colors.load=function()

	return colors
end

colors.setup=function()

	colors.load()

	return colors
end

colors.create=function(it,opts)
	it.opts=opts
	it.component="colors"
	it.name=opts.name or it.component
	
	it.cmap=opts.cmap or bitdown.cmap

--	it.drawtype=opts.drawtype or "first"
	it.layer=opts.layer or 0

	it.setup=function(opts)
		
		it.cmap_tex=gl.GenTexture()
		gl.BindTexture( gl.TEXTURE_2D , it.cmap_tex )	
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	gl.CLAMP_TO_EDGE)

		it.dirty(true)

	end

	it.clean=function()
		if it.cmap_tex then
			gl.DeleteTexture( it.cmap_tex )
			it.cmap_tex=nil
		end
	end

	it.update=function()
	end
	
	it.draw=function()

		if it.dirty() then
		
			it.dirty(false)

			gl.BindTexture( gl.TEXTURE_2D , it.cmap_tex )	
			gl.TexImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RGBA,
				256,
				1,
				0,
				gl.RGBA,
				gl.UNSIGNED_BYTE,
				it.cmap.grd.data ) -- it.cmap.grd contains a 256x1 color map texture

		end
		
	end

	it.dirty_flag=true
	it.dirty=function(flag)
		if type(flag)=="boolean" then it.dirty_flag=flag end
		return it.dirty_flag
	end

	return it
end

	return colors
end


