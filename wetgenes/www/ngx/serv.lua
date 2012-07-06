-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.ngx.log").log
local debug=require("debug")


local ngx=require("ngx")

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.serv"]=_M

function serv()
	xpcall(serv2,function(msg,lev)
		log( msg )
		log( debug.traceback() )
	end)
end

function serv2()

	local opts=require("opts")


	local srv=require("wetgenes.www.ngx.srv").new()
	ngx.ctx=srv -- this is out ctx
	
	if opts.vhosts_map then
		srv.vhost=opts.vhosts_map[1][2]
		for i,v in ipairs(opts.vhosts_map) do
			if ngx.var.host:find(v[1]) then
				srv.vhost=v[2]
				break
			end
		end
	end
	
	
	if srv.vhost then log("VHOST = "..srv.vhost) end
	
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

		log("require all mods")

		for n,v in pairs(opts.mods) do
			if type(n)=="string" then
				log("require "..n)
				local m,err=pcall(require,n)
				if not m then
					log("require failed on mod "..n.."\n"..(err or ""))
				end
			end
		end
		
	end
	
	basic.serv(srv)

end
