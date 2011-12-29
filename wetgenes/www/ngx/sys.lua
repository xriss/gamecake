-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

module(...)


function sleep(...)
	log("sys.sleep:")

--	return core.sleep(...)

end

function file_read(filename)
	local d
	local fp=io.open("html/"..filename,"r")
	if fp then
		d=fp:read("*a")
		fp:close()
	end
	return d
--	return core.file_read(filename)
end

function bytes_split(bytes,size)
	log("sys.bytes_split:")
--	return core.bytes_split(bytes,size)
end
function bytes_join(tab)
	return table.concat(tab)
--	return core.bytes_join(tab)
end

function bytes_to_string(bytes)
	return bytes
--	return core.bytes_to_string(bytes)
end

function bin_encode(t,b)
	log("sys.bin_encode:")
--	return core.bin_encode(t,b)
end

function md5(s,f)
	log("sys.md5:")
--	return core.md5(s,f)
end
function sha1(s,f)
	log("sys.sha1:")
--	return core.sha1(s,f)
end
function hmac_sha1(k,s,f)
	log("sys.hmac_sha1:")
--	return core.hmac_sha1(k,s,f)
end

function zip_list(z)
	log("sys.zip_list:")
--	return core.zip_list(z)
end
function zip_read(z,n)
	log("sys.sleep:")
--	return core.zip_read(z,n)
end

-----------------------------------------------------------------------------
--
-- check if the given file exists
--
-----------------------------------------------------------------------------
function file_exists(filename)

	local f=io.open("html/"..filename,"r")
	if f then
		f:close()
		return true
	end

	return false
end

-----------------------------------------------------------------------------
--
-- find a given lua file, within the lua path
-- do not pass in the .lua extension
-- returns a path to a file you can then open
--
-----------------------------------------------------------------------------
function file_find_lua(name)

	return "../lua/"..name..".lua"

end


-----------------------------------------------------------------------------
--
--
-----------------------------------------------------------------------------
function redirect(srv,url)

	if not srv.redirect(url) then -- header write failed, spit out some java script instead?
	
		srv.put([[<script type="text/javascript" >window.location = "]]..url..[[";</script>]])
		
	end

end

