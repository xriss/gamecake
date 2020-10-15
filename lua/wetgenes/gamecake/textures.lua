--
-- (C) 2020 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")

local wwin=require("wetgenes.win")
local wtongues=require("wetgenes.tongues")

-- slightly more generic texture handling than images which is just for images
-- maybe we should wrap the images interface over this more generic one

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,textures)
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
			
	textures.data={}

--[[

get a texture from an id and return it, if it is a table then it is 
assumed to be the texture and is returned.

]]
textures.get = function(id)
	local it
	if type(id)=="table" then
		it=id
	else
		it=textures.data[id]
	end
	return it
end

--[[

set a texture from an id and return it, it.id will be used if id is not 
explicitly provided.

]]
textures.set = function(it,id)
	id=id or it.id
	textures.data[id]=it
	return it
end



--[[

sometimes the OS may destroy all of the data on the opengl side
this function is called when we need to upload it all again

]]
textures.start = function()

	for n,t in pairs(textures.data) do
		textures.upload(n)
	end

end

--[[

sometimes the OS may destroy all of the data on the opengl side
this function is called when it is going to be deleted

]]
textures.stop = function()

	for n,t in pairs(textures.data) do
		textures.unload(n)
	end

end


--[[

 unload from opengl ( delete gl texture )

]]
textures.unload = function(id)
	local it=textures.get(id)

	if it and it.gl then
		gl.DeleteTexture( it.gl )
		it.gl=nil	
	end
end

--[[

 upload to opengl

]]
textures.upload = function(id)
	local it=textures.get(id)

	if not it.gl then
		it.gl=assert(gl.GenTexture())
	end
	
	textures.bind( it )
	
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,	it.TEXTURE_MIN_FILTER	or	gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,	it.TEXTURE_MAG_FILTER	or	gl.NEAREST)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,		it.TEXTURE_WRAP_S		or	gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,		it.TEXTURE_WRAP_T 		or	gl.CLAMP_TO_EDGE)

	local gl_data=it.gl_data -- maybe just here
	if it.get_gl_data then gl_data=it.get_gl_data(it) end -- maybe from a function

	assert( it.gl_data ) -- gonna need some data

	gl.TexImage2D(
		gl.TEXTURE_2D,
		0,
		it.gl_internal or gl.RGBA,
		it.gl_width,
		it.gl_height,
		0,
		it.gl_format or gl.RGBA,
		it.gl_type or gl.UNSIGNED_BYTE,
		gl_data )

	local err=gl.GetError()
	
	assert( err==0 ) -- well that went wrong

	it.gl_dirty=false

	return it
end


--[[

do a gl bind of this texture

]]
textures.bind = function(id)
	local it=textures.get(id)

	if it then
		if not it.gl or it.gl_dirty then -- may need to upload
			textures.upload(it)
		end
		gl.BindTexture( gl.TEXTURE_2D , it.gl or 0 )
	else
		gl.BindTexture( gl.TEXTURE_2D , 0 )
	end

end

--[[

create a new texture from initial values and return it

]]
textures.create = function(init)

	local it={}
	for n,v in pairs(init or {}) do it[n]=v end
	
	it.id = it.id or #textures.data+1 -- auto generate an id
	
	
	return it
end

	
	return textures
end


