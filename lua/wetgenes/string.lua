--
-- (C) 2013 Kriss@XIXs.com
--

local string=string
local table=table
local math=math

local type=type
local pairs=pairs
local ipairs=ipairs
local tostring=tostring
local setmetatable=setmetatable
local error=error
local tonumber=tonumber

local log=print

--
-- Some useful string functions.
--

local wstr={ modname=(...) } ; package.loaded[wstr.modname]=wstr

-----------------------------------------------------------------------------
--
-- turn a number of seconds into a rough duration
--
-----------------------------------------------------------------------------
wstr.rough_english_duration=function(t)
	t=math.floor(t)
	if t>=2*365*24*60*60 then
		return math.floor(t/(365*24*60*60)).." years"
	elseif t>=2*30*24*60*60 then
		return math.floor(t/(30*24*60*60)).." months" -- approximate months
	elseif t>=2*7*24*60*60 then
		return math.floor(t/(7*24*60*60)).." weeks"
	elseif t>=2*24*60*60 then
		return math.floor(t/(24*60*60)).." days"
	elseif t>=2*60*60 then
		return math.floor(t/(60*60)).." hours"
	elseif t>=2*60 then
		return math.floor(t/(60)).." minutes"
	elseif t>=2 then
		return t.." seconds"
	elseif t==1 then
		return "1 second"
	else
		return "0 seconds"
	end
end

-----------------------------------------------------------------------------
--
-- split a string into a table, flag enables pattern match on true
--
-----------------------------------------------------------------------------
wstr.str_split = function(div,str,flag)

	if (str=='') then return {""} end
	
	if (div=='') or not div then error("div expected", 2) end
	if (str=='') or not str then error("str expected", 2) end

	local pos,arr = 0,{}

	-- for each divider found
	for st,sp in function() return string.find(str,div,pos,not flag) end do
		table.insert(arr,string.sub(str,pos,st-1)) -- Attach chars left of current divider
		pos = sp + 1 -- Jump past current divider
	end

	if pos~=0 then
		table.insert(arr,string.sub(str,pos)) -- Attach chars right of last divider
	else
		table.insert(arr,str) -- return entire string
	end


	return arr
end
--  yeah the above is bad and should be turned into this
wstr.split = function (str,div,flag) return wstr.str_split(div,str,flag) end



-----------------------------------------------------------------------------
--
-- serialize a simple table to a lua string that would hopefully recreate said table if executed
--
-- returns a string
--
-----------------------------------------------------------------------------
wstr.serialize = function(o,opts)
opts=opts or {}
opts.done=opts.done or {} -- only do tables once

opts.indent=opts.indent or ""
opts.newline=opts.newline or ( opts.compact and "" or "\n" )

local fout=opts.fout

	if not fout then -- call with a new function to build and return a string
		local ret={}
		opts.fout=function(...)
			for i,v in ipairs({...}) do ret[#ret+1]=v end
		end
		wstr.serialize(o,opts)		
		opts.fout(opts.newline)
		return table.concat(ret)
	end

	if type(o) == "number" then
	
		return fout(o)
		
	elseif type(o) == "boolean" then
	
		if o then return fout("true") else return fout("false") end
		
	elseif type(o) == "string" then
	
		return fout(string.format("%q", o))
		
	elseif type(o) == "table" then
	
		
		if opts.done[o] and opts.no_duplicates then
			fout(opts.indent,"\n",opts.indent,"{--[[DUPLICATE]]}",opts.newline)
			return
		else
		
			fout("{",opts.newline)

			if opts.pretty then
				opts.indent=opts.indent.." "
			end
			
			opts.done[o]=true
			
			local maxi=0
			
			for k,v in ipairs(o) do -- dump number keys in order
				fout(opts.indent)
				wstr.serialize(v,opts)
				fout(",",opts.newline)
				maxi=k -- remember top
			end
			
			for k,v in pairs(o) do
				if (type(k)~="number") or (k<1) or (k>maxi) or (math.floor(k)~=k) then -- skip what we already dumped
					fout(opts.indent,"[")
					wstr.serialize(k,opts)
					fout("]=")
					wstr.serialize(v,opts)
					fout(",",opts.newline)
				end
			end
			
			if opts.pretty then
				opts.indent=opts.indent:sub(1,-2)
			end
			fout(opts.indent,"}")
			return
		end
	elseif type(o) == "nil" then	
		return fout("nil")
	else
		if opts.no_errors then
			return fout("nil--[[",type(o),"]]")
		else
			error("cannot serialize a " .. type(o))
		end
	end
	
end

-----------------------------------------------------------------------------
--
-- dump a table to a lua string for debuging output, 
--
-- returns a string
--
-----------------------------------------------------------------------------
wstr.dump = function(o,opts)
	opts=opts or {}
	
	opts.pretty=true
	opts.no_duplicates=true
	opts.no_errors=true

	return wstr.serialize(o,opts)
end

-- since we keep using this...
wstr.ls=function(o) print(wstr.dump(o)) end

-----------------------------------------------------------------------------
--
-- append english number postfix, 1st 2nd 3rd 4th etc
--
-----------------------------------------------------------------------------
wstr.str_insert_number_commas = function (n)

	local s=tostring(math.floor(n))
	local t={}
	
	while #s > 3 do
		table.insert(t,1,s:sub(-3))
		s=s:sub(1,-4)
	end
	table.insert(t,1,s)

	return table.concat(t,",")
end

-----------------------------------------------------------------------------
--
-- append english number postfix, 1st 2nd 3rd 4th etc
--
-----------------------------------------------------------------------------
wstr.str_append_english_number_postfix = function(n)

	local ith=n%10
	if n>10 and n<20 then ith=4 end -- teens are all "th"

	if     ith==1 then return n.."st"
	elseif ith==2 then return n.."nd"
	elseif ith==3 then return n.."rd" end

	return n.."th"
end

-----------------------------------------------------------------------------
--
-- join a table of things into an english list with commas and an "and" at the end
-- returns nil if the table is empty
--
-----------------------------------------------------------------------------
wstr.str_join_english_list = function(t)

local s

	for i,v in ipairs(t) do
	
		if not s then -- first
		
			s=v
			
		elseif t[i+1]==nil then -- last
		
			s=s.." and "..v
			
		else -- middle
		
			s=s..", "..v
			
		end
	
	end

	return s

end

-----------------------------------------------------------------------------
--
-- convert a string into a hex string
--
-----------------------------------------------------------------------------
wstr.str_to_hex = function(s)
	return string.gsub(s, ".", function (c)
		return string.format("%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- replace any %xx with the intended char, eg "%20" becomes a " "
--
-----------------------------------------------------------------------------
wstr.url_decode = function (str)
    return string.gsub(str, "%%(%x%x)", function(hex)
        return string.char(tonumber(hex, 16))
    end)
end

-----------------------------------------------------------------------------
--
-- replace % , & , # , ' , " and = chars with %xx codes
-- this is the bare minimum we need to escape so as not to confuse things
--
-----------------------------------------------------------------------------
wstr.url_encode = function(str)
    return string.gsub(str, "([&=%%#'\"])", function(c)
        return string.format("%%%02X", string.byte(c))
    end)
end

-----------------------------------------------------------------------------
--
-- replace anything that isnt an alphanumeric with %xx codes
--
-----------------------------------------------------------------------------
wstr.url_escape = function(str)
    return string.gsub(str, "([^%w])", function(c)
        return string.format("%%%02X", string.byte(c))
    end)
end

-----------------------------------------------------------------------------
--
-- a one way action that replaces anything that is not a-z or 0-9 with _
-- and converts the entire string to lowercase
--
-----------------------------------------------------------------------------
wstr.alpha_munge = function(str)
    return string.gsub(string.lower(str), "([^a-z0-9])", function(c)
        return "_"
    end)
end

-----------------------------------------------------------------------------
--
-- trime whitespace from ends of string
--
-----------------------------------------------------------------------------
wstr.trim = function(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end
wstr.trim_start = function(s)
  return (s:gsub("^%s*(.-)", "%1"))
end
wstr.trim_end = function(s)
  return (s:gsub("(.-)%s*$", "%1"))
end

-----------------------------------------------------------------------------
--
-- split on \n, each line also includes its own \n
--
-----------------------------------------------------------------------------
wstr.split_lines = function(text,separator)
	separator = separator or "\n"
	
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
-- split on whitespace, throw away all whitespace return only the words
--
-----------------------------------------------------------------------------
wstr.split_words = function(text,split)
	local separator = split or "%s+"
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	while split_start do
		if split_start>1 then table.insert(parts, text:sub(start, split_start-1)) end
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
wstr.split_whitespace = function(text)
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
-- split on transition to or from whitespace, include this white space in the table result
--
-- suport quoted strings ( " or ' ) containing whitespace and ignore backslashed quotes
--
-- such that a concat on the result would be a perfect reproduction of the original
--
-----------------------------------------------------------------------------
wstr.split_whitespace_quotes = function(text)
	local separator = "%s+"
	
	local isquote={["\""]=true,["'"]=true}
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	local quoted=nil
	local quotestart=nil
	
	while split_start do
		if not quoted and isquote[text:sub(start,start)] then -- found start quote
			quoted=text:sub(start,start)
			quotestart=start
		end
		if quoted then -- look for end quote
			if split_start>1 then
				if text:sub(split_start-1, split_start-1)==quoted then -- maybe end, need to count backslashes to be sure
					local bc=0
					for i=split_start-2,start,-1 do if text:sub(i,i)=="\\" then bc=bc+1 else break end end -- count backslashes
					if bc%2==0 then -- not backslashed, so this is the end of the quote
						table.insert(parts, text:sub(quotestart, split_start-1)) -- the quoted string
						table.insert(parts, text:sub(split_start, split_end))	-- the white space
						quoted=nil
						quotestart=nil
					end
				end
			end
		else
			if split_start>1 then table.insert(parts, text:sub(start, split_start-1)) end		-- the word
			table.insert(parts, text:sub(split_start, split_end))	-- the white space
		end
		start = split_end + 1
		split_start, split_end = text:find(separator, start)
	end
	
	if quoted then -- no end quote
		table.insert(parts, text:sub(quotestart) )
	else
		if text:sub(start)~="" then
			table.insert(parts, text:sub(start) )
		end
	end
	
	return parts
end


-----------------------------------------------------------------------------
--
-- split a string in two on first = 
--
-----------------------------------------------------------------------------
wstr.split_equal = function(text)
	local separator = "="
	
	local parts = {}
	local start = 1
	
	local split_start, split_end = text:find(separator, start,true)
	
	if split_start and split_start>1 and split_end<#text then -- data either side of seperator
	
		return text:sub(1,split_start-1) , text:sub(split_end+1)
		
	end
	
	return nil
end



-----------------------------------------------------------------------------
--
-- private replace utility function
-- look up string a inside data d and return the string we found
-- if we dont find anything then we return nil
--
-- if we try to look up a table containing a plate field
-- then that plate name will be used to format that table content as {it.nameofvar}
-- if that table contains a [1] then it will be treated as an array of data
-- and looped over to produce a result.
--
-----------------------------------------------------------------------------
wstr.table_lookup=function(a,d) -- look up a in table d

	local t
	
	local a1,a2=string.find(a, "%:") -- try and split on first ":" if one exists
	if a1 then a=string.sub(a,1,a1-1) end -- only use the bit before the :

	repeat local done=true

		t=d[a] -- as string

		if not t then -- try as number?
			local n=tonumber(a)
			if n then
				t=d[n]
			end
		end
		
		if not t then
			local fc=string.sub(a,1,1)
			if fc=="." or fc=="-" then done=false a=string.sub(a,2) end -- trim any starting dot or dash
		end
	until done

	if t then return t end
	
	local a1,a2=string.find(a, "%.",2) -- try and split on first "."
	if not a1 then return nil end -- didnt find a dot so return nil
	
	a1=string.sub(a,1,a1-1) -- the bit before the .
	a2=string.sub(a,a2+1) -- the bit after the .
	
	local dd=d[a1] -- use the bit before the dot to find the sub table
	
	if not dd then -- try as number?
		local n=tonumber(a1)
		if n then
			dd=d[n]
		end
	end
	
	if type(dd)=="table" then -- check we got a table
		return wstr.table_lookup(a2,dd) -- tail call this function
	end
	
end

wstr.replace_lookup_istable=function(a,d) -- check if this is a special table lookup (so we can leave them til
	local t=wstr.table_lookup(a,d)
	if t and type(t)=="table" then -- if a table then
		return true
	end
	return false
end

wstr.replace_lookup=function(a,d) -- look up a in table d
	local t=wstr.table_lookup(a,d)
	if t then
		if type(t)=="table" then -- if a table then
			local plate
			
			local a1,a2=string.find(a, "%:") -- try and split on first ":" if one exists
			if a2 then plate=string.sub(a,a2+1) end -- the bit before the :
			plate=plate or t.plate

			if t[1] and plate then -- a list of stuff and a plate
				local tt={}
				local it=d.it
				local i=1
				while t[i] do -- cant use ipairs or # as this data may only exist via the metatabel
					d.it=t[i]
					tt[#tt+1]=wstr.macro_replace(d[plate] or plate,d)
					i=i+1
				end
				d.it=it
				return table.concat(tt)
			else -- just one thing (or fully dump the list)
				local tt
				if plate then -- how to format
					local it=d.it
					d.it=t
					tt=wstr.macro_replace(d[plate] or plate,d)
					d.it=it
				else
					tt=wstr.dump(t)
				end
				return tt
			end
			return nil -- no not expand
		end
		return tostring(t) -- simple find, make sure we return a string
	end
end


-----------------------------------------------------------------------------
--
-- replace {tags} in the string with data provided
-- allow sub table look up with a.b notation in the name
--
-----------------------------------------------------------------------------
wstr.replace=function(a,d)

return (string.gsub( a , "{([%w%.%_%-%:]-)}" , function(a) -- find only words and "._-:" tightly encased in {}
-- this means that almost all legal use of {} in javascript will not match at all.
-- Even when it does (probably as a "{}") then it is unlikley to accidently find anything in the d table
-- so the text will just be returned as is.
-- So it may not be safe, but it is simple to understand and perfecty fine under most use cases.
-- Note: nothing is ever safe...
	return wstr.replace_lookup(a,d) or ("{"..a.."}") -- otherwise no change
end )) -- note gsub is in brackes so we just get its first return value

end

-----------------------------------------------------------------------------
--
-- like replace but allows for simple creation of temporary substitutions
-- this enables very simple macro expansion
-- so {var=}value{=var} would set var to value
-- and that value would last for the rest of the chunk
--
-----------------------------------------------------------------------------
wstr.macro_replace_once = function(text,old_d,opts)
	if not text then return nil end
	
	opts=opts or {}
	local opts_clean=opts.clean
	local opts_htmldbg=opts.dbg_html_comments

	local d={} -- we can store temporary vars in here
	if old_d then setmetatable(d,{__index=old_d})	end -- wrap original d to protect it
	
	
	local ret={}
	
	local validchars = "[%w%.%_%-%=%:]-"
	local separator = "{"..validchars.."}"
	local capture = "{("..validchars..")}"
	
	local parts = {}  
	local start = 1
	
	local split_start, split_end = text:find(separator, start)
	
	while split_start do
		if split_start>1 then table.insert(parts, text:sub(start, split_start-1)) end		-- part1
		table.insert(parts, text:sub(split_start, split_end))	-- part2
		start = split_end + 1
		split_start, split_end = text:find(separator, start)
	end
	
	if text:sub(start)~="" then
		table.insert(parts, text:sub(start) )
	end

	local count=0
	local capt=nil
	

-- step through	
	for i=1,#parts do local v=parts[i]
		local tag=nil
		local dat=nil
		local skip_capt=nil
		if string.len(v)>=3 then -- must be at least this long
			local fc=v:sub(1,1) -- first char
			local lc=v:sub(-1) -- last char
			if fc=="{" and lc=="}" then -- special part
				tag=v:sub(2,#v-1)
				local fc=tag:sub(1,1) -- first char
				local lc=tag:sub(-1) -- last char
				
				local checkminus=function()
					if fc=="-" then -- need to return nothing if chunk is empty
						-- perform a single level check, every replacement must exist
						if dat and not wstr.replace_lookup_istable(tag,d) then -- but cant check tables properly so skip them
							string.gsub( dat , capture , function(a)
								if a:sub(1,1)=="-" then return "" end
								if not wstr.replace_lookup(a,d) then dat=nil end -- clear if not found
								return ""
							end )
						end
						dat=dat or "" -- may totally remove chunk if not found
					end
				end
				
				if lc=="=" then -- start of capture
					if capt==nil then
						capt=tag:sub(1,-2)
						d[capt]=""
						skip_capt=true
						if opts_clean then
							dat=""
						end
					end
				elseif fc=="=" then -- end of capture
					if capt==tag:sub(2) then -- must match
						capt=nil
						if opts_clean then dat="" end
					end
				else -- normal lookup					
					if opts.escape then -- only tags that begin with . or table replacements
						if fc=="." then
							dat=wstr.replace_lookup(tag,d)
						else
							if wstr.replace_lookup_istable(tag,d) then -- do table replaces last
								dat=wstr.replace_lookup(tag,d)
								checkminus()
							end
						end
					else -- only tags that dont begin with .
						if fc~="." then
							if not wstr.replace_lookup_istable(tag,d) then -- leave table replaces until last
								dat=wstr.replace_lookup(tag,d)
								checkminus()
							end
						end
					end
				end
			end
		end
		local s
		if dat then
			count=count+1
			if opts_htmldbg and tag then -- insert html comments so we can "debug" sourcode
				s="<!--{ "..tag.." }-->\n"..dat
			else
				s=dat
			end
		else
			s=v
		end
		
		if not skip_capt then
			if capt then -- record capture
				d[capt]=d[capt]..s
				if opts_clean then s="" end
			end
		end
		
		ret[#ret+1]=s
	end

	return table.concat(ret,""),count
end

wstr.macro_replace = function(a,d,opts)

local opts=opts or {} --{dbg_html_comments=true} to include html dbg, this will break some macro use inside javascript or html attributes so is off by default turn on to dbg
	
	local ret=a
	local count=0

	opts.clean=false
	opts.escape=false
	for i=1,100 do -- maximum recursion
	
		ret,count=wstr.macro_replace_once(ret,d,opts)
		
		if count==0 then break end -- nothing left to replace

	end
	opts.clean=true -- a final cleanup of inline assignments
	opts.escape=true -- a final substitution of escaped chunks
	ret=wstr.macro_replace_once(ret,d,opts) -- finally remove temporary chunks

	return ret
end


-----------------------------------------------------------------------------
--
-- wrap a string to a given width, merging all whitespace to spaces but keeping line breaks
-- return a table of lines
--
-----------------------------------------------------------------------------
wstr.smart_wrap=function(s,w)
	local ls=wstr.split_whitespace(s)
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
		
			for i,v in string.gmatch(v,"\n") do -- keep newlines
				newline()
			end
		
		else -- a normal word
		
			if #line>0 then wide=wide+1 end

			if wide + #v > w then -- split
				newline()
			end
			
			line[#line+1]=v
			wide=wide+#v
			
		end
	end
	if wide~=0 then newline() end -- final newline
	
	return t
end
