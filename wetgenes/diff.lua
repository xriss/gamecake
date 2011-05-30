-----------------------------------------------------------------------------
-- Provides functions for diffing text.
--
-- (c) 2007, 2008  Yuri Takhteyev (yuri@freewisdom.org)
-- (c) 2007 Hisham Muhammad
--
-- License: MIT/X, see http://sputnik.freewisdom.org/en/License
--
-- the above was used as a base, it is no longer the same code...
--
-- this diff now returns a different slightly more compressed format
-- which is a table of numbers and strings of only the differences
-- and that format can be used to convert either string into the other string
--
-- most of the changes are actually commented and are just mild changes really
--
-----------------------------------------------------------------------------


local string=string
local table=table
local math=math

local assert=assert
local setmetatable=setmetatable
local ipairs=ipairs
local type=type

module("wetgenes.diff")

-- java version of lua, not so good at strings, so might as well use numbers for these as they are now internal only
local SAME = 0  -- token statuses
local IN   = 1
local OUT  = 2

-----------------------------------------------------------------------------
-- Split a string into tokens.  (Adapted from Gavin Kistner's split on
-- http://lua-users.org/wiki/SplitJoin.
--
-- @param text           A string to be split.
-- @param separator      [optional] the separator pattern (defaults to any
--                       white space - %s+).
-- @param skip_separator [optional] don't include the sepator in the results.     
-- @return               A list of tokens.
-----------------------------------------------------------------------------
local function split(text, separator, skip_separator)
	separator = separator or "%s+"
	local parts = {}  
	local start = 1
	local split_start, split_end = text:find(separator, start)
	while split_start do
		if skip_separator then
			table.insert(parts, text:sub(start, split_start-1))
		else
			table.insert(parts, text:sub(start, split_end)) -- now includes the seperator *inside* each token
		end
		start = split_end + 1
		split_start, split_end = text:find(separator, start)
	end
	if text:sub(start)~="" then
		table.insert(parts, text:sub(start) )
	end
	return parts
end


-----------------------------------------------------------------------------
-- Derives the longest common subsequence of two strings.  This is a faster
-- implementation than one provided by stdlib.  Submitted by Hisham Muhammad. 
-- The algorithm was taken from:
-- http://en.wikibooks.org/wiki/Algorithm_implementation/Strings/Longest_common_subsequence
--
-- @param t1             the first string.
-- @param t2             the second string.
-- @return               the least common subsequence as a matrix.
-----------------------------------------------------------------------------
local function quick_LCS(t1, t2)
   local m = #t1
   local n = #t2

   -- Build matrix on demand
   local C = {}
   local setmetatable = setmetatable
   local mt_tbl = {
      __index = function(t, k)
         t[k] = 0
         return 0
      end
   }
   local mt_C = {
      __index = function(t, k)
         local tbl = {}
         setmetatable(tbl, mt_tbl)
         t[k] = tbl
         return tbl
      end
   }
   setmetatable(C, mt_C)
   local max = math.max
   for i = 1, m+1 do
      local ci1 = C[i+1]
      local ci = C[i]
      for j = 1, n+1 do
         if t1[i-1] == t2[j-1] then
            ci1[j+1] = ci[j] + 1
         else
            ci1[j+1] = max(ci1[j], ci[j+1])
         end
      end
   end
   return C
end

-----------------------------------------------------------------------------
-- Returns a diff of two strings as a list of pairs, where the first value
-- represents a token and the second the token's status ("same", "in", "out").
--
-- @param old             The "old" text string
-- @param new             The "new" text string
-- @param separator      [optional] the separator pattern (defaults ot any
--                       white space).
-- @return               A list of annotated tokens.
-----------------------------------------------------------------------------
local function rev_diff(old, new, separator)
   assert(old); assert(new)
   new = split(new, separator); old = split(old, separator)

   -- First, compare the beginnings and ends of strings to remove the common
   -- prefix and suffix.  Chances are, there is only a small number of tokens
   -- in the middle that differ, in which case  we can save ourselves a lot
   -- in terms of LCS computation.
   local prefix = "" -- common text in the beginning
   local suffix = "" -- common text in the end
   while old[1] and old[1] == new[1] do
      local token = table.remove(old, 1)
      table.remove(new, 1)
      prefix = prefix..token
   end
   while old[#old] and old[#old] == new[#new] do
      local token = table.remove(old)
      table.remove(new)
      suffix = token..suffix
   end

   -- Setup a table that will store the diff (an upvalue for get_diff). We'll
   -- store it in the reverse order to allow for tail calls.  We'll also keep
   -- in this table functions to handle different events.
   local rev_diff = {
      put  = function(self, token, type) table.insert(self, token ) table.insert(self, type ) end, -- no sub tables, so less resources, probably
      ins  = function(self, token) self:put(token, IN) end,
      del  = function(self, token) self:put(token, OUT) end,
      same = function(self, token) if token then self:put(token, SAME) end end,
   }

   -- Put the suffix as the first token (we are storing the diff in the
   -- reverse order)

   rev_diff:same(suffix)

   -- Define a function that will scan the LCS matrix backwards and build the
   -- diff output recursively.
   local function get_diff(C, old, new, i, j)
      local old_i = old[i]
      local new_j = new[j]
      if i >= 1 and j >= 1 and old_i == new_j then
         rev_diff:same(old_i)
         return get_diff(C, old, new, i-1, j-1)
      else
         local Cij1 = C[i][j-1]
         local Ci1j = C[i-1][j]
         if j >= 1 and (i == 0 or Cij1 >= Ci1j) then
            rev_diff:ins(new_j)
            return get_diff(C, old, new, i, j-1)
         elseif i >= 1 and (j == 0 or Cij1 < Ci1j) then
            rev_diff:del(old_i)
            return get_diff(C, old, new, i-1, j)
         end
      end
   end
   -- Then call it.
   get_diff(quick_LCS(old, new), old, new, #old + 1, #new + 1)

   -- Put the prefix in at the end
   rev_diff:same(prefix)

   return rev_diff
end



-----------------------------------------------------------------------------
-- Returns a diff of two strings as a list of pairs, where the first value
-- represents a token and the second the token's status ("same", "in", "out").
--
-- @param old             The "old" text string
-- @param new             The "new" text string
-- @param separator      [optional] the separator pattern (defaults ot any
--                       white space).
-- @return
--
-- a table containg a list of the following commands
-- if a number then the length of string that is the same and should be skipped
-- if a string then it is part of a pair of strings
-- [1] contains the old string and [2] contains the new string
-- strings are always in pairs and numbers are always seperated by two strings
-- this should be reasonably compact data
--
-----------------------------------------------------------------------------
function diff(old, new, separator)

	local d=rev_diff(old, new, separator)

	local t={}
	
	local same,new,old
	
	local function reset() same=0 new="" old="" end
	
	reset()
	

-- scan the data from diff_base
	for i = #d , 2 , -2 do -- data comes in reversed and in pairs
	
	
	local v1=d[i] 		-- command
	local v2=d[i-1] 	-- data
	

		if v1==SAME then
		
			if ( new~="" or old~="" ) then -- change state
			
				table.insert(t,old)
				table.insert(t,new)
				reset()
			end
			
			same=same+v2:len()
		
		elseif v1==OUT then 
		
			if same>0 then -- change state
			
				table.insert(t,same)
				reset()
			end
			
			old=old..v2
			
		elseif v1==IN then
		
			if same>0 then -- change state
			
				table.insert(t,same)
				reset()
			end
			
			new=new..v2			
		end
		
	end

-- add the final chunk
	
	if same>0 then
	
		table.insert(t,same)
		
	elseif ( new~="" or old~="" ) then

		table.insert(t,old)
		table.insert(t,new)
	end

	return t
end


-----------------------------------------------------------------------------
--
-- patch a diff against a string and return the new string
--
-- set undo to true and the patch will be applied in reverse
--
-- s1,s2
-- d=diff(s1,s2)
-- s2==patch(s1,d)
-- s1==patch(s2,d,true)
--
-----------------------------------------------------------------------------
function patch(s,t,undo)

	local o={}
	local idx=1

	local i=1
	local len=#t
	while i<=len do
	
		local v1,v2
		v1=t[i]
		i=i+1
		
		if type(v1)=="string" then
		
			v2=t[i]
			i=i+1	-- strings come in pairs so get second string
			
			if undo then
			
				table.insert(o, v1 )
				idx=idx+v2:len()
				
			else
			
				table.insert(o, v2 )
				idx=idx+v1:len()
				
			end
		else
		
			table.insert(o, s:sub(idx,idx+v1-1) )
			idx=idx+v1
			
		end
	end
	
	return table.concat(o) -- build a return string

end

-- a more readable way to unpatch
function unpatch(s,t) return patch(s,t,true) end

