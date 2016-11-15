--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local wpack=require("wetgenes.pack")

local wwin=require("wetgenes.win")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

--[==[

-- handle the mapping of images to stashed files on disk
-- this allows faster loading after their initial creation
-- their creation/resizeing and convertion to other formats as we run low on memory

function M.bake(oven,stash)
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
	
	local codestamp=(opts.bake and opts.bake.stamp)+1
	
	local images=oven.rebake("wetgenes.gamecake.images")

	stash.data={}
	
-- keep the total amount of memory used to under this, nil for no limit
	stash.max_memory=nil

-- force images to be no more than this many pixels wide or high, first pass resize
	stash.max_texture_size=opts.max_texture_size or 2^11 -- (2048 default?)
	
-- convert to this many bits per pixel, GL compression is a bit of a mess but 16bit reduction is a simple start
	stash.bpp=32
	
--	stash.prefix=opts.grdprefix or "data/"
	
--
-- do we have a raw image availble to load, return its full filename with extension if we do
--
stash.find_image=function(filename)

	local fname
	local fext

	for i,v in ipairs( { ".png" , ".jpg" } ) do
		fext=v
		fname="data/"..filename..fext -- hardcode
		if zips.exists(fname) then  return fname end -- found it
	end

end

--
-- get the stash data for this filename, may involve loading and converting a file
--
stash.filename_to_id=function(filename)
	return string.gsub( string.lower(filename) ,"([^a-z0-9%_]+)","_")
end


stash.get_data=function(it, mip)

	local m=it.mips[mip]
	local d
	local f=stash.open(wwin.cache_prefix..it.id..".img")
	if f then
		f:seek("set",m.offset)
		d=f:read(m.size)
		f:close()
	end
	
	if d then return d end
	
	local t,d=stash.build_image(it.filename,it.fg)
	it.stash=t
	
	return d:sub(m.offset+1,m.offset+m.size-1)
end


--
-- get the stash meta data for this filename, may involve loading and converting a file
--
stash.get_image=function(filename,fg)

--	if not opts.imagestash then return end

	local id=stash.filename_to_id(filename)
		
	if stash.data[id] then return stash.data[id] end -- preloaded meta information so just return it

	do -- check for prebuilt metainfo
		local d=stash.readfile(wwin.cache_prefix..id..".lua") -- load and return data
		if d then
			d=wsbox.lson(d)
			if d then
				if d.stamp == codestamp then -- must be uptodate
					stash.data[id]=d
					d.fg=fg
					return stash.data[id]
				end
			end
		end
	end

	return stash.build_image(filename,fg)
end
	
stash.build_image=function(filename,fg)
	local g
	local id=stash.filename_to_id(filename)


-- create new stash file
	local t={}
	stash.data[id]=t
	
	t.stamp=codestamp
	
	t.id=id
	t.filename=filename
--	t.fg=fg

	if fg then g=fg() end
	if not g then -- load or use what is passed in
		t.dataname=stash.find_image(filename)
		g=assert(grd.create())
		local d=assert(zips.readfile(t.dataname),"Failed to load "..t.dataname)
		assert(g:load_data(d,filename:sub(-3))) -- last 3 letters pleaze

	-- clamp to a maximum size
		local max_size=2^images.max_mips
--		local max_size=1024

	-- tempory fixes to reduce basetexture memory footprint
	-- need to do this smarter

		if wwin.flavour=="android" then
			if images.max_mips>6 then
				images.max_mips=6
				max_size=2^images.max_mips
			end
		end
		
		if wwin.flavour=="raspi" then
			if images.max_mips>10 then
				images.max_mips=10
				max_size=2^images.max_mips
			end
		end

		if max_size and ( g.width > max_size or g.height>max_size ) then
			local hx=g.width>max_size and max_size or g.width
			local hy=g.height>max_size and max_size or g.height
print("Max texture size converting ",g.width,g.height," to ",hx,hy)
			g:scale( hx , hy ,1)
		end

	end
	assert(g:convert(grd.FMT_U8_RGBA_PREMULT))

-- force powah of 2 sizes, so we can mipmap

	t.width=g.width
	t.height=g.height
	t.texture_width=images.uptwopow(t.width)
	t.texture_height=images.uptwopow(t.height)

	if t.texture_width~=g.width or t.texture_height~=g.height then 
		g:resize(t.texture_width,t.texture_height,1) -- resize keeping the image in the topleft corner
	end

	t.texture_width=( t.texture_width > stash.max_texture_size )  and stash.max_texture_size or t.texture_width
	t.texture_height=( t.texture_height > stash.max_texture_size ) and stash.max_texture_size or t.texture_height
	g:scale(t.texture_width,t.texture_height,1)

	
	t.gl_width=t.texture_width
	t.gl_height=t.texture_height
	t.gl_internal=gl.RGBA
	t.gl_format=gl.RGBA
	t.gl_type=gl.UNSIGNED_BYTE


	local lson=zips.readfile(images.prefix..filename..".lua")
	if lson then -- check for lua metadata
		t.meta=wsbox.lson(lson) -- return it with the image
	end

	t.format=grd.FMT_U8_RGBA_PREMULT
	t.format_size=4

	local dats={}
	t.mips={}
	for i=16,1,-1 do -- build small to large so we can trim the large images or add more if we need to
		local ms=2^i
		local lm=t.mips[i+1]
		local m={}
		t.mips[i]=m
		
		m.width= ( t.texture_width  > ms ) and ms or t.texture_width
		m.height=( t.texture_height > ms ) and ms or t.texture_height

-- force a minimum texture size of 8x8
		m.width= ( m.width  < 8 ) and 8 or m.width
		m.height=( m.height < 8 ) and 8 or m.height

		m.size=m.width*m.height*t.format_size
		m.sizeup=math.ceil(m.size/16)*16 -- make sure each mip is a multiple of 16 bytes?
		m.offset=0
		
		if not lm or lm.size~=m.size then -- need to make new mip
			g:scale(m.width,m.height,1)
			dats[(i-1)*2+1]=wpack.tostring(g.data,m.size)
			dats[(i-1)*2+2]=""
			if m.size<m.sizeup then
				dats[(i-1)*2+2]=string.rep(string.char(0),m.sizeup-m.size) -- padding
			end
		else
			dats[(i-1)*2+1]=""
			dats[(i-1)*2+2]=""
		end
		
	end
	dats=table.concat(dats)

	local offset=0
	local last=0
	for i=1,16 do
		local m=t.mips[i]
		local lm=t.mips[i-1]
		
		if lm and lm.size~=m.size then -- new mip so step through file
			offset=offset+lm.sizeup
		end
		
		m.offset=offset -- remember possition in file
	end

print("building stash for "..id,#dats)
--print(id.." : "..wstr.dump(t))

--	stash.writefile(wwin.cache_prefix..id..".img",dats)
	
	stash.writefile(wwin.cache_prefix..id..".lua",wstr.serialize(t))
		
	return t,dats
end

--
-- open the given filename
--
function stash.open(fname,mode)
	return io.open(fname,mode or "rb")
end

--
-- write data to a file
--
function stash.writefile(fname,data)
	local f=stash.open(fname,"wb")
	if f then
		local d=f:write(data)
		f:close()
		return true
	end
	return nil,"failed to writefile \""..fname.."\""
end

--
-- read the entire file and return the data
--
function stash.readfile(fname)
	local f=stash.open(fname)
	if f then
		local d=f:read("*a")
		f:close()
		return d
	end
	return nil,"failed to readfile \""..fname.."\""
end

--
-- returns true if a file exists
--
function stash.exists(fname)
	local f=stash.open(fname)
	if f then
		f:close()
		return true
	end
	return false
end




	return stash
end


]==]

