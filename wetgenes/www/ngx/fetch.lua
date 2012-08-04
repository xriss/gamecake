-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log
local ngx=require("ngx")

local wstr=require("wetgenes.string")
local socket=require("socket")
local http=require("socket.http")

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.fetch"]=_M

function countzero()
	count=0
	api_time=0
end
countzero()

local kind_props={}	-- default global props mapped to kinds

local start_time -- handle simple api benchmarking of some calls
local function apis()
	start_time=os.time()
end
local function apie(...)
	api_time=api_time+os.time()-start_time
	return ...
end




function get(url)
	log("fetch.get:")
	apis()
	count=count+1
	
	local res = ngx.location.capture("/@fetch/"..url)

	ret={}
	ret.code=res.status
	ret.headers=res.header
	ret.body=res.body

log(wstr.serialize(ret))
	
	apie()
	return ret
end



function post(url,headers,body)
	log("fetch.post:"..url)
	apis()
	count=count+1

	local res_body={}


	local suc, code , headers = assert(socket.http.request{
		url=url,
		method="POST",
		headers=headers,
		source = ltn12.source.string(body),
		sink = ltn12.sink.table(res_body),
	})
	
	log("Received "..suc.." "..code.."\n") -- wstr.serialize(headers)
--	table.foreach(res_body,print)
	local ret=table.concat(res_body)

	apie()
	return ret
end
