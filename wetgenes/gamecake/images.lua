-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.images")

local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")



function bake(opts)

	local images={}
	
	images.cake=opts.cake
	images.gl=opts.gl
	
	local cake=images.cake
	local gl=images.gl
		
	images.data={}
	
	images.fmt=opts.cake.images_fmt
	images.zip=opts.zip
	images.prefix=opts.grdprefix or "data/"
	images.postfix=opts.grdpostfix or ".png"


images.get=function(id)
	return images.data[id]
end

images.set=function(d,id)
	images.data[id]=d
end


--
-- unload a previously loaded image
--
images.unload=function(id)
	local t=images.get(id)

	if t then
		if gl then --gl mode
				gl.DeleteTexture( t.id )
				t.id=nil	
		end
		images.set(nil,id)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
images.load=function(filename,id)
	local t=images.get(id)
	
	if t then return t end --first check it is not already loaded

print("loading",filename,id)

	local fname=images.prefix..filename..images.postfix
	local g=assert(grd.create())
	local d=assert(zips.readfile(fname))
	assert(g:load_data(d,"png"))
	
	if gl then --gl mode
	
		t={}
		images.upload_grd(t,g)

		images.set(t,id)
		
		t.filename=filename
		return t
	else
	
		assert(g:convert(images.fmt))
		
		images.set(g,id)

		g.filename=filename
		return g
	end
	
end

local function uptwopow(n)

	if n<1 then return 0
	elseif n<=16   then return 16
	elseif n<=32   then return 32
	elseif n<=64   then return 64
	elseif n<=128  then return 128
	elseif n<=256  then return 256
	elseif n<=512  then return 512
	elseif n<=1024 then return 1024
	elseif n<=2048 then return 2048
	elseif n<=4096 then return 4096
	end

	return 0
end

images.upload_grd= function(t,g)

	if not t then
		t={}
	end
	
	if not t.id then
		t.id=assert(gl.GenTexture())
	end
	
	t.x=0
	t.y=0
	t.width=g.width
	t.height=g.height
	
	if gl.extensions.ARB_texture_non_power_of_two then

		t.texture_width=g.width
		t.texture_height=g.height
		
	else -- need to place the image in a bigger texture and probably disable mipmaps.
	
		t.texture_width=uptwopow(g.width)
		t.texture_height=uptwopow(g.height)
		
	end
	
	gl.BindTexture( gl.TEXTURE_2D , t.id )
	
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE)

	if g.width==0 or g.height==0 then return t end -- no data to upload
	
--print(require("wetgenes.string").dump(g))
--	if g.format==grd.FMT_U8_RGBA or g.format==grd.FMT_U8_RGBA_PREMULT then
		-- ok to upload as is
--	else
		assert(g:convert(grd.FMT_U8_RGBA_PREMULT))
--	end
	

-- create a possibly bigger texture
	gl.TexImage2D(
		gl.TEXTURE_2D,
		0,
		gl.RGBA,
		t.texture_width,
		t.texture_height,
		0,
		gl.RGBA,
		gl.UNSIGNED_BYTE,
		nil)

-- and then copy the image data into it 		
	gl.TexSubImage2D(
		gl.TEXTURE_2D,
		0,
		0,
		0,
		g.width,
		g.height,
		gl.RGBA,
		gl.UNSIGNED_BYTE,
		g.data)

	return t
end

--
-- load many images from id=filename table
--
images.loads=function(tab)

	local function iorv(i,v) if type(i)=="number" then return v end return i end -- choose i or v

	for i,v in pairs(tab) do
		images.load(v,iorv(i,v))		
	end

end


images.start = function()

	for v,n in pairs(images.remember or {}) do
		images.load(v,n)
	end
	images.remember=nil
end

images.stop = function()

	images.remember={}
	
	for n,t in pairs(images.data) do
		images.remember[t.filename]=n	
		images.unload(n)
	end

end

-- do a gl bind of this texture
images.bind = function(t)
	gl.BindTexture( gl.TEXTURE_2D , t.id )
end
	
	return images
end


