-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.ngx.log").log
local debug=require("debug")


local cache=require("wetgenes.www.ngx.cache")



local ngx=require("ngx")

-- does this overloading let us just use the socket module?
--[[
local socket=assert(require("socket"))
socket.tcp=ngx.socket.tcp
socket.udp=ngx.socket.udp
socket.connect=ngx.socket.connect
]]

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.serv"]=_M

function serv()
	xpcall(serv2,function(msg,lev)
		log( msg )
		log( debug.traceback() )
	end)
end

-- work out which is our vhost but do not do any more setup, returns srv
function serv_srv()

	local opts=require("opts")
	if opts.setup then opts.setup() end -- may need to in itialize stuff

	local srv=require("wetgenes.www.ngx.srv").new()
	ngx.ctx=srv -- this is out ctx
	
	for i,v in ipairs(opts.vhosts_map or {} ) do
		srv.vhost=v[2]
		if ngx.var.host:find(v[1]) then break end
	end	

	return srv
	
end

function serv2()

	local opts=require("opts")
	local srv=serv_srv()
	
--	if srv.vhost then log("VHOST = "..srv.vhost) end
	
	srv.opts=function(...)
		local t=opts.vhosts[srv.vhost] or opts
		for i,v in ipairs({...}) do
			t=t and t[v]
		end
		return t
	end
	
	-- shove this basic functions into the global name space
	-- they will work with the opts to serv this app as needed
	local basic=require("base.basic")
	

	if not srv.opts().require_all_done then
		srv.opts().require_all_done=true

--		log("require all mods")

		for n,v in pairs(opts.mods) do
			if type(n)=="string" then
--				log("require "..n)
				local m,err=pcall(require,n)
				if not m then
					log("require failed on mod "..n.."\n"..(err or ""))
				end
			end
		end
		
	end

	
	basic.serv(srv)

end
