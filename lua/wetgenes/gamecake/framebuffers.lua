--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local grd=require("wetgenes.grd")
local pack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,framebuffers)
		
	local gl=oven.gl
	local cake=oven.cake
	local images=cake.images
	
	local funcs={}
	local metatable={__index=funcs}
	
	framebuffers.data=setmetatable({}, { __mode = 'vk' })

	framebuffers.dirty = function()
		for n,v in pairs(framebuffers.data) do
			v.dirty=true
		end
	end

	framebuffers.create = function(w,h,d,def)

		local fbo=def or {} -- allow default settings on 4th param
		
		fbo.w=0 -- no memory alloced at start
		fbo.h=0
		fbo.d=0
			
		framebuffers.resize(fbo,w or 0,h or 0,d or 0)
				
		framebuffers.data[fbo]=fbo
		
		setmetatable(fbo,metatable)
		fbo.__gc=pack.alloc(1,{__gc=function() fbo:clean() end})
		
		return fbo
	end


	framebuffers.start = function()
		for v,n in pairs(framebuffers.data) do
--			framebuffers.resize(v,v.w,v.h,v.d) -- realloc
		end
	end

	framebuffers.stop = function()
		for v,n in pairs(framebuffers.data) do
			framebuffers.clean(v)
		end

	end

	framebuffers.free_depth = function(fbo)
		if fbo.depth then
			gl.DeleteRenderbuffer(fbo.depth)
			fbo.depth=nil
		end
	end

	framebuffers.free_texture = function(fbo)
		if fbo.texture then
			gl.DeleteTexture(fbo.texture)
			fbo.texture=nil
		end
	end

	framebuffers.free_frame = function(fbo)
		if fbo.frame then
			gl.DeleteFramebuffer(fbo.frame)
			fbo.frame=nil
		end
	end

	framebuffers.clean = function(fbo)
		framebuffers.free_depth(fbo)
		framebuffers.free_texture(fbo)
		framebuffers.free_frame(fbo)
		fbo.w=0
		fbo.h=0
		fbo.d=0
	end

	framebuffers.bind_texture = function(fbo)
		if fbo then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end
	
	framebuffers.bind_frame = function(fbo)
		if fbo then
			gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame or 0)
		else
			gl.BindFramebuffer(gl.FRAMEBUFFER,0)
		end
	end
	
	framebuffers.check = function(fbo)
		framebuffers.resize(fbo,fbo.w,fbo.h,fbo.d) -- realloc if we need to
	end

	framebuffers.resize = function(fbo,w,h,d)
--print("fbo resize",w,h,d)

		if w==0 then h=0 d=0 end
		if h==0 then d=0 w=0 end
	
		if w==0 or h==0 then framebuffers.free_texture(fbo) end
		
		if d==0 then framebuffers.free_depth(fbo) end

		if w~=0 and h~=0 then
			if fbo.no_uptwopow then -- do not auto increase size of fbo
				fbo.txw=w
				fbo.txh=h
			else
				fbo.txw=images.uptwopow(w) -- always keep textures in power of two for simplicity (hardware probs)
				fbo.txh=images.uptwopow(h)
			end
			if d~=0 then
				if not fbo.depth then
					fbo.depth=gl.GenRenderbuffer()
				end
				gl.BindRenderbuffer(gl.RENDERBUFFER, fbo.depth)
				gl.RenderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, fbo.txw, fbo.txh)
			end
			if not fbo.texture then
				fbo.texture=gl.GenTexture()
			end
			fbo.uvw=w/fbo.txw -- need to use these max uv coords when drawing with texture instead of 1
			fbo.uvh=h/fbo.txh 

			gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, fbo.TEXTURE_MIN_FILTER or framebuffers.TEXTURE_MIN_FILTER or gl.LINEAR)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, fbo.TEXTURE_MAG_FILTER or framebuffers.TEXTURE_MAG_FILTER or gl.LINEAR)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     fbo.TEXTURE_WRAP_S     or framebuffers.TEXTURE_WRAP_S     or gl.CLAMP_TO_EDGE)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     fbo.TEXTURE_WRAP_T     or framebuffers.TEXTURE_WRAP_T     or gl.CLAMP_TO_EDGE)
			gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, fbo.txw, fbo.txh, 0, gl.RGBA, gl.UNSIGNED_BYTE,
				string.rep("\0\0\0\0",fbo.txw*fbo.txh)) -- might need some zero data, depends on driver so safest to provide it.

			gl.BindTexture(gl.TEXTURE_2D, 0)
			
			if not fbo.frame then
				fbo.frame=gl.GenFramebuffer()
			end
			gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame)

			if fbo.depth then -- optional depth
				gl.FramebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, fbo.depth)
			end
			gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture, 0)

			if( gl.CheckFramebufferStatus(gl.FRAMEBUFFER) ~= gl.FRAMEBUFFER_COMPLETE) then
				print("FBO error",fbo.depth,gl.CheckFramebufferStatus(gl.FRAMEBUFFER),gl.GetError())
			end
			
			gl.BindFramebuffer(gl.FRAMEBUFFER, 0)

		end
		
		fbo.w=w
		fbo.h=h
		fbo.d=d

	end
	
	framebuffers.mipmap = function(fbo) -- generate mipmaps and enable default mipmapping filter
--print("fbo mipmap???",tostring(fbo.texture))
--print(debug.traceback())

		if fbo.texture then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,fbo.TEXTURE_MIN_FILTER or framebuffers.TEXTURE_MIN_FILTER or gl.LINEAR_MIPMAP_LINEAR)
--print("FBmipmap",fbo.texture,fbo.txw, fbo.txh,gl.GetError())
			gl.GenerateMipmap(gl.TEXTURE_2D)	
		end
	end

-- read back data from a framebuffer, return it in a grd object
	framebuffers.download = function(fbo,w,h,x,y)
--print("fbo download",w,h,x,y)
	
		w=w or fbo.w
		h=h or fbo.h
		x=x or 0
		y=y or 0

		local g=assert(grd.create(grd.FMT_U8_RGBA_PREMULT,w,h,1))

		framebuffers.bind_frame(fbo)
		
		gl.ReadPixels(
			x,
			y,
			w,
			h,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			g.data )
			
		g:flipy() -- opengl data comes in upside down
		
		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
		
		return g
	end


-- set some functions into the metatable of each fbo
	for i,n in ipairs({
		"clean",
		"check",
		"bind_frame",
		"bind_texture",
		"resize",
		"download",
		"mipmap",
		"free_depth",
		"free_texture",
		"free_frame",
		}) do
		funcs[n]=framebuffers[n]
	end

	return framebuffers
end




