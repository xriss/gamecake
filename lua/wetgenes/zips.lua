--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- handle zip files containing lua code and data

local zip=require("zip")
local wsandbox=require("wetgenes.sandbox")
local apps ; pcall( function() apps=require("apps") end )
local core ; pcall( function() core=require("wetgenes.gamecake.core") end )


module("wetgenes.zips")

files={} -- zip files for loader to search

--dirs={} -- dirs to check in file system

-- prefix to use when io.opening 
paths={"",apps and apps.find_bin()}


-- convert a sensible name into something we can store in an apk
-- no directorys and most chars are illegal, we just convert them to _
-- and look in the raw directory, this is a one way destructive function.
-- the returned filename can then be checkedfor in the apk
function apk_munge_filename(s)

	local r=string.lower(s)

	r=string.gsub(r,"([^a-z0-9%_]+)","_") -- replace most everything with an underscore

	return "res/raw/"..r
end

--
-- add a zipfile to the end of places to search and return it
-- so we can add more options ifwe need to
--
function add_zip_file(fname,t)
	t=t or {}
	
	t.fname=fname
	t.z=zip.open(fname) -- open the file for later use
		
	files[#files+1]=t
	return t
end
--
-- Like add zip but with a munger as apk files are retarded
--
function add_apk_file(fname,t)
	t=t or {}
	t.munge=apk_munge_filename
	return add_zip_file(fname,t)
end
--
-- add a zipfile (already read into memory) to the end of places to search and return it
-- so we can add more options if we need to
--
function add_zip_data(mem,t)
	t=t or {}
	
	t.fname="**data**"
	t.mem=mem -- do not garbagecollect this value...
	t.z=zip.open_mem(mem) -- the file exists in memory only
		
	files[#files+1]=t
	return t
end

--
-- this is inserted into the package loaders to load modules from zip
--
function loader(...)

	if not files[1] then return "\n\tno zips : no files to search" end

	local args={...}
	local name=args[1]

	if not name then return "\n\tno zips : no module name" end -- sanity

	local sname=name:gsub("%.","/") -- replace . with /
	local fnames={ "lua/"..sname..".lua" , "lua/"..sname.."/init.lua" }

	for i,v in ipairs(files) do

		if v.z then -- this is the pre opened zipfile
		
			for i,fname in ipairs(fnames) do
				fname=(v.munge and v.munge(fname)) or fname -- munge the filename (apk), or just use as is?
				local file=v.z:open(fname)
				if file then
					local str=file:read("*a")
					file:close()
					if str:sub(1,2)=="#!" then
						str="--"..str -- ignore hashbang on first line
					end
					local func,err=assert(loadstring(str,name))
					return func or err
				end
			end
			
		end

	end

	return "\n\tno zips '"..name.."'" -- not found

end

--
-- open the given filename
--
function open(fname)
	for i,v in ipairs(files) do
		if v.z then -- this is the pre opened zipfile
			local mname=(v.munge and v.munge(fname)) or fname -- munge the filename (apk), or just use as is?
			local file=v.z:open(mname)
			if file then
				return file
			end
		end
	end
	local ret

	for i,ioprefix in ipairs(paths) do
		ret=io.open(ioprefix..fname,"rb")
		if ret then return ret end
	end

	return ret
end


--
-- read the entire file and return the data
--
function readfile(fname)
	local f=open(fname)
	if f then
		local d=f:read("*a")
		f:close()
		return d
	end
	
	if core then -- final attempt at getting a cached string
		return core.get_cache_string(fname)
	end
end

--
-- read the entire file and return the data
--
function readlson(fname)
	local d=readfile(fname)
	if d then
		return wsandbox.lson(d) -- decode
	end
end

--
-- returns true if a file exists
--
function exists(fname)
	local f=open(fname)
	if f then
		f:close()
		return true
	end
	return false
end


--
-- get a directory listing of the gicen dirname
-- we must merge all possible zips with a first come priority to files
-- apk are special as we have munged filenames in a flat filesystem
-- so we have to treat the dname as a prefix
--
-- this is currently impossible, until we update the zip wossname
--
function dir(dname)
	local ret={}
	local found=false
	
	local mname=(v.munge and v.munge(fname)) or fname -- munge the filename (apk), or just use as is?

	for i,v in ipairs(files) do
		if v.z then -- this is the pre opened zipfile
--			local file=v.z:open(mname)
		end
	end
	
	return found and ret
end

