-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log
local ngx=require("ngx")

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
	
	local t={}
	for i=1,#bytes,size do
		t[#t+1]=bytes:sub(i,i+size-1)
	end
	return t
end

function bytes_join(tab)
	return table.concat(tab)
end

function bytes_to_string(bytes)
	return bytes
end

function bin_encode(enc,s)
	log("sys.bin_encode:")

	if enc=="hex" then
		r=str_to_hex(s)
	elseif enc=="base64" then
		r=b64_enc(s)
	else
		r=s
	end
	
	return r 
--	return core.bin_encode(t,b)
end

function md5(s,t)
	log("sys.md5:")
	if f=="bin" then return ngx.md5_bin(s) end
	return ngx.md5(s)
end
function sha1(s,f)
	log("sys.sha1:")
--	if f=="bin" then return ngx.sha1_bin(s) end
--	return ngx.sha1(s)
--	return core.sha1(s,f)
end
function hmac_sha1(k,s,f)
	log("sys.hmac_sha1:")
	if f=="bin" then return ngx.hmac_sha1(k,s) end
	return ngx.hmac_sha1(k,s)
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
-- convert a string into a hex string
--
-----------------------------------------------------------------------------
function str_to_hex(s)
	return string.gsub(s, ".", function (c)
		return string.format("%02x", string.byte(c))
	end)
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



-- character table string
local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- encoding
function b64_enc(data)
    return ((data:gsub('.', function(x) 
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r;
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return b:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end

-- decoding
function b64_dec(data)
    data = string.gsub(data, '[^'..b..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(b:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
        return string.char(c)
    end))
end
