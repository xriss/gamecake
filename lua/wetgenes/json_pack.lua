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


do

local math_abs=math.abs
local math_log=math.log
local math_floor=math.floor
local string_byte=string.byte

-- return a "+" b where b is bitfiddled and a becomes an acumilated checksumish
local chksumish_number=function(a,b)
	if a<0x800000000000 then a=a*2 end -- do a little shift on a if we are less than a 48bit integer
	local b=math_abs(b) -- negative numbers are confuzling
	local e=math_log(b) -- this is not power of 2 but close enough for our ishymath
	local c=math_floor( b/(2^(e-46)) ) -- c is now probably a rather large 48bit number
	if c~=c then c=0 end -- bad nan
	return math_abs(a-c) -- subtract from a so we can hopefully shift again soon and abs it so we stay positive and xorish maybe
end

local chksumish_bool=function(a,b)
	return chksumish_number(a,b and 1 or 3)
end

local chksumish_string=function(a,b)
	for i=1,#b,6 do
		local c=	string_byte(b,i  )*0x10000000000 +
					string_byte(b,i+1)*0x100000000   +
					string_byte(b,i+2)*0x1000000     +
					string_byte(b,i+3)*0x10000       +
					string_byte(b,i+4)*0x100         +
					string_byte(b,i+5)
		if a<0x800000000000 then a=a*2 end -- do a little shift on a if we are less than a 48bit integer
		a=math_abs(a-c) -- subtract from a so we can hopefully shift again soon and abs it so we stay positive and xorish maybe
	end
	return a
end

local chksumish ; chksumish=function(a,b)
	local t=type(b)
	if t=="number" then return chksumish_number(a,b) end
	if t=="string" then return chksumish_string(a,b) end
	if t=="table" then
		if type(b[1])~="nil" then -- array
			for n,v in ipairs(b) do
				a=chksumish(a,v)
			end
		else -- object must iterate in a stable way so...
			local ns={}
			for n,v in pairs(b) do ns[#ns+1]=n end
			table.sort(ns)
			for _,n in pairs(ns) do
				a=chksumish(a,b[n])
			end
		end
		return a
	end
	return chksumish_bool(a,b)
end


json_pack.chksumish_number = chksumish_number
json_pack.chksumish_bool   = chksumish_bool
json_pack.chksumish_string = chksumish_string
json_pack.chksumish        = chksumish

end

