--
-- (C) 2013 Kriss@XIXs.com
--

local table=table
local ipairs=ipairs
local pairs=pairs
local string=string
local math=math
local os=os
local print=print
local tonumber=tonumber



-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
local M={ modname=(...) } ; package.loaded[M.modname]=M


-----------------------------------------------------------------------------
--
-- split a string into a table
--
-----------------------------------------------------------------------------
local function str_split(div,str,enable_special_chars)

  if (div=='') or not div then error("div expected", 2) end
  if (str=='') or not str then error("str expected", 2) end
  
  local pos,arr = 0,{}
  
  -- for each divider found
  for st,sp in function() return string.find(str,div,pos,not enable_special_chars) end do
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

-----------------------------------------------------------------------------
--
-- replace any %xx with the intended char, eg "%20" becomes a " "
--
-----------------------------------------------------------------------------
function M.url_decode(str)
    return string.gsub(str, "%%(%x%x)", function(hex)
        return string.char(tonumber(hex, 16))
    end)
end

-----------------------------------------------------------------------------
--
-- replace % , & and = chars with %xx codes
--
-----------------------------------------------------------------------------
function M.url_encode(str)
    return string.gsub(str, "([&=%%])", function(c)
        return string.format("%%%02x", string.byte(c))
    end)
end


-----------------------------------------------------------------------------
--
-- decode a string into a msg
-- if last is passed in then this table is adjusted rather than a new table being created
--
-----------------------------------------------------------------------------
function M.str_to_msg(str,last)
local msg=last or {}
local arr
local set

	arr=str_split("&",str)
	
	for i,v in ipairs(arr) do
	
		if v~="" then
		
			set=str_split("=",v)
			
			if set[1] and set[2] then
			
				msg[ set[1] ]=M.url_decode(set[2])
			end
			
		end
	end

	return msg
end


-----------------------------------------------------------------------------
--
-- encode a msg into a string
-- if last is available then only *changes* from msg to last are encoded, last is also updated with these changes
-- this gives a very simple delta compression
--
-----------------------------------------------------------------------------
function M.msg_to_str(msg,last)
local str
local line="&"

	for i,v in pairs(msg) do
	
		if (not last) or (last[i]~=v) then -- only store changes in string
		
			line=line.. i .."=".. M.url_encode(v) .."&"
			
			if last then last[i]=v end
			
		end
		
	end
	
	return line
end

