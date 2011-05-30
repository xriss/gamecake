
local assert,error,getfenv,getmetatable,ipairs,loadstring,next,pairs,pcall,print,rawget,rawset,setfenv,setmetatable,tonumber,tostring,type,unpack,xpcall = assert,error,getfenv,getmetatable,ipairs,loadstring,next,pairs,pcall,print,rawget,rawset,setfenv,setmetatable,tonumber,tostring,type,unpack,xpcall

local string,table=string,table



module("wetgenes.cgilua.misc")


-----------------------------------------------------------------------------
--
-- convert an ipstr "a.b.c.d" to a number
--
-----------------------------------------------------------------------------
function ipstr_to_number(str)

local num=0

	for word in string.gmatch(str, "[^.]+") do num=num*256+tonumber(word) end
	
	return num

end
-----------------------------------------------------------------------------
--
-- convert a number to an ipstr "a.b.c.d"
--
-----------------------------------------------------------------------------
function number_to_ipstr(num)

local h,s

	h=string.format("%08x",num)

	s=tonumber( string.sub(h,1,2) , 16 ) .. "." ..
	  tonumber( string.sub(h,3,4) , 16 ) .. "." ..
	  tonumber( string.sub(h,5,6) , 16 ) .. "." ..
	  tonumber( string.sub(h,7,8) , 16 ) 

	return s
end


-----------------------------------------------------------------------------
--
-- split a string into a table
--
-----------------------------------------------------------------------------
function str_split(div,str)

  if (div=='') or not div then error("div expected", 2) end
  if not str then error("str expected", 2) end
  if str=="" then return {""} end
  
  local pos,arr = 0,{}
  
  -- for each divider found
  for st,sp in function() return string.find(str,div,pos,true) end do
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
-- join a table of things into an english list with commas and an "and" at the end
-- returns nil if the table is empty
--
-----------------------------------------------------------------------------
function str_join_english_list(t)

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
function str_to_hex(s)
	return string.gsub(s, ".", function (c)
		return string.format("%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- escape a string for mysql, convert to a (possibly large) number in hex
--
-----------------------------------------------------------------------------
function mysql_escape(s)
	return "0x"..str_to_hex(s)
end


-----------------------------------------------------------------------------
--
-- replace any %xx with the intended char, eg "%20" becomes a " "
--
-----------------------------------------------------------------------------
function url_decode(str)
    return string.gsub(str, "%%(%x%x)", function(hex)
        return string.char(tonumber(hex, 16))
    end)
end

-----------------------------------------------------------------------------
--
-- replace % , & and = chars with %xx codes
--
-----------------------------------------------------------------------------
function url_encode(str)
    return string.gsub(str, "([&=%%])", function(c)
        return string.format("%%%02x", string.byte(c))
    end)
--  return str
end
