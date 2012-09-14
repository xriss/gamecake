-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.images")

base=require(...)
meta={}
meta.__index=base

local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")



function bake(opts)

	local images={}
	setmetatable(images,meta)
	
	images.cake=opts.cake
	images.gl=opts.gl
	
	images.data={}
	
	images.fmt=opts.cake.images_fmt
	images.zip=opts.zip
	images.prefix=opts.grdprefix or "data/"
	images.postfix=opts.grdpostfix or ".png"
	
	return images
end

get=function(images,id,name)
	name=name or "base"
	return images.data[name] and images.data[name][id]
end

set=function(images,d,id,name)
	name=name or "base"
	local tab
	
	if images.data[name] then
		tab=images.data[name]		
	else
		tab={}
		images.data[name]=tab
	end
	
	tab[id]=d	
end


--
-- unload a previously loaded image
--
unload=function(images,id,name)
	local gl=images.gl
	name=name or "base"
	
	local t=images:get(id,name)

	if t then
		if gl then --gl mode
				gl.DeleteTexture( t.id )			
		end
		images:set(nil,id,name)
	end
end

--
-- load a single image, and make it easy to lookup by the given id
--
load=function(images,filename,id,name)
	local gl=images.gl
	name=name or "base"

	local t=images:get(id,name)
	
	if t then return t end --first check it is not already loaded

print("loading",id,name)

	local fname=images.prefix..filename..images.postfix
	
	local g=assert(grd.create())
	

	local d=assert(zips.readfile(fname))
	assert(g:load_data(d,"png"))
	
--[[
	if images.zip then -- load from a zip file
		
		local f=assert(images.zip:open(fname))
		local d=assert(f:read("*a"))
		f:close()

		assert(g:load_data(d,"png"))
	else
		assert(g:load_file(fname,"png"))
	end
]]	
	if gl then --gl mode
	
		t={}
		images:upload_grd(t,g)

		images:set(t,id,name)
		
		t.filename=filename
		return t
	else
	
		assert(g:convert(images.fmt))
		
		images:set(g,id,name)

		g.filename=filename
		return g
	end
	
end

function uptwopow(n)

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

function upload_grd(images,t,g)
	local gl=images.gl

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
loads=function(images,tab)

	for i,v in pairs(tab) do
	
		if type(v)=="table" then -- use a subtable and its name
		
			for ii,vv in pairs(v) do
			
				if type(ii)=="number" then -- just use filename twice
					images:load(i.."_"..vv,vv,i)
				else
					images:load(i.."_"..vv,ii,i)
				end
				
			end
			
		elseif type(i)=="number" then -- just use filename twice
			images:load(v,v)
		else
			images:load(v,i)
		end
		
	end

end


start = function(images)

	for v,n in pairs(images.remember or {}) do
		images:load(v,n[1],n[2])
	end
	images.remember=nil
end

stop = function(images)

	images.remember={}
	
	for n,tab in pairs(images.data) do

		for i,t in pairs(tab) do
		
			images.remember[t.filename]={i,n}
		
			images:unload(i,n)
			
		end

	end

end


