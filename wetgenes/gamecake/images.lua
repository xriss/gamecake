-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


module("wetgenes.gamecake.images")

base=require(...)
meta={}
meta.__index=base

local grd=require("wetgenes.grd")



function create(opts)

	local images={}
	setmetatable(images,meta)
	
	images.cake=opts.cake
	images.gl=opts.gl
	
	if images.gl then --gl mode
		images.texs={}
	else
		images.grds={}
	end
	
	images.fmt=opts.cake.images_fmt
	images.zip=opts.zip
	images.prefix=opts.prefix or "art/out"
	images.postfix=opts.postfix or ".png"
	

	return images
end


--
-- load a single image, and make it easy to lookup by the given id
--
load=function(images,name,id)

	local gl=images.gl

	local fname=images.prefix..name..images.postfix
	
	local g=assert(grd.create())
	
	if images.zip then -- load from a zip file
		
		local f=assert(images.zip:open(fname))
		local d=assert(f:read("*a"))
		f:close()

		assert(g:load_data(d,"png"))
	else
		assert(g:load_file(fname,"png"))
	end
	
	if gl then --gl mode
	
		t={}
		t.id=assert(gl.GenTexture())
		t.w=g.width
		t.h=g.height
		t.tw=g.width
		t.th=g.height

		gl.BindTexture( gl.TEXTURE_2D , t.id )
		
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE)
	
		assert(g:convert(grd.FMT_U8_RGBA))
--		assert(g:convert(grd.FMT_U16_RGBA_4444))
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			g.width,
			g.height,
			0,
			gl.RGBA,
--			gl.GL_UNSIGNED_SHORT_4_4_4_4,
			gl.UNSIGNED_BYTE,
			g.data)

		images.texs[id]=t
	else
	
		assert(g:convert(images.fmt))
		
		images.grds[id]=g
	end
	

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




