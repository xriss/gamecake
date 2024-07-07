--[[#lua.wetgenes.json_diff

(C) 2024 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

I assume we are not competing in the lua json_diff library world so 
just call it json_diff

	local json_diff=require("wetgenes.json_diff")

When we talk of json objects or json values we mean that the values 
must be valid json. So no storing of functions/etc or mixxing of tables 
and objects. Infinite recursion where data links back into itself is 
also not possible in json so not allowed here.

We assume we have data that could be validly serialised as json but 
this is not enforced. If you want to be 100% safe then convert your 
data to json text and back again before handing it to these functions.

If a table has a length of more than 0 then it is considered a json 
array otherwise it is a json object. An empty table is considered an 
empty object there is no lua equivalent to an empty json array.

Json arrays must be normal lua tables, so first index is 1 not 0 this 
may cause problems if your data is made of objects that have been 
accidently converted into arrays. Probably wont have happened but best 
to be aware of possible sharp edges.

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local json_diff=M

--[[#lua.wetgenes.json_diff.value_dedupe

dedupe a value, after this
]]
json_diff.value_equal=function(a,b)


--[[#lua.wetgenes.json_diff.value_equal

Compare two json values and return true if they are equal, this may 
decend into a tree of tables and objects so can be an expensive test.

]]
json_diff.value_equal=function(a,b)

	if a==b then return true end

	if type(a)=="table" and type(b)=="table" then
		if #a>0 and #b>0 then -- array
			if #a~=#b then return false end -- must be same length
			for i=1,#a do
				if not json_diff.value_equal( a[i] , b[i] ) then return false end
			end
			return true
		else -- object
			local done={} -- checks can be very expensive so only perform once
			for i,v in pairs(a) do
				done[i]=true
				if not json_diff.value_equal( a[i] , b[i] ) then return false end
			end
			for i,v in pairs(b) do
				if not done[i] then -- do not check twice
					if not json_diff.value_equal( a[i] , b[i] ) then return false end
				end
			end
			return true
		end
	end

	return false
end


--[[#lua.wetgenes.json_diff.table_find

Given two tables of json data, return the length , starta , startb of 
the longest common subsequence in table indexes or nil if not similar.

]]
json_diff.table_find=function(a,b,froma,fromb,sizea,sizeb)

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

--[[#lua.wetgenes.json_diff.table_trim

Given two tables of json data, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.

]]
json_diff.table_trim=function(a,b,froma,fromb,sizea,sizeb)

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

--[[#lua.wetgenes.json_diff.table_match

Given two tables of json data, return two synced tables of tables where 
as much json data as possible match in each sub batch.

each of these tables can be concatonated to create the original table.

When stepping thoigh bothe tables, matches are shown by the tables 
having equality ( are both pointer to same table ) and differences will 
be insertions or deletions depending on which of the two has the empty 
table.

EG these two lists

	[a,b,c,d,e,f]
	[a,b,e,f]

would become

	[ [a,b] , [c,d] , [e,f] ]
	[ [a,b] , [] , [e,f] ]

The both [a,b] tables would be equal to each other, eg they are the 
same table so will simply compare as true with an == test. Same with 
the [e,f] tables.

]]
json_diff.table_match=function(a,b,froma,fromb,sizea,sizeb)

	froma=froma or 1
	fromb=fromb or 1

	sizea=sizea or #a
	sizeb=sizeb or #b

	local ra={}
	local rb={}
	
	local append=function(out,data,from,size)
		local t={}
		for i=from,size do t[#t+1]=data[i] end
		out[#out+1]=t
		return t
	end

-- find trim
	local ls,le=json_diff.table_trim(a,b,froma,fromb,sizea,sizeb)
	
-- first trim
	if ls>0 then
		rb[#rb+1]=append(ra,a,starta,starta+ls-1)
	end
	
-- adjust by trim
	froma = froma+ls
	fromb = fromb+ls
	sizea = sizea-le
	sizeb = sizeb-le

	local recurse
	recurse=function(a,b,froma,fromb,sizea,sizeb)

		local length , starta , startb = json_diff.table_find(a,b,froma,fromb,sizea,sizeb)
		
		if length then -- something matched
		
			if starta~=froma or startb~=fromb then
				recurse(a,b,froma,fromb,starta-1,startb-1) -- first bit
			end
			
			rb[#rb+1]=append(ra,a,starta,starta+length-1) -- matching bits tables will be the same so ra[i]==rb[i]

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
		rb[#rb+1]=append(ra,a,sizea+1,sizea+le)
	end


	return ra,rb
end
