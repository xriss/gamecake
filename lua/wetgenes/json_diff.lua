--[[#lua.wetgenes.json_diff

(C) 2024 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

I assume we are not competing in the lua json_diff library world so 
just call it json_diff

	local json_diff=require("wetgenes.json_diff")

When we talk of json objects or json values we mean that the values 
must be valid json. So no storing of functions/etc or mixxing of tables 
and objects. Infinite recursion where data links back into itself is 
also not possible in json so not allowed/expected here.

We assume we have data that could be validly serialised as json but 
this is not enforced. If you want to be 100% safe then convert your 
data to json text and back again before handing it to these functions.

If a table has a length of more than 0 then it is considered a json 
array otherwise it is a json object. An empty table is considered an 
empty object there is no lua equivalent to an empty json array.

Lua is not good with holes in its arrays, so that is also not suported.

We do try and diff arrays of objects, but, since arrays may shift up 
and down it would be better if you had object maps of id to data and 
arrays of ids that references these maps rather than arrays of objects.

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


--[[#lua.wetgenes.json_diff.array_common

Given two arrays of json data, return the length , starta , startb of 
the longest common subsequence in table indexes or nil if not similar.

]]
json_diff.array_common=function(a,b,froma,fromb,sizea,sizeb)

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

				if ( remain(sa+l,sb+l) <= 0 ) or not json_diff.equal( a[sa+l] , b[sb+l] ) then -- ended
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

--[[#lua.wetgenes.json_diff.array_trim

Given two arrays of json data, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.

]]
json_diff.array_trim=function(a,b,froma,fromb,sizea,sizeb)

	froma=froma or 1
	fromb=fromb or 1

	sizea=sizea or #a
	sizeb=sizeb or #b

-- the smallest size
	local size = ( sizea < sizeb ) and sizea or sizeb
	
	local ls=0
	for d=0,size-1 do
		if json_diff.equal( a[froma+ls] , b[fromb+ls] ) then ls=ls+1
		else break end
	end

-- the smallest remaining size
	local size = ( (sizea-ls) < (sizeb-ls) ) and (sizea-ls) or (sizeb-ls)

	local le=0
	for d=0,size-1 do
		if json_diff.equal( a[sizea-le] , b[sizeb-le] ) then le=le+1
		else break end
	end

	return ls,le
end

--[[#lua.wetgenes.json_diff.array_match

Given two arrays of json data, return two synced arrays of arrays where 
as much json data as possible match in each sub batch.

each of these arrays can be concatonated to create the original array.

When stepping thoigh bothe arrays, matches are shown by the sub array 
having equality and differences will be insertions or deletions 
depending on which of the two has the empty array.

EG these two lists

	[a,b,c,d,e,f]
	[a,b,e,f]

would become

	[ [a,b] , [c,d] , [e,f] ]
	[ [a,b] , [] , [e,f] ]

The two [a,b] arrays will be equal to each other when tested with a 
simple == compare as they are references to the same table.

]]
json_diff.array_match=function(a,b,froma,fromb,sizea,sizeb)

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
	local ls,le=json_diff.array_trim(a,b,froma,fromb,sizea,sizeb)
	
-- first trim
	if ls>0 then
		rb[#rb+1]=append(ra,a,1,ls)
	end
	
-- adjust by trim
	froma = froma+ls
	fromb = fromb+ls
	sizea = sizea-le
	sizeb = sizeb-le

	local recurse
	recurse=function(a,b,froma,fromb,sizea,sizeb)

		local length , starta , startb = json_diff.array_common(a,b,froma,fromb,sizea,sizeb)
		
		if length then -- something matched
		
			if starta~=froma or startb~=fromb then
				recurse(a,b,froma,fromb,starta-1,startb-1) -- first bit
			end
			
			rb[#rb+1]=append(ra,a,starta,starta+length-1) -- matching bits tables will be the same so ra[i]==rb[i]

			if starta+length~=sizea or startb+length~=sizeb then
				recurse(a,b,starta+length,startb+length,sizea,sizeb) -- last bit
			end

		else -- nothing matched so insert all data in return tables
		
			append(ra,a,froma,sizea)
			append(rb,b,fromb,sizeb)

		end

	end
	recurse(a,b,froma,fromb,sizea,sizeb)

-- last trim
	if le>0 then
		rb[#rb+1]=append(ra,a,#ra-le,#ra)
	end

	return ra,rb
end

--[[#lua.wetgenes.json_diff.equal

Compare two json values and return true if they are equal, this may 
decend into a tree of tables and objects so can be an expensive test.

]]
json_diff.equal=function(a,b)

	if a==b then return true end

	if type(a)=="table" and type(b)=="table" then
		if a[1] and b[1] then -- array
			if #a~=#b then return false end -- must be same length
			for i=1,#a do
				if not json_diff.equal( a[i] , b[i] ) then return false end
			end
			return true
		else -- object
			local done={} -- checks can be very expensive so only perform once
			for i,v in pairs(a) do
				done[i]=true
				if not json_diff.equal( a[i] , b[i] ) then return false end
			end
			for i,v in pairs(b) do
				if not done[i] then -- do not check twice
					if not json_diff.equal( a[i] , b[i] ) then return false end
				end
			end
			return true
		end
	end

	return false
end

--[[#lua.wetgenes.json_diff.similar

Compare two json values and return true if they are similar arrays or 
objects, this may descend into a tree of tables and objects so can be 
an expensive test.

An array or object is similar if it contains at least one value that is 
the same in both.

Will return false if not given two objects or two arrays.

]]
json_diff.similar=function(a,b)

	if type(a)=="table" and type(b)=="table" then
		if a[1] and b[1] then -- array
			if json_diff.array_common(a,b) then return true end
			return false
		else -- object
			for i,v in pairs(a) do
				if json_diff.equal( a[i] , b[i] ) then return true end
			end
			return false
		end
	end

	return false
end

--[[#lua.wetgenes.json_diff.diff

Return a diff of two values

]]
json_diff.diff=function(a,b,both)

-- array or object
	if type(a)=="table" and type(b)=="table" then
		if a[1] and b[1] then -- both are arrays
-- these are probed in pairs to pickup the following patterns
-- {num,num} {0,tab} {num,tab} {tab,num} {tab,tab}
			local d={"a"}
			local ra,rb=json_diff.array_match(a,b)
			for idx=1,#ra do
				if ra[idx]==rb[idx] then -- the same
					local len=#ra[idx]
					d[#d+1]=-len
				else -- diff
					local l=#ra[idx] ; if #rb[idx]<l then l=#rb[idx] end -- minimum
					if l>0 then -- diff this many shared datas
						d[#d+1]=l
						for i=1,l do
							d[#d+1]=json_diff.diff(ra[idx][i],rb[idx][i],both)
						end
						if l<#ra[idx] then
							local r={} ; for i=l,#ra[idx] do r[#r+1]=ra[idx][i] end
							d[#d+1]=0 -- empty rb[idx]
							d[#d+1]=both and r or #r -- ra[idx] remainder 
						elseif l<#rb[idx] then
							local r={} ; for i=l,#rb[idx] do r[#r+1]=rb[idx][i] end
							d[#d+1]=r -- rb remainder 
							d[#d+1]=0 -- empty ra[idx]
						end
					else -- one of these will be a 0 , both should not be
						if #rb[idx]==0 then		d[#d+1]=0	-- empty
						else					d[#d+1]=rb[idx]	-- data
						end
						if #ra[idx]==0 then		d[#d+1]=0	-- empty
						else					d[#d+1]=both and ra[idx] or #ra[idx]	-- data or count
						end
					end
				end
			end
			return d
		end
		if ( not a[1] ) and ( not b[1] ) then -- both are objects
			local d={"o"}
			local done={} -- checks can be very expensive so only perform once<
			for i,v in pairs(a) do
				done[i]=true
				if not json_diff.equal( a[i] , b[i] ) then
					d[#d+1]=i
					d[#d+1]=json_diff.diff(a[i],b[i],both)
				end
			end
			for i,v in pairs(b) do
				if not done[i] then -- do not check twice
					if not json_diff.equal( a[i] , b[i] ) then
						d[#d+1]=i
						d[#d+1]=json_diff.diff(a[i],b[i],both)
					end
				end
			end
			return d
		end
	end

-- value this also deals with array to object or object to array
-- so not just for numbers and strings
	if both then
		return { "v" , b , a }
	end
	return { "v" , b }
end

--[[#lua.wetgenes.json_diff.dupe

create a (possible) deep copy duplicate of a json value

]]
json_diff.dupe=function(a)
	if type(a)=="table" then
		local r={}
		for i,v in pairs(a) do r[i]=json_diff.dupe(v) end
		return r
	end
	return a
end

--[[#lua.wetgenes.json_diff.apply

apply a diff , please dupe a before handing it to this function as we 
will be altering it.

]]
json_diff.apply=function(a,d)
	if d[1]=="v" then
		return d[2]
	elseif d[1]=="o" then
		local idx=2
		while d[idx] do
			a[ d[idx] ]=json_diff.apply( a[ d[idx] ],d[idx+1])
			idx=idx+2
		end
	elseif d[1]=="a" then
		local idx=2
		local iax=1
		while d[idx] do
			local da=d[idx]
			local db=d[idx+1]
			local ta=type(da)
			local tb=type(db)
			if tb=="table" then db=#db end -- db will always be length of db table
			
			if     ta=="number" and da<0 then -- advance over values that have not changed
				iax=iax-da
				idx=idx+1
			elseif ta=="number" and da==0 then -- delete
				for i=1,db do -- db will always be length of db table
					table.remove(a,iax+db-i) -- backwards
				end
				idx=idx+2
			elseif ta=="number" and tb=="table"  then
				for i=1,da do
					a[iax+i-1]=json_diff.apply(a[iax+i-1],d[idx+i])
				end
				iax=iax+da
				idx=idx+1+da
			elseif ta=="table"  and ( tb=="number" or tb=="table" ) then -- replace
				if db>0 then -- db will always be length of db table
					for i=1,db do table.remove(a,iax+db-i) end -- remove backwards
				end
				for i=1,#da do table.insert(a,iax+i-1,da[i]) end -- add
				iax=iax+#da
				idx=idx+2
			else
				error("invalid json_diff types")
			end
		end
	else
		error("invalid json_diff")
	end
	return a
end

--[[#lua.wetgenes.json_diff.undo

undo a diff which *must* have been created with the both flag set in 
order to have undo data available.

]]
json_diff.undo=function(b,d)
	if d[1]=="v" then
		return d[3]
	elseif d[1]=="o" then
		local idx=2
		while d[idx] do
			b[ d[idx] ]=json_diff.undo( b[ d[idx] ],d[idx+1])
			idx=idx+2
		end
	elseif d[1]=="a" then
		local idx=2
		local ibx=1
		while d[idx] do
			local da=d[idx]
			local db=d[idx+1]
			local ta=type(da)
			local tb=type(db)
			if ta=="table" then da=#da end -- da will always be length of da table
			
			if     ta=="number" and da<0 then -- advance over values that have not changed
				ibx=ibx-da
				idx=idx+1
			elseif ta=="number" and da==0 then -- insert
				for i=1,#db do table.insert(b,ibx+i-1,db[i]) end -- add
				idx=idx+2
			elseif ta=="number" and tb=="table"  then
				for i=1,da do
					b[ibx+i-1]=json_diff.undo(b[ibx+i-1],d[idx+i])
				end
				ibx=ibx+da
				idx=idx+1+da
			elseif ta=="table" and ( tb=="table" or tb=="number" ) then -- replace
				if da>0 then
					for i=1,da do table.remove(b,ibx+da-i) end -- remove backwards
				end
				if tb=="table" then -- might be zero
					for i=1,#db do table.insert(b,ibx+i-1,db[i]) end -- add
					ibx=ibx+#db
				else
					assert(db==0)
				end
				idx=idx+2
			else
				error("invalid json_diff types")
			end
		end
	else
		error("invalid json_diff")
	end
	return b
end

