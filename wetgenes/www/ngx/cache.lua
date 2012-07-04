-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")

local ngx=ngx

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.cache"]=_M

hax={}


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
local function gethax(srv)
	local vhost=getvhost(srv)
	if not hax[vhost] then hax[vhost]={} end
	return hax[vhost]
end

function clear(srv)
--	log("cache.clear:")
	apis()

	hax[getvhost(srv)]={}

	apie()
end


function del(srv,id)
--	log("cache.del:")
	apis()

	local hax=gethax(srv)
	
	hax[id]=nil

	apie()
end

function put(srv,id,tab,ttl,opts)
--	log("cache.put:",id)
	apis()
	
	local hax=gethax(srv)

	local t={tab=tab,ttl=os.time()+ttl}

	if opts=="ADD_ONLY_IF_NOT_PRESENT" then
		local r=hax[id]
		if r and r.ttl<os.time() then -- stale data must die
			hax[id]=nil
			r=nil
		end
		if not r then hax[id]=t end
	else
		hax[id]=t
	end
	
	apie()
end

function get(srv,id)
--	log("cache.get:",id)	
	apis()
	count=count+1

	local hax=gethax(srv)
	
	r=hax[id]
	
	if r and r.ttl<os.time() then -- stale data must die
		hax[id]=nil
		r=nil
	end
	
	if r then count_got=count_got+1 end
	
--log(wstr.serialize(r))
	apie()
	return r and r.tab
end

function inc(srv,id,num,start)
--	log("cache.inc:",id)
	apis()

	local hax=gethax(srv)
	
	local r=(hax[id] or start)+num
	hax[id]=r
	
--log(r)
	apie()
	return r
end

