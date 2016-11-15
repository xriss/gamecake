--
-- (C) 2013 Kriss@XIXs.com
--

local table=table
local string=string

local ipairs=ipairs
local pairs=pairs

local type=type

local print=log or print
local assert=assert

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
  arg[0]=label
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
-- simple but will parse small basic data xml files just fine and thats what im using it for
--
-- it is however very easy to produce a valid xml file that will break this...
-- but, as long as you are just using tags/data then this is fine
--
-- by putting the tag name in [0] we can use the string namespace for all attributes
-- and can iterate over the table entries 1+ in a normal way
--
-- [0] == tag name
-- [1++] == contained strings or tables (sub tags)
-- [stringnames] == attributes IE all string keys
--
-----------------------------------------------------------------------------
function parse(s)
  local stack = {}
  local top = {}
  table.insert(stack, top)
  local i = 1

local ret_stack=2 -- we start by assuming we will be skipping a header

  local warnings
  local warn=function(s)
	if not warnings then warnings={} end
	warnings[#warnings+1]=s
  end



local tagpatq="<%?(%/?)([%w_%-%:]+)(.-)(%/?)%?>"
local tagpat ="<(%/?)([%w_%-%:]+)(.-)(%/?)>"

	local j2
-- find header
  local ni,j,c,label,args, empty = string.find(s, tagpatq)

  if not ni then -- ignore missing <? tag header at start
	ret_stack=1
	ni,j,c,label,args, empty = string.find(s, tagpat)
  end
  label=(label or "?"):lower()
  
  while ni do
--print(ni,j,c,label,args,empty)
    local text = string.sub(s, i, ni-1)

--print(text)
    
    if not string.find(text, "^%s*$") then -- if not just white space
      table.insert(top, text)
    end
    if empty == "/" then  -- empty element tag
      table.insert(top, parse_args(args,label) )
    elseif c == "xml" then   -- top tag
      top = parse_args(args,label)
      table.insert(stack, top)   -- new level
    elseif c == "" then   -- start tag
      top = parse_args(args,label)
      table.insert(stack, top)   -- new level
    else  -- end tag
		local autoclose=true -- autoclose
		while autoclose do
		  local toclose = table.remove(stack)  -- remove top
		  top = stack[#stack]
		  if #stack < 1 then
			warn("Tag <"..label.."> not matched ")
		  else
			  if toclose[0] ~= label then
				warn("Tag <"..(toclose[0] or "?").."> doesnt match <"..(label or "?")..">.")
				else
				autoclose=false
			  end
		  end
		  if top then
			table.insert(top, toclose)
		  else
			warn("Malformed XML.")
			break
		  end
		end
    end 
    i = j+1

-- catch cdata in tag
    ni,j2,text = string.find(s, "^%<%!%[CDATA%[(.-)%]%]%>", i)
    if ni then
--print(text)
		table.insert(top, text) -- just insert the content
		j=j2
		i = j+1
	end
    
    
    ni,j,c,label,args, empty = string.find(s,tagpat, i)

    label=(label or "?"):lower()
  end
  local text = string.sub(s, i)
  if not string.find(text, "^%s*$") then
    table.insert(stack[#stack], text)
  end
  return stack[ret_stack],warnings
end



-----------------------------------------------------------------------------
--
-- convert everything within this chunk, back to a html string
--
-----------------------------------------------------------------------------
function unparse(parent,opts)
	opts=opts or {tabjoin=true}

	local tabjoin=opts.tabjoin -- only one of these recursive calls should have this flag
	if tabjoin then opts.tabjoin=false end -- so disable it
	
	local out=function(s) opts.out[#opts.out+1]=s end
	local att=function(t)
		local at={}
		for i,v in pairs(t) do
			local t=type(i)
			if t=="string" then
				at[#at+1]=i.."=".."\""..v.."\""
			end
		end
		if at[1] then
			return " "..table.concat(at," ").." "
		else
			return ""
		end
	end
	
	if not opts.out then opts.out={} end

	for i,v in ipairs(parent) do
		local t=type(v)
		if t=="string" then
			out(v)
		elseif t=="table" then
			if v[0] then
				if v[1] then -- stuff within this tag
					out("<"..v[0]..att(v)..">")
						unparse(v,opts)
					out("</"..v[0]..">")
				else -- an empty tag
					out("<"..v[0]..att(v).."/>")
				end
			end
		end
	end
	
	if tabjoin then -- reenable flag before returning
		opts.tabjoin=true
		return table.concat(opts.out)
	else
		return opts
	end
end

-----------------------------------------------------------------------------
--
-- get/set the attr tag of the given type (deals with silly case problems)
--
-----------------------------------------------------------------------------
function attr(parent,name,set)

	name=string.lower(name)

	if set then parent[name]=set end

	return parent[name]
--[[

	for n,v in pairs(parent) do
		if type(n)=="string" and string.lower(n)==name then
			if set then parent[n]=set end
			return v
		end
	end
	
	return nil
]]
end


-----------------------------------------------------------------------------
--
-- find the first child tag of the given type, this does not recurse
--
-----------------------------------------------------------------------------
function child(parent,name)

	name=string.lower(name)

	for i,v in ipairs(parent) do
		if type(v)=="table" and v[0] and string.lower(v[0])==name then return v end
	end
	
	return nil

end

-----------------------------------------------------------------------------
--
-- find all the children tag of the given type, this does not recurse
--
-----------------------------------------------------------------------------
function childs(parent,name)

	name=string.lower(name)
	
	local t={}
	
	for i,v in ipairs(parent) do
		if type(v)=="table" and v[0] and string.lower(v[0])==name then t[#t+1]=v end
	end
	
	return t

end

-----------------------------------------------------------------------------
--
-- find the first descendent tag of the given type, this does recurse
--
-----------------------------------------------------------------------------
function descendent(parent,name)

	name=string.lower(name)

	for i,v in ipairs(parent) do
		if type(v)=="table" and v[0] and string.lower(v[0])==name then return v end
	end
	
	for i,v in ipairs(parent) do -- recurse
		local ret
		if type(v)=="table" then ret=descendent(v,name) end
		if ret then return ret end
	end
	
	return nil

end

-----------------------------------------------------------------------------
--
-- find **all** the descendent tags of the given type, this does recurse
--
-----------------------------------------------------------------------------
function descendents(parent,name,opts)
	opts=opts or {}
	opts.out=opts.out or {}
	
	local out=function(s) opts.out[#opts.out+1]=s end
	
	name=string.lower(name)

	for i,v in ipairs(parent) do -- this level
		if type(v)=="table" and v[0] and string.lower(v[0])==name then out(v) end
	end
	
	for i,v in ipairs(parent) do -- recurse
		if type(v)=="table" then descendents(v,name,opts) end
	end
	
	return opts.out -- maybe an empty output
end



-----------------------------------------------------------------------------
--
-- find the first descendent tag of the given class, this does recurse
--
-----------------------------------------------------------------------------
function class(parent,name)

	name=string.lower(name)

	for i,v in ipairs(parent) do
		if type(v)=="table" then
			if attr(v,"class") then
--print( attr(v,"class") .."\n" )
				if string.lower(attr(v,"class"))==name then return v end
			end
		end
	end
	
	for i,v in ipairs(parent) do -- recurse
		local ret
		if type(v)=="table" then ret=class(v,name) end
		if ret then return ret end
	end
	
	return nil

end

-----------------------------------------------------------------------------
--
-- find **all** the descendent tags of the given class, this does recurse
--
-----------------------------------------------------------------------------
function classes(parent,name,opts)
	opts=opts or {}
	opts.out=opts.out or {}
	
	local out=function(s) opts.out[#opts.out+1]=s end
	
	name=string.lower(name)

	for i,v in ipairs(parent) do -- this level
		if type(v)=="table" then
			if attr(v,"class") then
				if string.lower(attr(v,"class"))==name then out(v) end
			end
		end
		if type(v)=="table" and attr(v,"class") and string.lower(attr(v,"class"))==name then out(v) end
	end
	
	for i,v in ipairs(parent) do -- recurse
		if type(v)=="table" then classes(v,name,opts) end
	end
	
	return opts.out -- maybe an empty output

end

