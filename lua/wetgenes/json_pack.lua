--[[#lua.wetgenes.json_pack

(C) 2024 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local json_pack=require("wetgenes.json_pack")

Turn a valid json table into a binary data string and visa versa.

A valid json table...

	Does not include functions/userdata/etc.
	
	Does not self reference.
	
	Each table should be an array or an object never both.
	
	May have metatables as long as it is not too funky so added 
	function calls for a class should be fine but this will obviously 
	be lost. Please test and check outputs if including meta.

So normal json style data.


Currently we are using cmsgpack and zlib, this may be changed if a 
better way is found.

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local json_pack=M

local cmsgpack=require("cmsgpack")
local zlib=require("zlib")
local zip_inflate=function(d) return ((zlib.inflate())(d)) end
local zip_deflate=function(d) return ((zlib.deflate())(d,"finish")) end
local   compress=function(it) return zip_deflate(cmsgpack.pack(it))   end
local decompress=function(it) return cmsgpack.unpack(zip_inflate(it)) end


--[[#lua.wetgenes.json_pack.into_data

	d=json_pack.into_data(j)

Where j is some json data and d is a binary string of the same data.

]]
json_pack.into_data=function(j)
	local d=compress(j)
	return d
end

--[[#lua.wetgenes.json_pack.from_data

	j=json_pack.from_data(d)

Where j is some json data and d is a binary string of the same data.

]]
json_pack.from_data=function(d)
	local j=decompress(d)
	return j
end

