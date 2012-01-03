-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local ngx=require("ngx")

local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")

module(...)

--------------------------------------------------------------------------------
--
-- build a new srv structure for this request
--
--------------------------------------------------------------------------------
function new()
	srv={}
	
	srv.put=function(...)
		ngx.print(...)
	end
	
	srv.exit=function(n)
--		ngx.exit(n)
		return n
	end
	
	srv.set_header=function(n,v)
--		log("srv.set_header:",n,"=",v)
		ngx.header[n] = v;
	end
	
	srv.set_mimetype=function(v)	
		srv.set_header("content_type",v)
--		log("srv.set_mimetype:",v)
	end
	
	srv.set_cookie=function(n,v,tt)
		log("srv.set_cookie:",n,v)
		local t=os.time()+ (tt or (60*60))
		
--		ngx.header["Set-Cookie"]=ngx.header["Set-Cookie"] or {}
--		ngx.header["Set-Cookie"][ #(ngx.header["Set-Cookie"]) +1 ]=n.."="..v.."; Expires="..ngx.cookie_time(t)
		
		ngx.header["Set-Cookie"]=n.."="..v.."; Expires="..ngx.cookie_time(t)
	end

	srv.redirect=function(url)
--		log("srv.redirect:",url)
		return ngx.redirect(url)
	end

	srv.reloadcache=function(...)
		log("srv.reloadcache:",...)
	end

	srv.cache={} -- a very local cache
	
	srv.method=ngx.var.request_method -- probably GET or POST
	
	srv.ip=ngx.var.remote_addr -- users IP
	


	local scheme=ngx.var.scheme			-- http or https
	local domain=ngx.var.server_name
	local port=ngx.var.server_port
	local uri=ngx.var.uri				-- begins with /
	
	if port and port~="80" then
	
		srv.url=scheme.."://"..domain..":"..port..uri -- the url requested (not including any query string)

	else

		srv.url=scheme.."://"..domain..uri -- the url requested (not including any query string)

	end
	
	srv.query=ngx.var.args -- the query string
--log(srv.query)
	
	
	srv.headers=ngx.req.get_headers()
	
	srv.cookies={}
	local cs
	if type(srv.headers.Cookie=="string") then cs={srv.headers.Cookie} else cs=srv.headers.Cookie end
	log(srv.headers.Cookie)
	for i,s in ipairs(cs or {}) do
		local n,v=wstr.split_equal(s)
		srv.cookies[n]=v
	end

	ngx.req.read_body()
	srv.posts=ngx.req.get_post_args()

	srv.uploads={}

	srv.gets=ngx.req.get_uri_args()

	srv.vars={}
	for key, val in pairs( srv.posts ) do
		srv.vars[key]=val
	end
	for key, val in pairs( srv.gets ) do
		srv.vars[key]=val
	end

	return srv
end
