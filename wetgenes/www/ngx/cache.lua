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
	
	ngx.shared.cache:flush_all()

	apie()
end


function del(srv,id)
	apis()

	local k=getvhost(srv).."&"..id
	
	ngx.shared.cache:delete(k)

	apie()
end

function put(srv,id,v,ttl,opts)
--	log("cache.put:",id)
	apis()
	
	local k=getvhost(srv).."&"..id
	
	if type(v)=="number" then
		--as is
	else 
		v=wstr.serialize(v)
	end

	if opts=="ADD_ONLY_IF_NOT_PRESENT" then
	
		ngx.shared.cache:add(k,v,ttl)
		
	else -- normal set
	
		ngx.shared.cache:set(k,v,ttl)
		
	end

	
	apie()
end

function get(srv,id)
	apis()
	count=count+1

	local k = getvhost(srv).."&"..id
	local r = ngx.shared.cache:get(k)
	
	if r then count_got=count_got+1 end
	
	if type(r)=="string" then -- need to unserialise the data
		r=wbox.lson(r)
	end

	apie()
	return r and r.tab
end

function inc(srv,id,num,start)
	apis()

	local k=getvhost(srv).."&"..id


	r=ngx.shared.cache:incr(k,num)
	if not r then
		r=start
		ngx.shared.cache:set(k,num)
	end

	apie()
	return r
end

