--
-- wetjson
--         version 2011-02-04
--         use encode to encode a table and decode to decode a json string
--
-- other json encode/decode pure lua library seemed too slow
-- here is a fast and loose one lets see if it goes any faster :)
-- should be a direct replacement for JSON4Lua which is what I was using before
--
-- find the latest version online here
-- http://code.google.com/p/aelua/source/browse/trunk/aelua/lua/wetjson.lua
--
--
-- Copyright (C) 2011 by http://about.wetgenes.com/
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.
--

local table=table
local string=string


local print=print
local type=type
local ipairs=ipairs
local pairs=pairs
local tonumber=tonumber
local tostring=tostring
local math=math
local error=error

module(...)

null=function() return null end -- wetjson.null is a magick value to represent null

-----------------------------------------------------------------------------
--
-- trim whitespace from ends of string
--
-----------------------------------------------------------------------------
local function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

-----------------------------------------------------------------------------
--
-- unescape a string
--
-----------------------------------------------------------------------------
local unesc_tab={
	["b"]="\b",
	["f"]="\f",
	["n"]="\n",
	["r"]="\r",
	["t"]="\t",
}
local function unesc(s)

	s=string.gsub(s, "\\([^u])", function(c)
		return unesc_tab[c] or c
    end)

	s=string.gsub(s, "\\u(%x%x%x%x)", function(c)
		return string.char( tonumber(c,16) or 32 ) or ""
    end)
    
  return s
end

-----------------------------------------------------------------------------
--
-- escape a string anything outside of basic printable 7bit ascii or a " or a \
--
-----------------------------------------------------------------------------
local function esc(s)

	s=string.gsub(s, "([^#-Z_-~ !%^%[%]])", function(c)
		return string.format( "\\u%04x" , string.byte( c ) )
    end)
    
  return s
end

-----------------------------------------------------------------------------
--
-- split on interesting characters to create a table
--
-- a concat on the result would be a perfect reproduction of the original
--
-----------------------------------------------------------------------------
local function split(text)
	local separator = "[\",:{}%[%]']"
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	while split_start do
		if split_start>1 then table.insert(parts, text:sub(start, split_start-1)) end		-- the word
		table.insert(parts, text:sub(split_start, split_end))	-- the white space
		start = split_end + 1
		split_start, split_end = text:find(separator, start)
	end
	
	if text:sub(start)~="" then
		table.insert(parts, text:sub(start) )
	end
	
	return parts
end


-----------------------------------------------------------------------------
--
-- is this an array?
--
-----------------------------------------------------------------------------
local function is_array(t)
	local len=#t
	if len==0 then return false end -- short circuit
	for i,v in pairs(t) do
		if type(i)=="number" then
			if math.floor(i)~=i then -- must be int
				return false
			end
			if i<1 or i>len then -- and in this range
				return false
			end
		else
			return false
		end
	end
	return true
end

-----------------------------------------------------------------------------
--
-- json string to table
--
-----------------------------------------------------------------------------
function decode(s,opts)
opts=opts or {}

local t

-- start by adjusting all \" escapes in the string into \u0000 escapes
-- that way we do not have to worry about " being anywhere apart
-- from real string deliminators which makes parsing easier?
-- this way we wont have to ignore \" as a special case inside strings

	s=string.gsub(s, "\\([\"])", function(c)
		return string.format("\\u%04x",string.byte(c))
    end)


-- now we break the string using any of the following characters as deliminators
-- {}[]:,"
-- this gives an array of interesting points to parse

	t=split(s,opts)
	
	local chash=0
	local out={}
	local top={tab=out,idx=1,inc=true}
	local stack={ top }
	local sb -- string buffer building for the use of
	local sbend -- the string terminator
	
	function err(s)
		error(s.." ("..chash..")")
	end
	
	function push(v)
		top=v
		stack[#stack+1]=top
	end
	
	function pop()
		stack[#stack]=nil
		top=stack[#stack]
		if not top then err("too many close brackets") end
	end
	
	function setval()
		if top.idx==nil then -- set idx not val
			if type(val)=="table" then err("cannot use table as index") end
			top.idx=val
			val=nil
		else
--print(top.idx,"=",val)
			top.tab[top.idx]=val
			if top.inc then top.idx=top.idx+1 else top.idx=nil end
		end
	end
	
	for i,v in ipairs(t) do
			
		if sb then -- continue building a string
		
			if v==sbend then -- end of string
				if sb[2] then
					val=unesc(table.concat(sb))
				else
					val=unesc(sb[1])
				end
				sb=nil
				setval()
			else
				sb[#sb+1]=v
			end
			
		else
		
			local l=trim(v) -- remove any white space from both ends
--print(l,#stack)			
			if #l>0 then -- ignore whitespace
			
				if l=="\"" then -- start a string 
					sb={}
					sbend="\""
				elseif l=="'" then -- start a string 
					sb={}
					sbend="'"
				elseif l=="{" then
					val={}
					setval()
					push({tab=val})
					val=nil
				elseif l=="}" then
					pop()
				elseif l=="[" then
					val={}
					setval()
					push({tab=val,idx=1,inc=true})
					val=nil
				elseif l=="]" then
					pop()
				elseif l==":" then
				elseif l=="," then
				else
					val=l
					if val=="true" then val=true
					elseif val=="false" then val=false
					elseif val=="null" then val=null
					else
						val=tonumber(val) or val
					end
					setval()
				end
				
			end
		end
	
		chash=chash+#v -- very basic error locator
	end
	
	
	
--print(#t)

	return out and out[1] , "OK"
end


-----------------------------------------------------------------------------
--
-- table to json string
--
-----------------------------------------------------------------------------
function encode(tab,opts)
opts=opts or {}

local out={}
local put=function(s)
	out[#out+1]=s or ""
end

local encode_str
local encode_it
local encode_tab

	function err(s)
		error(s)
	end
	
	if not tab then return err("null inout") end

	encode_str=function(str)
		return "\""..esc(tostring(str)).."\""
	end
	
	encode_it=function(it,t)
		t=t or type(it)
		if t=="number" then
			return tostring(it)
		elseif t=="boolean" then
			if it then return "true" else return "false" end
		elseif t=="function" then
			if it==null then return "null" end
		else
			return encode_str(it)
		end
		
		return ""
	end
	
	encode_tab=function(vv,array)
		local t
		local comma=false
		if array then
			put("[")
			for i=1,#vv do local v=vv[i]
				put(comma and ",") comma=true
				t=type(v)
				if t=="table" then
					encode_tab(v,is_array(v))
				else
					put(encode_it(v,t))
				end
			end
			put("]")
		else
			put("{")
			for i,v in pairs(vv) do
				put(comma and ",") comma=true
				put(encode_it(i)) -- allow numbers or strings
				put(":")
				t=type(v)
				if t=="table" then
					encode_tab(v,is_array(v))
				else
					put(encode_it(v,t))
				end
			end
			put("}")
		end
	end

	encode_tab(tab,is_array(tab)) -- technically this should not be an array but we allow it


	return table.concat(out)
end

