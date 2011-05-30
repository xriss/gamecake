
local table=table
local string=string

local ipairs=ipairs
local pairs=pairs

local type=type

local print=log or print
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
  arg[0]=label or "?"
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
  local ni,j,c,label,args, empty = string.find(s, "<%?(%/?)([%w_]+)(.-)(%/?)%?>")

local ret_stack=2
  
  if not ni then -- ignore missing <? tag
	ret_stack=1
	ni,j,c,label,args, empty = string.find(s, "<(%/?)([%w_]+)(.-)(%/?)>")
  end

  
  while ni do
--print(ni,j,c,label,args,empty)
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
  return stack[ret_stack]
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

	for n,v in pairs(parent) do
		if type(n)=="string" and string.lower(n)==name then
			if set then parent[n]=set end
			return v
		end
	end
	
	return nil

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


