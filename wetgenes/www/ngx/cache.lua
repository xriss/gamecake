-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")
local wbox=require("wetgenes.sandbox")

local ngx=ngx

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.cache"]=_M

--hax={}

function check_mc()
	if ngx.ctx.mc then return ngx.ctx.mc end
	
	ngx.ctx.mc=require("wetgenes.www.ngx.memcache").Connect() -- every request needs a new connection...

	ngx.ctx.mc:set_encode(function(s) local r=wstr.serialize(s) return r end) -- so we may store tables
	ngx.ctx.mc:set_decode(function(s) return wbox.lson(s) end)
	
	return ngx.ctx.mc
end

function countzero()
	count=0
	count_got=0
	api_time=0

end
countzero()

local start_time -- handle simple api benchmarking of some calls
local function apis()
	start_time=os.time()
end
local function apie(...)
	api_time=api_time+os.time()-start_time
	return ...
end

local function getvhost(srv)
	local vhost="data"
	if ngx and ngx.ctx and ngx.ctx.vhost then
		vhost=ngx.ctx.vhost
	end
	return vhost
end

function clear(srv)
	apis()
	local mc=check_mc()
	
	mc:flush_all()
--	ngx.shared.cache:flush_all()

	apie()
end


function del(srv,id)
	apis()
	local mc=check_mc()

	local k=getvhost(srv).."&"..id
	
	mc:delete(k)
--	ngx.shared.cache:delete(k)

	apie()
end

function put(srv,id,v,ttl,opts)
--	log("cache.put:",id)
	apis()
	local mc=check_mc()
	
	local k=getvhost(srv).."&"..id
	
	ttl=ttl or 60*5
	
--[[
	if type(v)=="number" then
		--as is
	else 
		v=wstr.serialize(v)
	end
]]

	if opts=="ADD_ONLY_IF_NOT_PRESENT" then
	
		mc:add(k,v,ttl)
--		ngx.shared.cache:add(k,v,ttl)
		
	else -- normal set
	
		mc:set(k,v,ttl)
--		ngx.shared.cache:set(k,v,ttl)
		
	end

	
	apie()
end

function get(srv,id)
	apis()
	local mc=check_mc()
	count=count+1

	local k = getvhost(srv).."&"..id
	local r = mc:get(k)
--	local r = ngx.shared.cache:get(k)
	
	if r then count_got=count_got+1 end
	
--[[
	if type(r)=="string" then -- need to unserialise the data
		r=wbox.lson(r)
	end
]]
	apie()
	return r
end

function inc(srv,id,num,start)
	apis()
	local mc=check_mc()

	local k=getvhost(srv).."&"..id


	r=mc:incr(k,num)
--	r=ngx.shared.cache:incr(k,num)
	if not r then
		r=start
		mc:set(k,num)
--		ngx.shared.cache:set(k,num)
	end

	apie()
	return r
end

