-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,images)
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
		
	images.data={}
	
	images.prefix=opts.grdprefix or "data/"
	images.postfix=opts.grdpostfix or { ".png" , ".jpg" }


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

-- do we have an image availble to load?
images.exists=function(filename)

	local fname
	local fext

	if type(images.postfix)=="table" then -- try a few formats
		for i,v in ipairs(images.postfix) do
			fext=v
			fname=images.prefix..filename..fext -- hardcode
			if zips.exists(fname) then  return true end -- found it
		end
	else
		fext=images.postfix
		fname=images.prefix..filename..fext -- hardcode
	end
	
	if zips.exists(fname) then  return true end -- found it

end

--
-- load a single image, and make it easy to lookup by the given id
--
images.load=function(filename,id)
	local t=images.get(id)
	
	if t then return t end --first check it is not already loaded

--print("loading",filename,id)
oven.preloader(filename)

	local mname=images.prefix..filename..".lua"
	local fname
	local fext
	
	if type(images.postfix)=="table" then -- try a few formats
		for i,v in ipairs(images.postfix) do
			fext=v
			fname=images.prefix..filename..fext -- hardcode
			if zips.exists(fname) then  break end -- found it
		end
	else
		fext=images.postfix
		fname=images.prefix..filename..fext -- hardcode
	end
	
	local g=assert(grd.create())
	local d=assert(zips.readfile(fname),"Failed to load "..fname)
	assert(g:load_data(d,fext:sub(2))) -- skip extension period

-- clamp to a maximum size
	local max_size=1024
	if g.width > max_size or g.height>max_size then
		local hx=g.width>max_size and max_size or g.width
		local hy=g.height>max_size and max_size or g.height
print("Max texture size converting ",g.width,g.height," to ",hx,hy)
		g:scale( hx , hy ,1)
	end
	
--	g:scale(g.width/2,g.height/2,1)
	
	if gl then --gl mode
	
		t={}
		images.upload_grd(t,g)

		images.set(t,id)
		
		t.filename=filename
		
		local lson=zips.readfile(mname)
		if lson then -- check for lua metadata
			t.meta=wsbox.lson(lson) -- return it with the image
		end
		
		return t

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
images.uptwopow=uptwopow

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
	
--	gl.extensions.ARB_texture_non_power_of_two=false
--	if gl.extensions.ARB_texture_non_power_of_two then
--		t.texture_width=g.width
--		t.texture_height=g.height
--	else -- need to place the image in a bigger texture and probably disable mipmaps.
	
-- always need powah of two so we may mipmap
-- probably best to keep textures powah of two if at all possible as well?

		t.texture_width=uptwopow(g.width)
		t.texture_height=uptwopow(g.height)

--	end
	
	gl.BindTexture( gl.TEXTURE_2D , t.id )
	
--	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR)
--	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE)

--best mipmap?
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,t.TEXTURE_MIN_FILTER or images.TEXTURE_MIN_FILTER or gl.LINEAR_MIPMAP_LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,t.TEXTURE_MAG_FILTER or images.TEXTURE_MAG_FILTER or gl.LINEAR)


	if g.width==0 or g.height==0 then return t end -- no data to upload
	
--print(require("wetgenes.string").dump(g))
--	if g.format==grd.FMT_U8_RGBA or g.format==grd.FMT_U8_RGBA_PREMULT then
		-- ok to upload as is
--	else

		assert(g:convert(grd.FMT_U8_RGBA_PREMULT))

--	end

-- force powah of 2 sizes
if t.texture_width~=g.width or t.texture_height~=g.height then 
		g:resize(t.texture_width,t.texture_height,1) -- resize keeping the image in the topleft corner
end
	
--local zero=string.rep(string.char(0,0,0,0), t.texture_width * t.texture_height)

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
		g.data ) -- need to zero the texture?

-- create mipmaps
	local w=g.width
	local h=g.height
	local idx=0
	while w>=2 or h>=2 do
		idx=idx+1
		w=math.ceil(w/2)
		h=math.ceil(h/2)
		g:scale(w,h,1)
--print("mipmap",w,h)
		

-- create a possibly bigger texture
		gl.TexImage2D(
			gl.TEXTURE_2D,
			idx,
			gl.RGBA,
			w,
			h,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			g.data ) -- need to zero the texture?

gl.CheckError()

	end
	
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

	if images.remember then
		for v,n in pairs(images.preload or {}) do
			images.load(v,n)
		end
	end

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


