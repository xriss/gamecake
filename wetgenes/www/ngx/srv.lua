-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local ngx=require("ngx")

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
	
	srv.set_header=function(...)
		ngx.print(...)
	end
	
	srv.set_mimetype=function(...)
		ngx.print(...)
	end
	
	srv.set_cookie=function(...)
		ngx.print(...)
	end

	srv.redirect=function(...)
		ngx.print(...)
	end

	srv.reloadcache=function(...)
		ngx.print(...)
	end

	srv.cache={} -- a very local cache
	
	srv.method=ngx.var.request_method -- probably GET or POST
	
	srv.ip=ngx.var.remote_addr -- users IP
	
	srv.url=ngx.var.uri -- the url requested (not including any query string)
	
	srv.query=ngx.var.args -- the query string
	
	srv.cookies={}
	
	srv.headers={}
	
	srv.posts={}

	srv.uploads={}

	srv.gets={}

	srv.vars={}

	return srv
end
