--
-- Some cgilua helper functions
--

local cgi
local cgilua=cgilua

local string=string
local table=table
local math=math

local type=type
local pairs=pairs
local setfenv=setfenv

local socket=require("socket")
local misc=require("wetgenes.cgilua.misc")

module("wetgenes.cgilua")
cgi=_M -- cgi should point to this module

-----------------------------------------------------------------------------
--
-- print a debug string somewhere useful
--
-----------------------------------------------------------------------------
dbg=function(s)
	cgilua.errorlog(s)
	if not headers_sent then 
		contentheader("text", "html")
	end
	cgilua.put(s)
end

-----------------------------------------------------------------------------
--
-- just a copy of some cgilua put
--
-----------------------------------------------------------------------------
put=function(s)
	cgilua.put(s)
end
header=function(a,b)
	cgilua.header(a,b)
end

-----------------------------------------------------------------------------
--
-- Set content type and remember headers have been sent
--
-----------------------------------------------------------------------------
function contentheader(type, subtype)

	if headers_sent then return end -- so this is safe to call multiple times
	
	headers_sent=true -- flag
	
	cgilua.contentheader(type, subtype)

end


-----------------------------------------------------------------------------
--
-- redirect
-- either with headers or throw out some javascript if that is too late
--
-----------------------------------------------------------------------------
function redirect(url)

	if headers_sent then

		cgilua.put([[<script type="text/javascript"> window.location = ]]..string.format("%q",url)..[[; </script>]])

	else

		cgilua.redirect(url)

	end

end


-----------------------------------------------------------------------------
--
-- Build some always useful information about this request
-- into this module
--
-----------------------------------------------------------------------------
function setup() -- call setup once to set global values for this request?

	start_time=socket.gettime()
	query_count=0
	
	math.randomseed( math.floor(start_time*1000) )
	math.random()

	headers_sent=false

	path=cgilua.servervariable("SCRIPT_NAME")..cgilua.servervariable("PATH_INFO")

	server=cgilua.servervariable("SERVER_NAME")

	query=cgilua.servervariable("QUERY_STRING")

	ip=cgilua.servervariable("REMOTE_ADDR")
	ipnum=misc.ipstr_to_number(ip)

	url="http://"..server..path
	url_query=url

	if query and query~="" then url_query=url.."?"..query end
	-- url_query is now probably full if we need to redirect to ourselves (which we often do)
	
	slash=misc.str_split("/",url) --  a normally useful array

	-- the lua query/post are dangerous as they may contain tables...
	-- this creates safe copies that are only strings

	gets={}
	for i,v in pairs(cgilua.QUERY) do
		if type(i)=="string" and type(v)=="string" then -- do not allow tables to break simple code
			gets[i]=v
		end
	end

	posts={}
	for i,v in pairs(cgilua.POST) do
		if type(i)=="string" and type(v)=="string" then -- do not allow tables to break simple code
			posts[i]=v
		end
	end

	json=nil
	if gets.fmt=="json" or posts.fmt=="json" then -- a special json request, start building the return
		json={}
		json.doups={}
	end
	
	return _M
end
