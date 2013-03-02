-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local grd=require("wetgenes.grd")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,framebuffers)
		
	local gl=oven.gl
	local cake=oven.cake
	
	local funcs={}
	local metatable={__index=funcs}
	
	framebuffers.data={}

	framebuffers.create = function(w,h,d)

		local fbo={w=0,h=0,d=0}
		
		framebuffers.resize(fbo,w or 0,h or 0,d or 0)
				
		framebuffers.data[fbo]=fbo
		
		setmetatable(fbo,metatable)
		
		return fbo
	end


	framebuffers.start = function()
		for v,n in pairs(framebuffers.data) do
--			framebuffers.resize(v,v.w,v.h,v.d) -- realloc
		end
	end

	framebuffers.stop = function()
		for v,n in pairs(framebuffers.data) do
			framebuffers.clean(fbo)
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

		if w==0 then h=0 d=0 end
		if h==0 then d=0 w=0 end
	
		if w==0 or h==0 or w~=fbo.w or h~=fbo.h or d~=fbo.d then -- size 0 or any change means free everything
			framebuffers.clean(fbo)
		end
		
		if w~=0 and h~=0 then 
			if d~=0 then
				if not fbo.depth then
					fbo.depth=gl.GenRenderbuffer()
					gl.BindRenderbuffer(gl.RENDERBUFFER, fbo.depth)
					gl.RenderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, w, h)
				end
			end
			if not fbo.texture then
				fbo.texture=gl.GenTexture()
				gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
				gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, 0)
				gl.GenerateMipmap(gl.DONT_CARE)
				gl.BindTexture(gl.TEXTURE_2D, 0)
			end
			if not fbo.frame then
				fbo.frame=gl.GenFramebuffer()
				gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame)

				if fbo.depth then -- optional depth
					gl.FramebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, fbo.depth)
				end
				gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture, 0)

				assert( gl.CheckFramebufferStatus(gl.FRAMEBUFFER) == gl.FRAMEBUFFER_COMPLETE)
				
				gl.BindFramebuffer(gl.FRAMEBUFFER, 0)
			end
		end
		
		fbo.w=w
		fbo.h=h
		fbo.d=d
		
	end

-- read back data from a framebuffer, return it in a grd object
	framebuffers.download = function(fbo,w,h,x,y)
	
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
		"free_depth",
		"free_texture",
		"free_frame",
		}) do
		funcs[n]=framebuffers[n]
	end

	return framebuffers
end




