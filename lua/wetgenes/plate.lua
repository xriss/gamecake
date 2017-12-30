--
-- (C) 2014 Kriss@XIXs.com and released under the MIT license,
-- see http://opensource.org/licenses/MIT for full license text.
--

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- grab some util functions
local export,lookup,set_env=require("wetgenes"):export("export","lookup","set_env")

-- single line replacement for the module creation function
local M={} ; package.loaded[(...)]=M ; local wplate=M

--[[#lua.wetgenes.plate.table_lookup flag=1

	value=wetgenes.plate.table_lookup(name,data)

look up name inside data and return the value we found if we don't 
find anything then we return nil

name.name.name syntax may be used to reference tables within tables.

A name that looks like a number may be converted into a number if it 
doesnt exist as a string key, so "array.1" can be used to return the 
first item from an array.


]]

wplate.table_lookup=function(a,d) -- look up a in table d

	local t
	
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
		return wplate.table_lookup(a2,dd) -- tail call this function
	end
	
end

--[[#lua.wetgenes.plate.replace_lookup_istable

	bool=wetgenes.plate.replace_lookup_istable(name,data)

Test if the return from "wplate.table_lookup" is a table.


]]
wplate.replace_lookup_istable=function(a,d) -- check if this is a special table lookup (so we can leave them til
	local t=wplate.table_lookup(a,d)
	if t and type(t)=="table" then -- if a table then
		return true
	end
	return false
end

--[[#lua.wetgenes.plate.replace_lookup

	value=wetgenes.plate.replace_lookup(name,data)

Calls "wetgenes.plate.table_lookup" then performs special formatting on 
table returns.

Always returns a string or nil, so number values will converted to a 
string.


]]
wplate.replace_lookup=function(a,d) -- look up a in table d
	local t=wplate.table_lookup(a,d)
	if t then
		if type(t)=="table" then -- if a table then
			if t[1] then -- a list of stuff
				if t.plate then -- how to format
					local tt={}
					local it=d.it
					local i=1
					while t[i] do -- cant use ipairs or # as this data may only exist via the metatabel
						d.it=t[i]
						tt[#tt+1]=wplate.macro_replace(d[t.plate] or t.plate,d)
						i=i+1
					end
					d.it=it
					return table.concat(tt)
				end
			else -- just one thing
				if t.plate then -- how to format
					local it=d.it
					d.it=t
					local tt=wplate.macro_replace(d[t.plate] or t.plate,d)
					d.it=it
					return tt
				end
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
wplate.replace=function(a,d)

return (string.gsub( a , "{([%w%._%-]-)}" , function(a) -- find only words and "._-" tightly encased in {}
-- this means that almost all legal use of {} in javascript will not match at all.
-- Even when it does (probably as a "{}") then it is unlikley to accidently find anything in the d table
-- so the text will just be returned as is.
-- So it may not be safe, but it is simple to understand and perfecty fine under most use cases.
-- Note: nothing is ever safe...
	return wplate.replace_lookup(a,d) or ("{"..a.."}") -- otherwise no change
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
wplate.macro_replace_once = function(text,old_d,opts)
	if not text then return nil end
	
	opts=opts or {}
	local opts_clean=opts.clean
	local opts_htmldbg=opts.dbg_html_comments

	local d={} -- we can store temporary vars in here
	if old_d then setmetatable(d,{__index=old_d})	end -- wrap original d to protect it
	
	
	local ret={}
	
	local separator = "{[%w%._%-=]-}"
	
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
						if dat and not wplate.replace_lookup_istable(tag,d) then -- but cant check tables properly so skip them
							string.gsub( dat , "{([%w%._%-]-)}" , function(a)
								if a:sub(1,1)=="-" then return "" end
								if not wplate.replace_lookup(a,d) then dat=nil end -- clear if not found
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
							dat=wplate.replace_lookup(tag,d)
						else
							if wplate.replace_lookup_istable(tag,d) then -- do table replaces last
								dat=wplate.replace_lookup(tag,d)
								checkminus()
							end
						end
					else -- only tags that dont begin with .
						if fc~="." then
							if not wplate.replace_lookup_istable(tag,d) then -- leave table replaces until last
								dat=wplate.replace_lookup(tag,d)
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

wplate.macro_replace = function(a,d,opts)

local opts=opts or {} --{dbg_html_comments=true} to include html dbg, this will break some macro use inside javascript or html attributes so is off by default turn on to dbg
	
	local ret=a
	local count=0

	opts.clean=false
	opts.escape=false
	for i=1,100 do -- maximum recursion
	
		ret,count=wplate.macro_replace_once(ret,d,opts)
		
		if count==0 then break end -- nothing left to replace

	end
	opts.clean=true -- a final cleanup of inline assignments
	opts.escape=true -- a final substitution of escaped chunks
	ret=wplate.macro_replace_once(ret,d,opts) -- finally remove temporary chunks

	return ret
end

