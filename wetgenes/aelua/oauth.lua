

local wet_html=require("wetgenes.html")

local sys=require("wetgenes.aelua.sys")

local json=require("wetgenes.json")
local dat=require("wetgenes.aelua.data")

local users=require("wetgenes.aelua.users")

local fetch=require("wetgenes.aelua.fetch")

local img=require("wetgenes.aelua.img")

local log=require("wetgenes.aelua.log").log -- grab the func from the package

local wet_string=require("wetgenes.string")
local str_split=wet_string.str_split
local serialize=wet_string.serialize


local math=math
local string=string
local table=table
local os=os

local ipairs=ipairs
local pairs=pairs
local tostring=tostring
local tonumber=tonumber
local type=type
local pcall=pcall
local loadstring=loadstring


module(...)

-----------------------------------------------------------------------------
--
-- The escape function for oauth must be exactly this, RFC3986
-- it would be nice if / was not escaped since base64 is also used all over the place
-- but that is not the case.
--
--
-----------------------------------------------------------------------------
function esc(s)
	return string.gsub(s,'[^0-9A-Za-z%-._~]', -- RFC3986 happy chars
		function(c) return ( string.format("%%%02X", string.byte(c)) ) end )
end
-----------------------------------------------------------------------------
--
-- unesc performs the oposite of esc
--
-----------------------------------------------------------------------------
function unesc(str)
    return string.gsub(str, "%%(%x%x)", function(hex)
        return string.char(tonumber(hex, 16))
    end)
end

-----------------------------------------------------------------------------
--
-- perform a hmac_sha1 that is oauth friendly, key is an RFC3986 string, result is a base64 hash
--
-- this uses some java crypto functions exposed in sys for dealing with hmac and sha1
--
-----------------------------------------------------------------------------
function hmac_sha1(key,str)
	local bin=sys.hmac_sha1(key,str,"bin") -- key gets used as is, (string is 7bit safe).
	local b64=sys.bin_encode("base64",bin) -- we need to convert the resuilt to base64
	return b64
end

-----------------------------------------------------------------------------
--
-- get the current time and build a nonce for it using a passed in key value
-- which i do not think has to be terribly secret, we reuse hmac_sha1
-- just to keep the number of strange functions used low, really this could generate anything
--
-----------------------------------------------------------------------------
function time_nonce(key) -- pass in a secret key
	local t=math.floor(os.time())
	local n=sys.bin_encode("hex",sys.hmac_sha1(key,tostring(t).."&aelua.dumid.oauth","bin"))
	return t,n -- returns time,nonce
end

-----------------------------------------------------------------------------
--
-- vars contains the variables we intend to send to the oauth server
-- opts contains extra options:
--  post should be set to GET if we do not intend to use POST
--  url should be the oauth server url for this request
--  secret should be the token secret provided by the oauth server or nil for initial signin
--  api_secret should be your special api/consumer secret
-- these opts are used to build a request string from the vars and sign it so that
-- the oauth server will be happy with it
-- it returns a base64 signature , request string
-- the request string could be used in the body of a POST (default)
-- or if you set opts.post="GET" then you could the use it in a GET request
--
-- aparently it is better to put this stuff in the header, Authorization:
-- but I have my doubts
--
-----------------------------------------------------------------------------
function build(vars,opts)

	local post      =opts.post       or "POST" 
	local url       =opts.url        or ""
	local tok_secret=opts.tok_secret or ""
	local api_secret=opts.api_secret or ""

	local vals={}

-- esc and shove all oauth vars into vals
	for i,v in pairs(vars) do
		vals[#vals+1]={esc(i),esc(v)} -- record a simple table , i==[1] v==[2]
	end

	table.sort(vals, function(a,b)
			if a[1]==b[1] then return a[2]<b[2] end -- sort by [2] if [1] is the same
			return a[1]<b[1] -- other wise sort by [1]
		end)
		
-- now they are in the right order, build the query string

	for i=1,#vals do local v=vals[i]
		vals[i]=v[1].."="..v[2]
	end
	local query=table.concat(vals,"&")
	local base=post.."&"..esc(url).."&"..esc(query) -- everything always gets escaped
	local key=esc(api_secret).."&"..esc(tok_secret) -- the key is built from these strings
	local sign=hmac_sha1(key,base)
	
	return sign , query.."&oauth_signature="..esc(sign) -- sign it
end

-----------------------------------------------------------------------------
--
-- turns a responce string into a table of values, this is the oposite of build
-- and intended to deal with the results
--
-----------------------------------------------------------------------------
function decode(s)
	local ret={}
	local aa=str_split("&",s) -- first split on &
	for i=1,#aa do local v=aa[i]
		local a,b = wet_string.split_equal(v)
--log("decode : "..v.." : "..type(a).." : "..type(b))		
		ret[unesc(a)]=unesc(b) -- unescape both sides
	end
	return ret -- return a lookup table, which may be empty
end




