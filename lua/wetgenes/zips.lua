--
-- (C) 2013 Kriss@XIXs.com
--

-- handle zip files containing lua code and data

local zip=require("zip")
local wsandbox=require("wetgenes.sandbox")
local apps ; pcall( function() apps=require("apps") end )
local core ; pcall( function() core=require("wetgenes.gamecake.core") end )


--module("wetgenes.zips")


local M={ modname = (...) } package.loaded[M.modname] = M 


M.files={} -- zip files for loader to search

--dirs={} -- dirs to check in file system

-- prefix to use when io.opening 
M.paths={"",apps and apps.find_bin()}


-- uses assets/ dir now so no need to mangle
M.apk_munge_filename = function (s)
	return "assets/"..string.lower(s)
end

--
-- add a zipfile to the end of places to search and return it
-- so we can add more options ifwe need to
--
M.add_zip_file = function (fname,t)
	t=t or {}
	
	t.fname=fname
	t.z=assert( zip.open(fname) )-- open the file for later use
		
	M.files[#M.files+1]=t
	return t
end
--
-- Like add zip but with a munger as apk files are retarded
--
M.add_apk_file = function (fname,t)
	t=t or {}
	t.munge=M.apk_munge_filename
	return M.add_zip_file(fname,t)
end
--
-- add a zipfile (already read into memory) to the end of places to search and return it
-- so we can add more options if we need to
--
M.add_zip_data = function (mem,t)
	t=t or {}
	
	t.fname="**data**"
	t.mem=mem -- do not garbagecollect this value...
	t.z=assert( zip.open_mem(mem) ) -- the file exists in memory only
		
	M.files[#M.files+1]=t
	return t
end

--
-- this is inserted into the package loaders to load modules from zip
--
M.loader = function (...)

	if not M.files[1] then return "\n\tno zips : no files to search" end

	local args={...}
	local name=args[1]

	if not name then return "\n\tno zips : no module name" end -- sanity

	local sname=name:gsub("%.","/") -- replace . with /
	local fnames={ "lua/"..sname..".lua" , "lua/"..sname.."/init.lua" }
	
	for i=#M.files,1,-1 do -- last added zip has priority when searching
		local v=M.files[i]

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

	return "\n\tno zips '"..name.."'"
end

--
-- open the given filename
--
M.open = function (fname)
	for i,v in ipairs(M.files) do
		if v.z then -- this is the pre opened zipfile
			local mname=(v.munge and v.munge(fname)) or fname -- munge the filename (apk), or just use as is?
			local file=v.z:open(mname)
			if file then
				return file
			end
		end
	end
	local ret

	for i,ioprefix in ipairs(M.paths) do
		ret=io.open(ioprefix..fname,"rb")
		if ret then return ret end
	end

	return ret
end


--
-- read the entire file and return the data
--
M.readfile = function (fname)
	local f=M.open(fname)
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
M.readlson = function (fname)
	local d=M.readfile(fname)
	if d then
		return wsandbox.lson(d) -- decode
	end
end

--
-- returns true if a file exists
--
M.exists = function (fname)
	local f=M.open(fname)
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
M.dir = function (dname)
	local ret={}
	local found=false
	
	local mname=(v.munge and v.munge(fname)) or fname -- munge the filename (apk), or just use as is?

	for i,v in ipairs(M.files) do
		if v.z then -- this is the pre opened zipfile
--			local file=v.z:open(mname)
		end
	end
	
	return found and ret
end



-- try and add lua.zip if it exists
--[[
if exists("lua.zip") then
print("mounting lua.zip")
	add_zip_file("lua.zip")
end
]]
