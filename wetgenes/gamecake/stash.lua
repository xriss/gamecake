-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local grd=require("wetgenes.grd")
local zips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local wpack=require("wetgenes.pack")

local wwin=require("wetgenes.win")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-- handle the mapping of images to stashed files on disk
-- this allows faster loading after their initial creation
-- their creation/resizeing and convertion to other formats as we run low on memory

function M.bake(oven,stash)
	
	local opts=oven.opts
	local cake=oven.cake
	local gl=oven.gl
	
	stash.data={}
	
-- keep the total amount of memory used to under this, nil for no limit
	stash.max_memory=nil

-- force images to be no more than this many pixels wide or high
	stash.max_size=2048
	
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
	return string.lower( string.gsub(filename,"([^a-z0-9%_]+)","_") )
end

--
-- get the stash data for this filename, may involve loading and converting a file
--
stash.get_image=function(filename)

	do return end

	local id=stash.filename_to_id(filename)
	
	if stash.data[id] then return stash.data[id] end -- preloaded meta information, just return it

	local d=stash.readfile("cache/"..id..".lua") -- load and return data
	if d then
		stash.data[id]=wsbox.lson(d)
		return stash.data[id]
	end
	
-- create new stash file
	local t={}
	stash.data[id]=t
	
	t.id=id
	t.filename=filename
	t.dataname=stash.find_image(filename)

	local g=assert(grd.create())
	local d=assert(zips.readfile(t.dataname),"Failed to load "..t.dataname)
	assert(g:load_data(d,filename:sub(-3))) -- last 3 letters pleaze
	assert(g:convert(grd.FMT_U8_RGBA_PREMULT))

print(id.." : "..wstr.dump(g))

	t.format=grd.FMT_U8_RGBA_PREMULT
	t.width=g.width
	t.height=g.height

	stash.writefile("cache/"..id..".img",wpack.tostring(g.data,g.width*g.height*4))
	
	stash.writefile("cache/"..id..".lua",wstr.serialize(t))
		
	return t
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


