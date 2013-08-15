
-- manage simple rate limiting by ip, this will stop run away scripts from taking the piss
-- these structures are stored only in memcache so are very fuzzy/lazy and not to be trusted
-- but should mostly work, for now this is just rate limiting but a callback to handle extras
-- will be added when I work out what these extras might be

local cache=require("wetgenes.www.any.cache")
local json=require("wetgenes.json")
local wstr=require("wetgenes.string")

local log=require("wetgenes.www.any.log").log -- grab the func from the package


local tostring=tostring
local math=math
local os=os

module("wetgenes.www.any.iplog")

--------------------------------------------------------------------------------
--
-- turn a time in seconds into
--
-- time in : minutes , hours , days
--
--------------------------------------------------------------------------------
function get_mhd(s)
	return math.floor(s/(60)) , math.floor(s/(60*60)) , math.floor(s/(60*60*24))
end

--------------------------------------------------------------------------------
--
-- Create a new struct
--
--------------------------------------------------------------------------------
function create(ip)
	local it={}
	it.ip=ip
	it.mhd=	{ -- arrays for simple json tables
				{0,0,0}, -- count : minutes , hours , days
				{get_mhd(os.time())}, -- stamps : minutes , hours , days
			}
	return check(it)
end

--------------------------------------------------------------------------------
--
-- check data structure, count zero any old data as it goes out of date
--
--------------------------------------------------------------------------------
function check(it)
	local mhd={get_mhd(os.time())}
	if it.mhd[2][1]~=mhd[1] then it.mhd[1][1]=0 it.mhd[2][1]=mhd[1] end
	if it.mhd[2][2]~=mhd[2] then it.mhd[1][2]=0 it.mhd[2][2]=mhd[2] end
	if it.mhd[2][3]~=mhd[3] then it.mhd[1][3]=0 it.mhd[2][3]=mhd[3] end
	return it
end

--------------------------------------------------------------------------------
--
-- get or create if doesnt exist
--
--------------------------------------------------------------------------------
function manifest(ip)
	return get(ip) or create(ip)
end

--------------------------------------------------------------------------------
--
-- get by ip
--
--------------------------------------------------------------------------------
function get(ip)
	local d=cache.get(nil,"iplog="..ip)
	if d then
		return check(json.decode(d))
	end
	return nil
end

--------------------------------------------------------------------------------
--
-- put
--
--------------------------------------------------------------------------------
function put(it)
	cache.put(nil,"iplog="..it.ip , json.encode(it) , 60*60*24 )
end

--------------------------------------------------------------------------------
--
-- increment the current use by one or n if given
--
--------------------------------------------------------------------------------
function inc(it,n)
	it.mhd[1][1]=it.mhd[1][1]+(n or 1)
	it.mhd[1][2]=it.mhd[1][2]+(n or 1)
	it.mhd[1][3]=it.mhd[1][3]+(n or 1)
	return it
end

--------------------------------------------------------------------------------
--
-- this is the main ratelimit function you want, given an IP (string) mark up another access
-- and decide if we should bail on the request at this point, returns true if
-- it is ok to procede false if it isnt. The iplog structure is also in its second
-- return value for later inspection.
--
--------------------------------------------------------------------------------
function ratelimit(ip,n)
	local it=manifest(ip)
	inc(it,n)
	put(it) -- we do not care about overwrites, numbers are fuzzy
	if it.admin then return true,it end -- admin flag means it is always ok
	if it.mhd[1][1] > 200   then return false,it end -- max requests per minute
	if it.mhd[1][2] > 2000  then return false,it end -- max requests per hour
	if it.mhd[1][3] > 20000 then return false,it end -- max requests per day
	return true,it
end


--------------------------------------------------------------------------------
--
-- mark this ip as admin so it is never rate limited
--
--------------------------------------------------------------------------------
function mark_as_admin(ip)
	local it=manifest(ip)
	if not it.admin then -- switch on flag
		it.admin=true
		put(it) -- we do not care about overwrites, numbers are fuzzy
	end
end



--------------------------------------------------------------------------------
--
-- get some html info about this ip (for use on the 503 error page)
--
--------------------------------------------------------------------------------
function html_info(ip)
	local it=manifest(ip)

	return wstr.replace([[
<html> <title> Too Many Requests ( RATE LIMITED ) </title>
<body>
<center>
<h1>Your ip {ip} is being RATE LIMITED</h1>
<h3>You must wait a little while to access this server again.</h3>
<hr/>
<br/>
You have used {pm}% of your quota per minute.<br/>
<br/>
You have used {ph}% of your quota per hour.<br/>
<br/>
You have used {pd}% of your quota per day.<br/>
<br/>
<br/>
</center>
</body>
</html>
	]],{
		ip=ip,
		pm=math.floor(100*it.mhd[1][1]/200),
		ph=math.floor(100*it.mhd[1][2]/2000),
		pd=math.floor(100*it.mhd[1][3]/20000),
	})
end

