--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")

local wwin=require("wetgenes.win")
local wtongues=require("wetgenes.tongues")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,images)
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
			
	
--	local stash=oven.rebake("wetgenes.gamecake.stash")
--	images.exists=stash.exists -- check files exists

	images.data={}
	images.gl_mem=0
	
	if opts.gl_mem_fake_limit then
		images.gl_mem_fake_limit=opts.gl_mem_fake_limit -- 1024*1024*16
	end
	
	images.prefix=opts.grdprefix or "data/"
	images.postfix=opts.grdpostfix or { ".png" , ".jpg" }

images.get=function(id)
	return images.data[id]
end

images.set=function(d,id)
	images.data[id]=d
end


images.delete=function(id)
	local t
	if type(id)=="table" then
		t=id
	else
		t=images.get(id)
	end
	if t then
		images.unload(t)
		images.data[t.id]=nil
	end
end

--
-- unload a previously loaded image
--
images.unload=function(id)
	local t
	if type(id)=="table" then
		t=id
	else
		t=images.get(id)
	end

	if t and t.gl then
		gl.DeleteTexture( t.gl )

		if t.gl_mem then
			images.gl_mem=images.gl_mem-t.gl_mem
			t.gl_mem=0
		end

		t.gl=nil	
	end
end

images.reload=function(id)
	local t
	if type(id)=="table" then
		t=id
	else
		t=images.get(id)
	end

	if t then
--print("RELOAD",t.id,t.mip)
		images.upload(t) -- upload to GL
oven.preloader("reload",string.format("%9s : %s",(t.gl_width or 0).."x"..(t.gl_height or 0),t.id))

		local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")
		framebuffers.dirty() -- ask for all cached fbo to redraw, as they may now be out of sync
	end
end

images.find_image=function(filename)

--	local fname="data/"..filename..".00.lua" -- mips?

	for i,v in ipairs( { ".00.lua" , ".png" , ".jpg" , ".gif" } ) do
		local fext=v
		local fname=images.prefix..filename..fext -- hardcode
		if zips.exists(fname) then  return fname ,fext end -- found it
	end

end
images.exists=function(filename)
	return images.find_image(filename) and true or false
end

local function logmip(x,y)

	local mx=math.ceil(math.log(x)/math.log(2))
	local my=math.ceil(math.log(y)/math.log(2))
	
	if mx>my then return mx else return my end

end

images.prep_image=function(t)

-- record image flags at loadtime, so set any flags you want here in images.TEXTURE_*
-- and they will be copied at this stage
	t.TEXTURE_MIN_FILTER	=	images.TEXTURE_MIN_FILTER
	t.TEXTURE_MAG_FILTER	=	images.TEXTURE_MAG_FILTER
	t.TEXTURE_WRAP_S		=	images.TEXTURE_WRAP_S
	t.TEXTURE_WRAP_T		=	images.TEXTURE_WRAP_T


	local lson=zips.readfile(images.prefix..t.filename..".lua")
	if lson then -- check for lua metadata
		t.meta=wsbox.lson(lson) -- return it with the image
	end

	local g

	if t.fg then g=t.fg() end -- image get function
	
	if g then -- load image ?
	
			t.width=g.width
			t.height=g.height
			t.texture_width=t.width
			t.texture_height=t.height

			t.max_mips=logmip(g.width,g.height)
			if t.max_mips > images.max_mips then t.max_mips=images.max_mips end

	else
	
		t.dataname,t.fext=images.find_image(t.filename)
		if t.fext==".00.lua" then -- many files, load and pick
		
			t.dataname=nil
		
			local lson=zips.readfile(images.prefix..t.filename..".00.lua")
			if lson then -- check for lua mips metadata
				t.mips=assert(wsbox.lson(lson)) -- return it with the image
				t.max_mips=t.mips.mip
				if t.max_mips > images.max_mips then t.max_mips=images.max_mips end
			end

			t.fext=t.mips.ext

			t.width=t.mips.width
			t.height=t.mips.height
			t.texture_width=t.mips.texture_width
			t.texture_height=t.mips.texture_height

		else -- single file, need to open now to get the file sizes
		
			g=assert(grd.create())
			local d=assert(zips.readfile(t.dataname),"Failed to load "..t.dataname)
			assert(g:load_data(d,t.filename:sub(-3))) -- last 3 letters pleaze

			t.max_mips=logmip(g.width,g.height)
			if t.max_mips > images.max_mips then t.max_mips=images.max_mips end

			local max_size=2^images.max_mips

			assert(g:convert(grd.FMT_U8_RGBA_PREMULT)) -- premult force

			if max_size and ( g.width > max_size or g.height>max_size ) then
				local hx=g.width>max_size and max_size or g.width
				local hy=g.height>max_size and max_size or g.height
log("image","Max texture size converting ",g.width,g.height," to ",hx,hy)
				g:scale( hx , hy ,1)
			end

			t.width=g.width
			t.height=g.height
			t.texture_width=images.uptwopow(t.width)
			t.texture_height=images.uptwopow(t.height)

			if t.texture_width~=g.width or t.texture_height~=g.height then 
				g:resize(t.texture_width,t.texture_height,1) -- resize keeping the image in the topleft corner
			end

			t.gl_grd=g -- temp image cache

		end
	end
	
	return t
end

--
-- prepare gl values of a given mip, ready for upload
-- this will probably load a grd
--
images.aloc_gl_data=function(t,_mip)
	local mip=_mip
	local dataname=t.dataname

	if t.mips then

		if mip<4 then mip=4 end -- minimum mip of 4
		if mip>t.mips.mip then mip=t.mips.mip end -- clamp to max mip available
		
		dataname=images.prefix..t.filename..((".%02d"):format(mip))..t.fext
	end
	
	local g=t.gl_grd -- try last loaded cache
	
	if t.fg then -- use an image get function?
		g=t.fg()
		g=grd.create(g) -- take a copy so we can scale it later
	end

	if not g then --
		g=assert(grd.create())
		local d=assert(zips.readfile(dataname),"Failed to load "..dataname)
		assert(g:load_data(d,t.fext))
		if not t.mips then
			assert(g:convert(grd.FMT_U8_RGBA_PREMULT)) -- premult force
		end
	end

--	if not t.mips then -- may need a power of 2 resize from raw texture
		local width=images.uptwopow(g.width)
		local height=images.uptwopow(g.height)

		if width~=g.width or height~=g.height then 
			g:resize(width,height,1) -- resize keeping the image in the topleft corner
		end
---	end

-- perform bad scale fix
	local mmax=2^_mip
	if width > mmax  then width=mmax end
	if height > mmax  then height=mmax end

	if width~=g.width or height~=g.height then
		g:scale(width,height,1) -- resize
	end
	
	t.gl_width=g.width
	t.gl_height=g.height
	t.gl_internal=gl.RGBA
	t.gl_format=gl.RGBA
	t.gl_type=gl.UNSIGNED_BYTE
	
	t.gl_grd=g
	t.gl_data=g.data -- carefulnow
	
	images.gl_mem=images.gl_mem-t.gl_mem
	t.gl_mem=4*t.gl_width*t.gl_height
	images.gl_mem=images.gl_mem+t.gl_mem

end
images.free_gl_data=function(t,mip)
	t.gl_grd=nil
	t.gl_data=nil
end

--
-- load a single image, and make it easy to lookup by the given id
--
images.load=function(filename,id,fg)

--print(debug.traceback())

	local t=images.get(id)

	if t then return t end --first check it is not already loaded


	t={} -- create new
	images.data[id]=t

	t.binds=0
	t.fg=fg
	t.id=id
	t.binds=0 -- count binds, so we can guess which textures are currently in use
	t.mip=4 -- start with a 16x16 base texture size for fast loading?
	t.max_mips=images.max_mips

--print("loading",filename,id)

	if filename and filename:find("%$TONGUE") then -- find and use a $TONGUE replacement token
		
		local fn=filename:gsub("%$TONGUE",wtongues.get())
		if images.exists(fn) then -- try the current langauge
			filename=fn
		else
			filename=filename:gsub("%$TONGUE","english") -- default to english
		end
		
	end

	t.filename=filename
	
	images.prep_image(t)

oven.preloader("load",filename)

	return t
end


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
images.uptwopow=uptwopow


images.bind_and_set_parameters=function(t)

	gl.BindTexture( gl.TEXTURE_2D , t.gl )
	
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,t.TEXTURE_MIN_FILTER or gl.LINEAR_MIPMAP_LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,t.TEXTURE_MAG_FILTER or gl.LINEAR)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,	t.TEXTURE_WRAP_S or 	gl.CLAMP_TO_EDGE)
	gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,	t.TEXTURE_WRAP_T or 	gl.CLAMP_TO_EDGE)

end

images.upload = function(t)

	if not t.gl then
		t.gl=assert(gl.GenTexture())
		t.gl_active=true
		t.gl_mip=0 -- force change
		t.gl_mem=0
	end

	local mip=t.mip
	if t.mips then
		if mip<4 then mip=4 end -- minimum mip of 4
		if mip>t.mips.mip then mip=t.mips.mip end -- clamp to max mip available
	end

	if ( t.gl_mip==mip ) and ( not t.gl_dirty ) then -- no need to change
		return
	end
	t.gl_dirty=false
	
	t.gl_mip=mip
	
	images.bind_and_set_parameters(t)
	
	if t.width==0 or t.height==0 then return t end -- no data to upload

	images.aloc_gl_data(t,t.gl_mip) -- update gl_* params

	if images.flag_panic then
		t.gl_dirty=true -- so this texture will reloaded again later on
	else
		gl.TexImage2D(
			gl.TEXTURE_2D,
			0,
			t.gl_internal,
			t.gl_width,
			t.gl_height,
			0,
			t.gl_format,
			t.gl_type,
			t.gl_data )
	end
	
	if gl.GetError()==gl.OUT_OF_MEMORY then
		images.flag_panic=true
		t.gl_dirty=true -- so this texture will reloaded again later on
	end

	images.free_gl_data(t) -- free gl_* params we may have locked
	
	if not images.flag_panic then
		gl.GenerateMipmap(gl.TEXTURE_2D)
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

	for n,t in pairs(images.data) do
		images.reload(n)
	end

end

images.stop = function()

	for n,t in pairs(images.data) do
		images.unload(n)
	end

end

-- do a gl bind of this texture
images.bind = function(t)
	if t then
		t.binds=t.binds+1
		images.active=true
--		images.make_better(t)
		if not t.gl or t.gl_dirty then
			images.reload(t) -- may need to upload to gl hardware?
		end
		gl.BindTexture( gl.TEXTURE_2D , t.gl or 0 )
	else
		gl.BindTexture( gl.TEXTURE_2D , 0 )
	end
end

images.min_mips=4
images.max_mips=16
-- check stats and adjust the used images to higher rez and unused to lower
images.adjust_mips = function(args)
	args=args or {}

	local best
	local worst

	local total=0

	images.gl_mem=0
	
	local bet
	
	for n,t in pairs(images.data) do

		if t.gl then
			images.gl_mem=images.gl_mem+t.gl_mem
		end
		
		if t.binds==0 then
			t.gl_active=false
		else
			t.gl_active=true
			if t.mip<t.max_mips then -- upgrade
				bet=t
			end
		end
		
	end
	

	if (not bet) and args.force and not images.flag_paniced then -- find another texture
		for n,t in pairs(images.data) do
			if t.mip<t.max_mips then -- upgrade mip of all loaded textures?
				bet=t
				break
			end
		end
	end


	for n,t in pairs(images.data) do -- reset counters
		t.binds=0
	end
	
	if bet then
		images.make_better(bet)
	end

	images.check_panic()
	images.cando_panic()

	images.loadblock=false
end


-- make this texture better (bigger)
images.make_better = function(t)
	if t.mip<t.max_mips then
		t.mip=t.max_mips -- images.max_mips
		images.reload(t.id)
	end
end

-- panic if we have run out of memory, make every active texture worse (lower the highest version)
images.flag_paniced=false
images.flag_panic=false
images.check_panic=function()	


	if gl.GetError()==gl.OUT_OF_MEMORY then
		images.flag_panic=true

-- real memory error
--[[
		local memory_headroom=(16*1024*1024)
		if ( not images.gl_mem_fake_limit ) or images.gl_mem-memory_headroom < images.gl_mem_fake_limit then
-- any memory errors is bad, opengl can easily get itself in a real state
-- so be cautious in the future
			images.gl_mem_fake_limit=images.gl_mem-memory_headroom
print("setting GL fake memory limit to ",images.gl_mem_fake_limit/(1024*1024))
		end
]]
	end

	if images.gl_mem_fake_limit and not images.flag_panic then
		if images.gl_mem>images.gl_mem_fake_limit then
log("image","GL FAKE PANIC mem=",images.gl_mem/(1024*1024))	
			images.flag_panic=true
		end
	end


	return images.flag_panic
end
images.cando_panic=function()
	if images.flag_panic then
--		images.flag_paniced=true -- stop upgrading textures
		return images.panic()
	end
end
images.panic = function(args)

log("image","GL PANIC old memory mem=",images.gl_mem/(1024*1024))	

	images.flag_panic=false
	
	local mild=args and args.mild or false

	for n,t in pairs(images.data) do -- check for mild panic first (can free unused texture)
		if not t.gl_active and t.gl then mild=true end
	end

-- force severe...
mild=false

	if mild then -- mild panic

log("image","GL MILD MEMORY PANIC")

		for n,t in pairs(images.data) do -- unload inactive textures
			if not t.gl_active then
				images.unload(t)
			end
		end
	
	else -- severe panic
	
	

-- first maksure that reducing the max mips will have an effect by setting it to the
-- maximum mip curently used in the active images
		local max_active_mip=1
		for n,t in pairs(images.data) do
			if t.gl_active then
				if t.max_mips>max_active_mip then max_active_mip=t.max_mips end
			end
		end
		if max_active_mip < images.max_mips then images.max_mips=max_active_mip end


		if images.max_mips>1 then -- lower overall quality
			images.max_mips=images.max_mips-1
		end
log("image","GL SEVERE MEMORY PANIC "..images.max_mips.."mips" )

		for n,t in pairs(images.data) do -- unload all textures
			if t.max_mips>images.max_mips then t.max_mips=images.max_mips end
			if t.mip>t.max_mips then t.mip=t.max_mips end
			images.unload(t)
		end

		for n,t in pairs(images.data) do -- reload active textures at new lower rez
			if t.gl_active then
				images.reload(t)
			end
		end
	end

-- recalculate gl_mem
	images.gl_mem=0
	for n,t in pairs(images.data) do
		if t.gl then
			images.gl_mem=images.gl_mem+t.gl_mem
		end
	end
	
log("image","GL PANIC new memory mem=",images.gl_mem/(1024*1024))	
	
	images.check_panic()
	return images.cando_panic()
end

	
	return images
end


