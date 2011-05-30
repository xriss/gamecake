
local table=table
local string=string

local type=type
local tostring=tostring
local setmetatable=setmetatable

-- my string functions
local str=require("wetgenes.string")

module("wetgenes.html")

--
-- use the replace function from wetgenes.string
--
replace=str.replace


-----------------------------------------------------------------------------
--
-- build a string from a template,  with a table to be used as its environment
--
-- this environment will not get modified by the called function as it is wrapped here
--
-- even though the calling function is free to modify the table it gets
--
-----------------------------------------------------------------------------
get=function(html,src,env)

	local new_env={}
	if env then setmetatable(new_env,{__index=env})	end -- wrap to protect

	if html[src] then src=html[src] end
	
	if type(src)=="function" then return src(new_env) end
	
	if type(src)=="string" and env then return replace(src,new_env) end

	return tostring(src)
end


-----------------------------------------------------------------------------
--
-- very basic html esc to stop tags and entities from doing bad things
-- running text submitted from a user through this function should stop it from doing
-- anything other than just being text, it doesnt guarantee that it is valid xhtml / whatever
-- We just turn a few important characters into entities.
--
-----------------------------------------------------------------------------
function esc(s)
	local escaped = { ['<']='&lt;', ['>']='&gt;', ["&"]='&amp;' }
	return (s:gsub("[<>&]", function(c) return escaped[c] end))
end

-----------------------------------------------------------------------------
--
-- basic url escape, so as not to trigger url get params or anything else by mistake 
-- so = & # % ? " ' are bad and get replaced with %xx
--
-----------------------------------------------------------------------------
function url_esc(s)
	return string.gsub(s, "([&=%%%#%?%'%\" ><])", function(c)
		return string.format("%%%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- a url escape, that only escapes the string deliminators ' and " 
--
-----------------------------------------------------------------------------
function url_esc_string(s)
	return string.gsub(s, "(['%\" ])", function(c)
		return string.format("%%%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- convert any %xx into single chars
--
-----------------------------------------------------------------------------
function url_unesc(s)
	return string.gsub(s, "%%(%x%x)", function(hex)
		return string.char(tonumber(hex, 16))
	end)
end

