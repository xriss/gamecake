-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local core=require("wetgenes.www.gae.cache.core")
local log=require("wetgenes.www.any.log").log

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.cache"]=_M

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
	apis()
	return apie(core.clear())

end


function del(srv,id)
	if srv then srv.cache[id]=nil end
	apis()
	return apie(core.del(id))

end

function put(srv,id,tab,ttl)
	if srv then srv.cache[id]=tab end -- this local cache only lasts as long as a request
	apis()
--if( type(tab)~="string" ) then log(type(tab).." "..tostring(tab)) end -- it seems to be only safe to cache strings
	return apie(core.put(id,tab,ttl))

end

function get(srv,id)
	if srv and type(srv.cache[id])~="nil" then return srv.cache[id] end -- very fast retry for multiple gets
	
	apis()
	count=count+1

	local r=apie(core.get(id))

	if type(r)~="nil" then count_got=count_got+1 end -- a false is still a good result

	return r
end

function inc(srv,id,num,start)
	apis()
	local r=apie(core.inc(id,num,start))
	
	if srv then srv.cache[id]=r end -- so we can fast get it later in this request

	return r
end

