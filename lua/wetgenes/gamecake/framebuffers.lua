--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")
local grd=require("wetgenes.grd")
local pack=require("wetgenes.pack")


-- webgl issues with smaller than 256 ???
local function uptwopow(n)

	if     n<1      then return 0
	elseif n<=1     then return 1
	elseif n<=2     then return 2
	elseif n<=4     then return 4
	elseif n<=8     then return 8
	elseif n<=16    then return 16
	elseif n<=32    then return 32
	elseif n<=64    then return 64
	elseif n<=128   then return 128
	elseif n<=256   then return 256
	elseif n<=512   then return 512
	elseif n<=1024  then return 1024
	elseif n<=2048  then return 2048
	elseif n<=4096  then return 4096
	elseif n<=8192  then return 8192
	elseif n<=16384 then return 16384
	elseif n<=32768 then return 32768
--	elseif n<=65536 then return 65536
	end
	return 65536
end

local function zero_data(w,h,p)
--	return nil
-- a nil will work on most drivers but the memory will not be cleared on some drivers.
-- so the only safe way to initialize a texture to 0 is with this dumb allocation
	return string.rep("\0",w*h*p)
end



--[[#lua.wetgenes.gamecake.framebuffers

Deal with FrameBuffers as render targets and textures. Color and Depth 
buffers need to be allocated and managed.

So we need to be baked within an oven so we will talk about the return 
from the bake function rather than the module function.

	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local fbo=framebuffers.create(256,256,0)	-- a 256x256 texture only
	local fbo=framebuffers.create(256,256,-1)	-- a 256x256 depth only
	local fbo=framebuffers.create(256,256,1)	-- a 256x256 texture and depth
	local fbo=framebuffers.create()				-- 0x0 and we will resize later

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,framebuffers)
		
	local gl=oven.cake.gles -- oven.rebake("wetgenes.gamecake.gles")
	local images=oven.cake.images -- oven.rebake("wetgenes.gamecake.images")
	local views=oven.cake.views -- oven.rebake("wetgenes.gamecake.views")
	
	framebuffers.stack={}
	framebuffers.peek=function() return framebuffers.stack[#framebuffers.stack] end
	framebuffers.push=function(fbo)
		framebuffers.stack[#framebuffers.stack+1]=fbo
	end
	framebuffers.pop=function()
		local fbo=framebuffers.peek()
		framebuffers.stack[#framebuffers.stack]=nil
		return fbo
	end

	local funcs={}
	local metatable={__index=funcs}

--	local vendor=string.lower((oven.gl.Get(oven.gl.VENDOR) or ""))
--	framebuffers.webkithax=(vendor=="webkit") -- enable webkit hax (TODO: turn this into a benchmark test)

	framebuffers.data=setmetatable({}, { __mode = 'vk' })


--[[#lua.wetgenes.gamecake.framebuffers.dirty

	framebuffers.dirty()

Mark all framebuffer objects as dirty by setting fbo.dirty to be true. We do not 
do anything with this flag but it is used in external code and this is 
a useful helper function to set the flag.

]]
	framebuffers.dirty = function()
		for n,v in pairs(framebuffers.data) do
			v.dirty=true
		end
	end


-- the d is one of these three

	framebuffers.NEED_DEPTH=-1
	framebuffers.NEED_TEXTURE=0
	framebuffers.NEED_TEXTURE_AND_DEPTH=1

--[[#lua.wetgenes.gamecake.framebuffers.create

	local fbo=framebuffers.dirty()
	local fbo=framebuffers.dirty(x,y)
	local fbo=framebuffers.dirty(x,y,framebuffers.NEED_TEXTURE_AND_DEPTH)
	local fbo=framebuffers.dirty(0,0,0,{
		depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT},
		texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE},
		TEXTURE_MIN_FILTER=gl.LINEAR,
		TEXTURE_MAG_FILTER=gl.LINEAR,
		TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE,
		TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE,
	}) -- note this table will be returned as the fbo

Create a new framebuffer object and optionally provide an inital size 
and depth. The depth can use -1,0,1 or the following verbose flags 
framebuffers.NEED_DEPTH,framebuffers.NEED_TEXTURE or 
framebuffers.NEED_TEXTURE_AND_DEPTH to request a depth buffer(1,-1) or not(0). 

Finally you can pass in a table to be returned as the fbo that contains 
defaults or set defaults in the fbo that is returned.

	fbo.depth_format={gl.DEPTH_COMPONENT32F,gl.DEPTH_COMPONENT,gl.FLOAT}

Can be used to control exactly how a depth buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.texture_format={gl.RGBA,gl.RGBA,gl.UNSIGNED_BYTE}

Can be used to control exactly how a texture buffer is allocated with 
gl.TexImage2D when you care about that sort of thing, IE hardware 
issues.

	fbo.TEXTURE_MIN_FILTER=gl.LINEAR
	fbo.TEXTURE_MAG_FILTER=gl.LINEAR
	fbo.TEXTURE_WRAP_S=gl.CLAMP_TO_EDGE
	fbo.TEXTURE_WRAP_T=gl.CLAMP_TO_EDGE

These can be used to control default TexParameters in the fbo.

	fbo.no_uptwopow=true
	
By deafult we will use power of 2 sizes for the fbo that fit the 
requested size. This disables that and doing so will of course cause 
problems with some hardware. Generally if you avoid mipmaps it probably 
wont be a problem.

See #lua.wetgenes.gamecake.framebuffers.fbo for all the functions you 
can call on the fbo returned.

]]
	framebuffers.create = function(w,h,d,def)

		local fbo=def or {} -- allow default settings on 4th param
		setmetatable(fbo,metatable)
		
		fbo.w=0 -- no memory alloced at start
		fbo.h=0
		fbo.d=0
			
		framebuffers.resize(fbo,w or 0,h or 0,d or 0)
				
		framebuffers.data[fbo]=fbo
		fbo[0]=pack.alloc(1,{__gc=function() fbo:clean() end}) -- force garbage with fake userdata
		
-- create an fbo view since we tend to need one
		fbo.view=views.create({
			mode="fbo",
			fbo=fbo,
		})

		return fbo
	end


--[[#lua.wetgenes.gamecake.framebuffers.start

	framebuffers.start()

Called as part of the android life cycle, since we auto reallocate this 
does not actually have to do anything. But may do a forced resize in 
the future if that turns out to be more hardware compatible.

]]
	framebuffers.start = function()
--		for v,n in pairs(framebuffers.data) do
--			framebuffers.resize(v,v.w,v.h,v.d) -- realloc
--		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.stop

	framebuffers.stop()

Called as part of the android life cycle, we go through and call 
fbo.clean on each fbo to free the opengl resources.

]]
	framebuffers.stop = function()
		for v,n in pairs(framebuffers.data) do
			framebuffers.clean(v)
		end

	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.free_depth

	fbo:free_depth()

Free the depth texture only, is safe to call if there is no depth 
buffer.

]]
	framebuffers.free_depth = function(fbo)
		if fbo.depth then
			gl.DeleteTexture(fbo.depth)
			fbo.depth=nil
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.free_texture

	fbo:free_texture()

Free the rgba texture only, is safe to call if there is no rgba 
buffer.

]]
	framebuffers.free_texture = function(fbo)
		if fbo.texture then
			gl.DeleteTexture(fbo.texture)
			fbo.texture=nil
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.free_snapshot

	fbo:free_snapshot()

Free the snapshot buffers, fbo.texture_snapshot and fbo.depth_snapshot 
if they exist.

]]
	framebuffers.free_snapshot = function(fbo)
		if fbo.texture_snapshot then
			gl.DeleteTexture(fbo.texture_snapshot)
			fbo.texture_snapshot=nil
		end
		if fbo.depth_snapshot then
			gl.DeleteTexture(fbo.depth_snapshot)
			fbo.depth_snapshot=nil
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.free_frame

	fbo:free_frame()

Free the frame buffer only, is safe to call if there is no frame 
buffer.

]]
	framebuffers.free_frame = function(fbo)
		if fbo.frame then
			gl.DeleteFramebuffer(fbo.frame)
			fbo.frame=nil
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.clean

	fbo:clean()

Free all the opengl buffers.

]]
	framebuffers.clean = function(fbo)
		framebuffers.free_snapshot(fbo)
		framebuffers.free_depth(fbo)
		framebuffers.free_texture(fbo)
		framebuffers.free_frame(fbo)
		fbo.w=0
		fbo.h=0
		fbo.d=0
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.bind_texture

	fbo:bind_texture()

BindTexture the rgba texture part of this fbo.

If there is no texture we will bind 0.

]]
	framebuffers.bind_texture = function(fbo)
		if fbo and fbo.texture then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.bind_texture_snapshot

	fbo:bind_texture_snapshot()

BindTexture the rgba snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.

]]
	framebuffers.bind_texture_snapshot = function(fbo)
		if fbo and fbo.texture_snapshot then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture_snapshot or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end
	
--[[#lua.wetgenes.gamecake.framebuffers.fbo.bind_depth

	fbo:bind_depth()

BindTexture the depth texture part of this fbo.

If there is no texture we will bind 0.

]]
	framebuffers.bind_depth = function(fbo)
		if fbo and fbo.depth then
			gl.BindTexture(gl.TEXTURE_2D, fbo.depth or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.bind_depth_snapshot

	fbo:bind_depth_snapshot()

BindTexture the depth snapshot texture part of this fbo.

If there is no snapshot texture we will bind 0.

]]
	framebuffers.bind_depth_snapshot = function(fbo)
		if fbo and fbo.depth_snapshot then
			gl.BindTexture(gl.TEXTURE_2D, fbo.depth_snapshot or 0)
		else
			gl.BindTexture(gl.TEXTURE_2D, 0)
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.bind_frame

	fbo:bind_depth()

BindFramebuffer the framebuffer of this fbo.

If there is no framebuffer we will bind 0.

]]
	framebuffers.bind_frame = function(fbo)
		if fbo then
			gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame or 0)
		else
			gl.BindFramebuffer(gl.FRAMEBUFFER,0)
		end
	end
	
--[[#lua.wetgenes.gamecake.framebuffers.fbo.check

	fbo:check()

Check and allocatie if missing and needed (eg depth texture may not be 
needed) all our openGL buffers.

]]
	framebuffers.check = function(fbo)
		framebuffers.resize(fbo,fbo.w,fbo.h,fbo.d) -- realloc if we need to
	end


-- internal use only
	framebuffers.initialize_depth = function(fbo)
-- this is the only filter that *will* work on depth buffers
			if wwin.flavour=="emcc" then -- hack, should test
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
			else
-- I really want this one, but we will need to test it works before we can use it
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
			end

			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     gl.CLAMP_TO_EDGE)
			gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     gl.CLAMP_TO_EDGE)

			if fbo.depth_format then
				gl.TexImage2D(gl.TEXTURE_2D, 0, fbo.depth_format[1], fbo.txw, fbo.txh, 0, fbo.depth_format[2], fbo.depth_format[3],zero_data(fbo.txw,fbo.txh,4))
			elseif math.abs(fbo.d)>=32 then
				gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT32F, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.FLOAT,zero_data(fbo.txw,fbo.txh,4))
			elseif math.abs(fbo.d)>=24 then
				gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT24, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.UNSIGNED_INT,zero_data(fbo.txw,fbo.txh,4))
			else
				gl.TexImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT16, fbo.txw, fbo.txh, 0, gl.DEPTH_COMPONENT, gl.UNSIGNED_SHORT,zero_data(fbo.txw,fbo.txh,2))
			end
	end

-- internal use only
	framebuffers.initialize_texture = function(fbo)

		gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, fbo.TEXTURE_MIN_FILTER or framebuffers.TEXTURE_MIN_FILTER or gl.LINEAR)
		gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, fbo.TEXTURE_MAG_FILTER or framebuffers.TEXTURE_MAG_FILTER or gl.LINEAR)
		gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,     fbo.TEXTURE_WRAP_S     or framebuffers.TEXTURE_WRAP_S     or gl.CLAMP_TO_EDGE)
		gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,     fbo.TEXTURE_WRAP_T     or framebuffers.TEXTURE_WRAP_T     or gl.CLAMP_TO_EDGE)

		if fbo.texture_format then
			gl.TexImage2D(gl.TEXTURE_2D, 0, fbo.texture_format[1], fbo.txw, fbo.txh, 0, fbo.texture_format[2], fbo.texture_format[3],zero_data(fbo.txw,fbo.txh,4))
		else
			gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, fbo.txw, fbo.txh, 0, gl.RGBA, gl.UNSIGNED_BYTE,zero_data(fbo.txw,fbo.txh,4))
		end

--		else -- I think these might be the safest defaults on low hardware GLES2 should work with this
--			gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGB565, fbo.txw, fbo.txh, 0, gl.RGB, gl.UNSIGNED_SHORT_5_6_5,string.rep("\0",fbo.txw*fbo.txh*2))

	end


--[[#lua.wetgenes.gamecake.framebuffers.fbo.resize

	fbo:resize(w,h,d)

Change the size of our buffers, which probably means free and then
reallocate them.

The w,h,d use the same rules as framebuffer.create

]]
	framebuffers.resize = function(fbo,w,h,d)
--print("fbo resize",w,h,d)

		if w<1 then w=0 end
		if h<1 then h=0 end

		if w==0 then h=0 d=0 end
		if h==0 then d=0 w=0 end
		
		fbo.w=w
		fbo.h=h
		fbo.d=d
	
		if w==0 or h==0 then framebuffers.free_texture(fbo) end
		
		if d==0 then framebuffers.free_depth(fbo) end

		if w~=0 and h~=0 then
			if fbo.no_uptwopow then -- do not auto increase size of fbo
				fbo.txw=w
				fbo.txh=h
			else
				fbo.txw=uptwopow(w) -- always keep textures in power of two for simplicity (hardware probs)
				fbo.txh=uptwopow(h)
			end
			if d~=0 then
				if not fbo.depth then
					fbo.depth=gl.GenTexture()
				end
				
				gl.BindTexture(gl.TEXTURE_2D, fbo.depth)
				fbo:initialize_depth()
				gl.BindTexture(gl.TEXTURE_2D, 0)

			end

			fbo.uvw=w/fbo.txw -- need to use these max uv coords when drawing with texture instead of 1
			fbo.uvh=h/fbo.txh 

			if d>=0 then -- negative depth to disable color buffer
			
				if not fbo.texture then
					fbo.texture=gl.GenTexture()
				end

				gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
				fbo:initialize_texture()
				gl.BindTexture(gl.TEXTURE_2D, 0)

			end
			

			if not fbo.frame then
--print("CREATE FBO",gl.CheckFramebufferStatus(gl.FRAMEBUFFER))
				fbo.frame=gl.GenFramebuffer()
			end
			gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame)

			if fbo.depth then -- optional depth
--print("APPLY DEPTH",gl.CheckFramebufferStatus(gl.FRAMEBUFFER))
				gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, fbo.depth, 0)
			end
			if fbo.texture then
--print("APPLY TEXTURE",gl.CheckFramebufferStatus(gl.FRAMEBUFFER))
				gl.FramebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture, 0)
			end

			if( gl.CheckFramebufferStatus(gl.FRAMEBUFFER) ~= gl.FRAMEBUFFER_COMPLETE) then
				local status=gl.CheckFramebufferStatus(gl.FRAMEBUFFER)
				if gl.nums[status] then status=gl.nums[status]..":"..status end
				local err=gl.GetError()
				if gl.nums[err] then err=gl.nums[err]..":"..err end
				print("FBO error".." SIZE="..fbo.txw.."x"..fbo.txh.."x"..d.." STATUS="..status.." ERROR="..err)
				if  fbo.texture_format then
					print( unpack(fbo.texture_format) )
				end
--			else
--				print("FBO OK".." SIZE="..fbo.txw.."x"..fbo.txh.."x"..d)
			end
			
			gl.BindFramebuffer(gl.FRAMEBUFFER, 0)

		end

	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.snapshot

	fbo:snapshot()

Take a current snapshot copy of the texture and depth if they exist, 
store them in fbo.texture_snapshot and fbo.depth_snapshot for later 
binding.

]]
	framebuffers.snapshot = function(fbo)

		if not fbo.frame then return end -- nothing to snapshot

		if fbo.texture then
			if not fbo.texture_snapshot then
				fbo.texture_snapshot=gl.GenTexture()
				gl.BindTexture(gl.TEXTURE_2D, fbo.texture_snapshot)
				fbo:initialize_texture()
				gl.BindTexture(gl.TEXTURE_2D, 0)
			end
		end
		if fbo.depth then
			if not fbo.depth_snapshot then
				fbo.depth_snapshot=gl.GenTexture()
				gl.BindTexture(gl.TEXTURE_2D, fbo.depth_snapshot)
				fbo:initialize_depth()
				gl.BindTexture(gl.TEXTURE_2D, 0)
			end
		end

		local frame=gl.GenFramebuffer()
		gl.BindFramebuffer(gl.READ_FRAMEBUFFER, fbo.frame)
		gl.BindFramebuffer(gl.DRAW_FRAMEBUFFER, frame)

-- prepare snapshot textures
--[[
		if fbo.depth then
			gl.FramebufferTexture2D(gl.READ_FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, fbo.depth, 0)
		end
		if fbo.texture then
			gl.FramebufferTexture2D(gl.READ_FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture, 0)
		end
]]
		if fbo.depth_snapshot then
			gl.FramebufferTexture2D(gl.DRAW_FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, fbo.depth_snapshot, 0)
		end
		if fbo.texture_snapshot then
			gl.FramebufferTexture2D(gl.DRAW_FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fbo.texture_snapshot, 0)
		end
		if( gl.CheckFramebufferStatus(gl.DRAW_FRAMEBUFFER) ~= gl.FRAMEBUFFER_COMPLETE) then
			error("FBO error".." DEPTH="..fbo.d.." STATUS="..gl.CheckFramebufferStatus(gl.FRAMEBUFFER).." ERROR="..gl.GetError())
		end
-- copy textures
		gl.BlitFramebuffer(
			0,0,fbo.txw,fbo.txh,
			0,0,fbo.txw,fbo.txh,
			gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT,gl.NEAREST
		)
		gl.BindFramebuffer(gl.FRAMEBUFFER, fbo.frame) -- assumption
		gl.DeleteFramebuffer(frame)

	end

	
--[[#lua.wetgenes.gamecake.framebuffers.fbo.mipmap

	fbo:mipmap()

Build mipmaps for all existing texture buffers.

]]
	framebuffers.mipmap = function(fbo)
		framebuffers.mipmap_texture(fbo)
		framebuffers.mipmap_depth(fbo)
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.mipmap_texture

	fbo:mipmap_texture()

Build mipmaps for our texture buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.

]]
	framebuffers.mipmap_texture = function(fbo) -- generate mipmaps and enable default mipmapping filter
		if fbo.texture then
			gl.BindTexture(gl.TEXTURE_2D, fbo.texture)
			gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,fbo.TEXTURE_MIN_FILTER or framebuffers.TEXTURE_MIN_FILTER or gl.LINEAR_MIPMAP_LINEAR)
			gl.GenerateMipmap(gl.TEXTURE_2D)
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.mipmap_depth

	fbo:mipmap_depth()

Build mipmaps for our depth buffer if it exists and set 
TEXTURE_MIN_FILTER to LINEAR_MIPMAP_LINEAR so it will be used.

It is possible this may fail (hardware issues) and the 
TEXTURE_MIN_FILTER be reset to gl.NEAREST along with a flag to stop us 
even trying in the future,

]]
	framebuffers.mipmap_depth_broken=false
	framebuffers.mipmap_depth = function(fbo) -- generate mipmaps and enable default mipmapping filter
		if fbo.depth then
			if not framebuffers.mipmap_depth_broken then
				gl.BindTexture(gl.TEXTURE_2D, fbo.depth)
				gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR)
				gl.GenerateMipmap(gl.TEXTURE_2D)
				if gl.GetError()==gl.INVALID_OPERATION then -- bad driver, we fucked
					gl.TexParameter(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
					framebuffers.mipmap_depth_broken=true
				end
			end
		end
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.download

	fbo:download()
	fbo:download(w,h,x,y)

Read back color data from a framebuffer and return it in an RGBA 
PREMULT grd object (which is probably what it is).

If a width,height is given then we will read the given pixels only from 
the x,y location.

]]
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


--[[#lua.wetgenes.gamecake.framebuffers.fbo.render_start

	fbo:render_start()


Start rendering into this fbo.

Push old matrix and set the matrix mode to MODELVIEW

Set fbo.view and reset the gl state.

]]
	framebuffers.render_start = function(fbo)
	
		framebuffers.push(fbo)
		
		gl.MatrixMode(gl.PROJECTION)
		gl.PushMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PushMatrix()
		views.push_and_apply(fbo.view)
		gl.state.push(gl.state_defaults)
		fbo:bind_frame()
		
	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.render_stop

	fbo:render_stop()

Stop rendering into this fbo and restore the last fbo so these calls 
can be nested.

Restore the old view and old gl state.

Pop old matrix and set the matrix mode to MODELVIEW

]]
	framebuffers.render_stop = function(fbo)
	
		local fbo=framebuffers.pop()
		local oldfbo=framebuffers.peek() -- may be nil

		gl.BindFramebuffer(gl.FRAMEBUFFER, oldfbo and oldfbo.frame or 0)
		gl.state.pop()
		views.pop_and_apply()
		gl.MatrixMode(gl.PROJECTION)
		gl.PopMatrix()
		gl.MatrixMode(gl.MODELVIEW)
		gl.PopMatrix()

	end

--[[#lua.wetgenes.gamecake.framebuffers.fbo.pingpong

	fbo:pingpong(fbout,shadername,callback)
	framebuffers.pingpong({fbin1,fbin2},fbout,shadername,callback)

Render from one or more fbos into another using a fullscreen shader.

Sometime you need to repeatedly copy a texture back and though applying 
a shader, this is the function for you.

The textures will be bound to tex1,tex2,tex3,etc and the uvs supplied 
in a_texcoord with a_vertex being set to screen coords so can be used 
as is.

]]
	framebuffers.pingpong = function(fbin,fbout,shadername,callback)
	
		local canvas=oven.rebake("wetgenes.gamecake.canvas")
		local views=oven.rebake("wetgenes.gamecake.views")
	
		if not fbin[1] then fbin={fbin} end -- upgrade input to array
		
		fbout:render_start()

		gl.state.set({
			[gl.BLEND]						=	gl.FALSE,
			[gl.CULL_FACE]					=	gl.FALSE,
			[gl.DEPTH_TEST]					=	gl.FALSE,
			[gl.DEPTH_WRITEMASK]			=	gl.FALSE,
		})


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

		canvas.flat.tristrip("rawuv",t,shadername,function(p)
			for i=#fbin,1,-1 do -- bind all textures in reverse order so we always end with texture0 as active
				gl.ActiveTexture(gl.TEXTURE0+(i-1))
				local u=p:uniform("tex"..i)
				if u>=0 then 
					gl.Uniform1i( u , i-1 )
				end
				fbin[i]:bind_texture()
			end
			if callback then callback(p) end
		end)

		fbout:render_stop()

	end

-- set some functions into the metatable of each fbo
	for n,v in pairs(framebuffers) do
		if type(v)=="function" then
			funcs[n]=v
		end
	end

	return framebuffers
end




