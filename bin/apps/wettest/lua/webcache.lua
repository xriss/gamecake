
local io=io
local table=table
local coroutine=coroutine

local ipairs=ipairs
local math=math
local string=string
local print=print
local require=require
local loadstring=loadstring
local setfenv=setfenv

local work=require("work")
local wetstr=require("wetgenes.string")

local wetlua=wetlua

module(...)

cache={}

-----------------------------------------------------------------------------
--
-- escape a url so it can be used as a filename
-- this is a oneway function, you cannot get the url back from the filename.
--
-----------------------------------------------------------------------------
function esc_url(str)
    return string.lower( string.gsub(str, "([\128-\255/\\:])", function(c)
        return "_"
    end) )
end


-----------------------------------------------------------------------------
--
-- get a url, use lua cache or disk cache or read from the web
--
-----------------------------------------------------------------------------
function get_url(url)

	local ret=cache[url] -- first try for memory cache
	
	if not ret then -- then try for disk cache
	
		local erl=esc_url(url)
		local fname=wetlua.dir.."local/cache/"..erl..".lua"
		local fp=io.open(fname,"rb")
		
		if fp then -- got some cache
		
			local s=fp:read("*all")
			fp:close()
			local f=loadstring("return ("..s..")")
			setfenv(f,{})
			ret=f()
			
		else -- finally hit the web server
		
			ret=work.get_url(url)
			
			local fp=io.open(fname,"wb")	-- and save into the disk cache
			if fp then
				fp:write( wetstr.serialize(ret) )
				fp:close()
			end
			
		end
		cache[url]=ret
	end
	return ret
end
