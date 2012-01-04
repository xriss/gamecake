-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log

local wstr=require("wetgenes.string")

module(...)

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


function clear(srv)
--	log("cache.clear:")
	apis()

	hax={}

	apie()
end


function del(srv,id)
--	log("cache.del:")
	apis()

	hax[id]=nil

	apie()
end

function put(srv,id,tab,ttl)
--	log("cache.put:",id)
	apis()
	
	hax[id]=tab
	
	apie()
end

function get(srv,id)
--	log("cache.get:",id)	
	apis()
	count=count+1

	r=hax[id]
--log(wstr.serialize(r))
	apie()
	return r
end

function inc(srv,id,num,start)
--	log("cache.inc:",id)
	apis()

	local r=(hax[id] or start)+num
	hax[id]=r
	
--log(r)
	apie()
	return r
end

