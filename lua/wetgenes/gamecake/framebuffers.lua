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


-- the d is one of these three

	framebuffers.NEED_DEPTH=-1
	framebuffers.NEED_TEXTURE=0
	framebuffers.NEED_TEXTURE_AND_DEPTH=1

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
			gl.DeleteTexture(fbo.depth)
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
		if fbo and fbo.texture then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end
	
	framebuffers.bind_depth = function(fbo)
		if fbo and fbo.depth then
			gl.BindTexture(gl.TEXTURE_2D, fbo.depth or 0)
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

		if w<1 then w=0 end
		if h<1 then h=0 end

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
					fbo.depth=gl.GenTexture()
				end

				gl.BindTexture(gl.TEXTURE_2D, fbo.depth)

				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE)

				if fbo.depth_format then
					gl.TexImage2D(gl.TEXTURE_2D, 0, fbo.depth_format[1], fbo.txw, fbo.txh, 0, fbo.depth_format[2], fbo.depth_format[3],nil)
				elseif math.abs(d)>=32 then
					gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT32F, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.FLOAT,nil)
				elseif math.abs(d)>=24 then
					gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT24, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.UNSIGNED_INT,nil)
				else
					gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT16, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.UNSIGNED_SHORT,nil)
				end

				gl.BindTexture(gl.TEXTURE_2D, 0)

			end

			fbo.uvw=w/fbo.txw -- need to use these max uv coords when drawing with texture instead of 1
			fbo.uvh=h/fbo.txh 

			if d>=0 then -- negative depth to disable color buffer
			
				if not fbo.texture then
					fbo.texture=gl.GenTexture()
				end

				gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
				
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, fbo.TEXTURE_MIN_FILTER or framebuffers.TEXTURE_MIN_FILTER or gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, fbo.TEXTURE_MAG_FILTER or framebuffers.TEXTURE_MAG_FILTER or gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     fbo.TEXTURE_WRAP_S     or framebuffers.TEXTURE_WRAP_S     or gl.CLAMP_TO_EDGE)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     fbo.TEXTURE_WRAP_T     or framebuffers.TEXTURE_WRAP_T     or gl.CLAMP_TO_EDGE)

				if fbo.texture_format then
					gl.TexImage2D(gl.TEXTURE_2D, 0, fbo.texture_format[1], fbo.txw, fbo.txh, 0, fbo.texture_format[2], fbo.texture_format[3],nil)
				else
					gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, fbo.txw, fbo.txh, 0, gl.RGBA, gl.UNSIGNED_BYTE,nil)
				end
				
				gl.BindTexture(gl.TEXTURE_2D, 0)

			end
			

			if not fbo.frame then
				fbo.frame=gl.GenFramebuffer()
			end
			gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame)

			if fbo.depth then -- optional depth
				gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, fbo.depth, 0)
			end
			if fbo.texture then
				gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture, 0)
			end

			if( gl.CheckFramebufferStatus(gl.FRAMEBUFFER) ~= gl.FRAMEBUFFER_COMPLETE) then
				error("FBO error".." DEPTH="..d.." STATUS="..gl.CheckFramebufferStatus(gl.FRAMEBUFFER).." ERROR="..gl.GetError())
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
		"bind_depth",
		"resize",
		"download",
		"mipmap",
		"free_depth",
		"free_texture",
		"free_frame",
		}) do
		funcs[n]=framebuffers[n]
	end

-- render from one or more fbos into another using a fullscreen shader

	framebuffers.pingpong = function(fbin,fbout,shadername,callback)
	
		local views=oven.cake.views
	
		if not fbin[1] then fbin={fbin} end -- upgrade input to array
		
		local view=views.create({
			mode="fbo",
			fbo=fbout,
		})

		gl.PushMatrix()
		views.push_and_apply(view)
		gl.state.push(gl.state_defaults)
		gl.state.set({
			[gl.BLEND]						=	gl.FALSE,
			[gl.CULL_FACE]					=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.FALSE,
			[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
		})

		fbout:bind_frame()

		local v1=gl.apply_modelview( {fbout.w*-0,	fbout.h* 1,	0,1} )
		local v2=gl.apply_modelview( {fbout.w*-0,	fbout.h*-0,	0,1} )
		local v3=gl.apply_modelview( {fbout.w* 1,	fbout.h* 1,	0,1} )
		local v4=gl.apply_modelview( {fbout.w* 1,	fbout.h*-0,	0,1} )
		local t={
			v1[1],	v1[2],	v1[3],	0,				0, 			
			v2[1],	v2[2],	v2[3],	0,				fbin[1].uvh,
			v3[1],	v3[2],	v3[3],	fbin[1].uvw,	0, 			
			v4[1],	v4[2],	v4[3],	fbin[1].uvw,	fbin[1].uvh,
		}

		oven.cake.canvas.flat.tristrip("rawuv",t,shadername,function(p)
			for i=#fbin,1,-1 do -- bind all textures in reverse order so we always end with texture0 as active
				gl.ActiveTexture(gl.TEXTURE0+(i-1))
				local u=p:uniform("tex"..i)
				if u>=0 then 
					gl.Uniform1i( u , i-1 )
				end
				if i==0 then
					local u=p:uniform("tex")
					if u>=0 then 
						gl.Uniform1i( u , 0 )
					end
				end
				fbin[i]:bind_texture()
			end
			if callback then callback(p) end
		end)

		gl.BindFramebuffer(gl.FRAMEBUFFER, 0)

		gl.state.pop()
		views.pop_and_apply()
		gl.PopMatrix()


	end

	return framebuffers
end




