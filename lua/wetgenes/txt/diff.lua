--[[#lua.wetgenes.txt.diff

(C) 2020 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local wtxtdiff=require("wetgenes.txt.diff")
  
]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M



--[[#lua.wetgenes.txt.diff.split

Use the delimiter to split a string into a table of strings such that 
each string ends in the delimiter (except for possibly the final string) and a 
table.concat on the result will recreate the input string exactly.

	table = wtxtdiff.split(string,delimiter)

String is the string to split and delimiter is a lua pattern so any 
special chars should be escaped.

for example

	st = wtxtdiff.split(s) -- split on newline (default)
	st = wtxtdiff.split(s,"\n") -- split on newline (explicit)

	st - wtxtdiff.split(s,"%s+") -- split on white space

]]
M.split=function(s,d)
	d=d or "\n"
	local ss={} -- output table
	local ti=1  -- table index
	local si=1  -- string index
	while true do
		local fa,fb=string.find(s,d,si) -- find delimiter
		if fa then
			ss[ti]=string.sub(s,si,fb) -- add string to table
			ti=ti+1
			si=fb+1
		else break end -- no more delimiters
	end
	if string.sub(s,si) ~= "" then -- is there some string remaining?
		ss[ti]=string.sub(s,si)
	end
	return ss
end



--[[#lua.wetgenes.txt.diff.find

Given two tables of strings, return the length , starta , startb of the longest 
common subsequence in table indexes or nil if not similar.


]]
M.find=function(a,b,froma,fromb,sizea,sizeb)

	froma=froma or 1
	fromb=fromb or 1

	sizea=sizea or #a
	sizeb=sizeb or #b

	local starta=1
	local startb=1
	local length=0

-- maximum remaining indexs to check ( best possible length )
	local remain=function(ia,ib) local ra=1+sizea-ia local rb=1+sizeb-ib return ra>rb and ra or rb end

	for ia=froma,sizea do
	
		local sa=ia
		local sb=fromb
		local l=0

		if remain(sa+l,sb+l) <= length then break end -- can't possibly be better

		for ib=fromb,sizeb do

			sb=ib
			l=0
			
			if remain(sa+l,sb+l) < length then break end -- can't possibly be better

			while true do

				if ( remain(sa+l,sb+l) <= 0 ) or ( a[sa+l] ~= b[sb+l] ) then -- ended
					if l > length then -- found a longer string
						starta=sa
						startb=sb
						length=l
					end
					break
				else
					l=l+1
				end
			end

		end
	end

	if length>0 then return length , starta , startb end
	return nil
end

--[[#lua.wetgenes.txt.diff.trim

Given two tables of strings, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.


]]
M.trim=function(a,b,froma,fromb,sizea,sizeb)

	froma=froma or 1
	fromb=fromb or 1

	sizea=sizea or #a
	sizeb=sizeb or #b

-- the smallest size
	local size = ( sizea < sizeb ) and sizea or sizeb
	
	local ls=0
	for d=0,size-1 do
		if a[froma+ls] == b[fromb+ls] then ls=ls+1
		else break end
	end

-- the smallest remaining size
	local size = ( (sizea-ls) < (sizeb-ls) ) and (sizea-ls) or (sizeb-ls)

	local le=0
	for d=0,size-1 do
		if a[sizea-le] == b[sizeb-le] then le=le+1
		else break end
	end

	return ls,le
end

--[[#lua.wetgenes.txt.diff.match

Given two tables of strings, return two tables of strings of 
the same length where as many strings as possible match.

]]
M.match=function(a,b,froma,fromb,sizea,sizeb)

	froma=froma or 1
	fromb=fromb or 1

	sizea=sizea or #a
	sizeb=sizeb or #b

	local ra={}
	local rb={}
	
	local append=function(out,data,from,size)
		local t={}
		for i=from,size do t[#t+1]=data[i] end
		out[#out+1]=table.concat(t)
	end

	
-- find trim
	local ls,le=M.trim(a,b,froma,fromb,sizea,sizeb)
	
-- first trim
	if ls>0 then
		append(ra,a,starta,starta+ls-1)
		append(rb,b,startb,startb+ls-1)
	end
	
-- adjust by trim
	froma = froma+ls
	fromb = fromb+ls
	sizea = sizea-le
	sizeb = sizeb-le

	local recurse
	recurse=function(a,b,froma,fromb,sizea,sizeb)

		local length , starta , startb = M.find(a,b,froma,fromb,sizea,sizeb)
		
		if length then -- something matched
		
			if starta~=froma or startb~=fromb then
				recurse(a,b,froma,fromb,starta-1,startb-1) -- first bit
			end
			
			append(ra,a,starta,starta+length-1) -- matching bits both of
			append(rb,b,startb,startb+length-1) -- these should be the same

			if starta+length~=sizea or startb+length~=sizeb then
				recurse(a,b,starta+length,startb+length,sizea,sizeb) -- last bit
			end

		else -- nothing matched so insert joined data to return table
		
			append(ra,a,froma,sizea)
			append(rb,b,fromb,sizeb)

		end

	end
	recurse(a,b,froma,fromb,sizea,sizeb)

-- last trim
	if le>0 then
		append(ra,a,sizea+1,sizea+le)
		append(rb,b,sizeb+1,sizeb+le)
	end


	return ra,rb
end
