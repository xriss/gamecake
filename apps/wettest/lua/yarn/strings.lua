
--some functions for manipulating strings, copypasta rather than depend on external libs

local _G=_G


local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local require=require
local type=type
local loadstring=loadstring
local pairs=pairs
module(...)

-----------------------------------------------------------------------------
--
-- split on separator
--
-- such that a concat(seperator) on the result would return the original
--
-----------------------------------------------------------------------------
function split(text,separator)
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	while split_start do
		if split_start>1 then table.insert(parts, text:sub(start, split_start-1)) end		-- the word
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
-- split on transition to or from whitespace, include this white space in the table result
--
-- such that a concat on the result would be a perfect reproduction of the original
--
-----------------------------------------------------------------------------
function split_whitespace(text)
	local separator = "%s+"
	
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
-- trime whitespace from ends of string
--
-----------------------------------------------------------------------------
function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end
function trim_start(s)
  return (s:gsub("^%s*(.-)", "%1"))
end
function trim_end(s)
  return (s:gsub("(.-)%s*$", "%1"))
end

-----------------------------------------------------------------------------
--
-- split on \n, each line also includes its own \n
--
-----------------------------------------------------------------------------
function split_lines(text)
	local separator = "\n"
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start,true)
	
	while split_start do
		table.insert(parts, text:sub(start, split_end))
		start = split_end + 1
		split_start, split_end = text:find(separator, start,true)
	end
	
	if text:sub(start)~="" then
		table.insert(parts, text:sub(start) )
	end
	
	return parts
end

-----------------------------------------------------------------------------
--
-- wrap a string to a given width, merging all whitespace to spaces but keeping line breaks
--
-----------------------------------------------------------------------------
function smart_wrap(s,w)
	local ls=split_whitespace(s)
	local t={}
	
	local wide=0
	local line={}
	
	local function newline()
		t[#t+1]=table.concat(line," ") or ""
		wide=0
		line={}
	end
	
	for i,v in ipairs(ls) do
	
		if v:find("%s") then -- just white space
		
			for i,v in string.gfind(v,"\n") do -- keep newlines
				newline()
			end
		
		else -- a normal word
		
			if wide + #v > w then -- split
				newline()
			end
			
			line[#line+1]=v
			if #line>1 then wide=wide+1 end
			wide=wide+#v
			
		end
	end
	if wide~=0 then newline() end -- final newline
	
	return t
end


-----------------------------------------------------------------------------
--
-- opposite of serialize, returns table
--
-----------------------------------------------------------------------------
function unserialize(s)
	if not s then return nil end
	
	local t
	local f,err=loadstring(s)
	if f then
		t={}
		setfenv(f,t)
		local ret,err pcall(f)
		if not ret then t=nil end -- error
	end
	return t
end

-----------------------------------------------------------------------------
--
-- serialize a simple table to a lua string that would hopefully recreate said table if executed
--
-- returns a string
--
-----------------------------------------------------------------------------
function serialize(o,fout)

	if not fout then -- call with a function to build and return a strin	
		local ret={}
		fout=function(s)
			ret[#ret+1]=s
		end
		serialize(o,fout)		
		return table.concat(ret)
	end

	if type(o) == "number" then
	
		return fout(o)
		
	elseif type(o) == "boolean" then
	
		if o then return fout("true") else return fout("false") end
		
	elseif type(o) == "string" then
	
		return fout(string.format("%q", o))
		
	elseif type(o) == "table" then
	
		fout("{\n")
		
		for k,v in pairs(o) do
			fout("  [")
			serialize(k,fout)
			fout("] = ")
			serialize(v,fout)
			fout(",\n")
		end
		
		return fout("}\n")
	else
		error("cannot serialize a " .. type(o))
	end
	
end


