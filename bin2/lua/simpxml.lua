
local table=table
local string=string

local pairs=pairs
local ipairs=ipairs

local type=type

local tostring=tostring
local print=print

module(...)


-----------------------------------------------------------------------------
--
-- some old simple xml parsing code, found here and fixed up a little :)
--
-- http://lua-users.org/lists/lua-l/2002-06/msg00040.html
--
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
--
-- auxiliar function to parse tag attributes
--
-----------------------------------------------------------------------------
function parse_args(s,label)
  local arg = {}
  arg[0]=string.lower(label or "?")
  string.gsub(s, "([%w_]+)%s*=%s*([\"'])(.-)%2", function (w, _, a)
    arg[string.lower(w)] = a
  end)
  return arg
end

-----------------------------------------------------------------------------
--
-- string "s" is a string with XML marks. This function parses the string
-- and returns the resulting tree.
--
-- simple but will parse small data xml files just fine and thats what im using it for
--
-- it is very easy to produce a valid xml file that will break this
-- however as long as you are just using tags/data and especially if you control the creation
-- everything will be peachy
--
-- by putting the tag name in [0] we can use the string namespace for all attributes
--
-- [0] == tag name (lowercase)
-- [1++] == contained strings or tables (sub tags)
-- [string] == attributes (indexe strings will be converted to lowercase)
--
-- all string keys are attributes associated with the tag
-- all numerical keys are sub tags/strings except [0] which is the tag name
--
-- see the other functions here for how to simply step through this tree
--
-----------------------------------------------------------------------------
function parse(s)
  local stack = {}
  local top = {}
  table.insert(stack, top)
  local i = 1
  local ni,j,c,label,args, empty = string.find(s, "<%?(%/?)([%w_]+)(.-)(%/?)%?>")
  
  while ni do
    local text = string.sub(s, i, ni-1)
    if not string.find(text, "^%s*$") then
      table.insert(top, text)
    end
    if empty == "/" then  -- empty element tag
      table.insert(top, parse_args(args,label) )
    elseif c == "" then   -- start tag
      top = parse_args(args,label)
      table.insert(stack, top)   -- new level
    else  -- end tag
      local toclose = table.remove(stack)  -- remove top
      top = stack[#stack]
      if #stack < 1 then
        assert(false,"Tag <"..label.."> not matched ")
      end
      if toclose[0] ~= label then
	    assert(false,"Tag <"..(toclose[0] or "?").."> doesnt match <"..(label or "?")..">.")
      end
      table.insert(top, toclose)
    end 
    i = j+1
    ni,j,c,label,args, empty = string.find(s, "<(%/?)([%w_]+)(.-)(%/?)>", j)
  end
  local text = string.sub(s, i)
  if not string.find(text, "^%s*$") then
    table.insert(stack[#stack], text)
  end
  return stack[2]
end


-----------------------------------------------------------------------------
--
-- find the first child tag of the given type
--
-----------------------------------------------------------------------------
function child(parent,name)

	if type(name)=="table" then -- sub sub sub lookup
	
		local ret=parent
		for i,v in ipairs(name) do
			if not ret then return nil end
			ret=child(ret,v)
		end
		return ret
		
	else
		name=string.lower(name)

		for i,v in ipairs(parent) do
			if type(v)=="table" and v[0] and string.lower(v[0])==name then return v end
		end

	end
	
	return nil

end



-----------------------------------------------------------------------------
--
-- convert an xml tree to an xml string that could then be output to a file
--
-- again, best used with small data sets only please :)
--
-----------------------------------------------------------------------------
function tree_to_string(tree)

	local t={}
	function out(s) t[#t+1]=tostring(s) end
	local indent=-1
	
	local function dosub(l)
		indent=indent+1
	
		local tabs=""
		if indent>0 then tabs=string.rep(" ",indent) end
	
		out(tabs)
		out("<"..l[0])
		local ns={}
		for i,v in pairs(l) do
			if type(i)=="string" then --find attributes
				ns[#ns+1]=i
			end
		end
		table.sort(ns) -- ascii sort attributes
		for i=1,#ns do
			out(" "..ns[i].."=\""..l[ ns[i] ].."\"")
		end
		if not l[1] then -- no children
			out(" />\n")
		else
			out(">\n")
			
			for i=1,#l do
				if type(l[i])=="table" then
					dosub(l[i])
				else
					out(l[i])
				end
			end
			
			out(tabs)
			out("</"..l[0]..">\n")
		end

		indent=indent-1
	end
-- the top chunk is an xml version string and possibly some text
-- skip that bit, just output its chunks with this standard xml header
	out([[<?xml version="1.0" standalone="no" ?>]])
	out("\n")
	for i=1,#tree do
		if type(tree[i])=="table" then
			dosub(tree[i])
		end
	end

	return table.concat(t)
end
