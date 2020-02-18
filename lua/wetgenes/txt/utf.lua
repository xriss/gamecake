--[[#lua.wetgenes.txt.utf

(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

	local wutf = require("wetgenes.txt.utf")

helper functions to help manage a string as a stream of utf8 tokens.

]]

-- locals
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[wutf.modname]=M
local wutf=M

--[[#lua.wetgenes.txt.utf.charpattern

	string:gmatch(wutf.charpattern)

lua pattern to match each utf8 character in a string

]]
wutf.charpattern="([%z\1-\127\194-\244][\128-\191]*)"

--[[#lua.wetgenes.txt.utf.map_unicode_to_latin0

	latin0 = wutf.map_unicode_to_latin0[unicode] or unicode

I prefer the coverage of latin0 (ISO/IEC 8859-15) for font layout as it 
is just a few small differences for western european languages to get 
most needed glyphs into the first 256 codes.

]]
wutf.map_unicode_to_latin0={
	[0x20AC]=0xa4,	[0x0160]=0xa6,	[0x0161]=0xa8,	[0x017d]=0xb4,
	[0x017e]=0xb8,	[0x0152]=0xbc,	[0x0153]=0xbd,	[0x0178]=0xbe,
}
--[[#lua.wetgenes.txt.utf.map_latin0_to_unicode

	unicode = wutf.map_latin0_to_unicode[latin0] or latin0

]]
wutf.map_latin0_to_unicode={
	[0xa4]=0x20AC,	[0xa6]=0x0160,	[0xa8]=0x0161,	[0xb4]=0x017d,
	[0xb8]=0x017e,	[0xbc]=0x0152,	[0xbd]=0x0153,	[0xbe]=0x0178,
}

--[[#lua.wetgenes.txt.utf.string

	unicode = wutf.code(string,index)

get the utf8 value at the given byte index.


	unicode = wutf.code(string)

get the utf8 value at the start of this string

]]
wutf.code=function(s,idx)
	idx=idx or 1 -- default to start of string

	local code=nil -- invalid code if nothing below matches
	local mul=0   -- multi bytes

	local b=s:byte( idx )
	if     b< 0x80 then code=b            -- 1 byte
	elseif b< 0xe0 then code=b-0xc0 mul=1 -- 2 byte
	elseif b< 0xf0 then code=b-0xe0 mul=2 -- 3 byte
	elseif b<=0xf4 then code=b-0xf0 mul=3 -- 4 byte
	end

	for i = idx+1 , idx+mul do -- add requested multi bytes
		local b=s:byte(i)
		if (not b) or (b < 0x80) or (b > 0xbf) then return nil end -- invalid so return nil
		code=code*0x40 + (b-0x80)
	end

	return code -- this may be nil
end

--[[#lua.wetgenes.txt.utf.size

	size = wutf.size(string,index)

get the size in bytes of the utf8 value at the given byte index.

	size = wutf.size(string)

get the size in bytes of the utf8 value at the start of this string

The return value will be 1-4 as 4 is the biggest utf8 code size.

]]
wutf.size=function(s,idx)
	idx=idx or 1 -- default to start of string
	local b=s:byte( idx )
	if     b< 0x80 then return 1 -- 1 byte
	elseif b< 0xe0 then return 2 -- 2 byte
	elseif b< 0xf0 then return 3 -- 3 byte
	elseif b<=0xf4 then return 4 -- 4 byte
	end
end

--[[#lua.wetgenes.txt.utf.char

	string = wutf.char(number)

convert a single unicode value to a utf8 string of 1-4 bytes

]]
wutf.char=function(code)
	if     code <=     0x7F then return string.char(
		                  code                     )
	elseif code <=    0x7FF then return string.char(
		0xC0 + math.floor(code/0x40)               ,
		0x80 +           (code     )%0x40          )
	elseif code <=   0xFFFF then return string.char(
		0xE0 + math.floor(code/0x1000)             ,
		0x80 + math.floor(code/0x40  )%0x40        ,
		0x80 +           (code       )%0x40        )
	elseif code <= 0x10FFFF then return string.char(
		0xF0 + math.floor(code/0x40000)            ,
		0x80 + math.floor(code/0x1000 )%0x40       ,
		0x80 + math.floor(code/0x40   )%0x40       ,
		0x80 +           (code        )%0x40       )
	end
	return ""
end

--[[#lua.wetgenes.txt.utf.chars

	string = wutf.chars(number,number,...)
	string = wutf.chars({number,number,...})

convert one or more unicode values into a utf8 string

]]
wutf.chars=function(...)
	local t={...} -- multiargs
	if type(t[1])=="table" then t=t[1] end -- or single table
	for i=1,#t do t[i]=wutf.char(t[i]) end
	return table.concat(t)
end

--[[#lua.wetgenes.txt.utf.length

	length = wutf.length(string)

get the length in codes of this string

]]
wutf.length=function(s)
	local l=0
	for char in s:gmatch(wutf.charpattern) do l=l+1 end
	return l
end

--[[#lua.wetgenes.txt.utf.length

	unicode = wutf.ncode(string,index)

get the utf8 value at the given code index.

Note that this is slower than wutf.code as we must search the string to 
find the byte index of the code. 

]]
-- get the nth code from the string (slow)
wutf.ncode=function(s,n)
	local l=0
	for char in s:gmatch(wutf.charpattern) do
		l=l+1
		if l==n then return wutf.code(char) end
	end
end

