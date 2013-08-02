-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log=require("wetgenes.www.any.log").log
local ngx=require("ngx")

local wstr=require("wetgenes.string")
--local socket=require("socket")
--local http=require("socket.http")
--local ltn12=require("ltn12")

module(...)
local _M=require(...)
package.loaded["wetgenes.www.any.fetch"]=_M

--[[

local function create()

--log("CREATE")
	local p={}
	local tcp=ngx.socket.tcp()

	p.accept=function(_,...)
--log("accept")
--log(...)
		return tcp:accept(...)
	end

	p.bind=function(_,...)
--log("bind")
--log(...)
		return tcp:bind(...)
	end

	p.close=function(_,...)
--log("close")
--log(...)
		return tcp:close(...)
	end
	
	p.connect=function(_,...)
--log("connect")
--log(...)
		tcp:connect(...)
		return true
	end

	p.getpeername=function(_,...)
--log("getpeername")
--log(...)
		return tcp:getpeername(...)
	end

	p.getsockname=function(_,...)
--log("getsockname")
--log(...)
		return tcp:getsockname(...)
	end

	p.getstats=function(_,...)
--log("getstats")
--log(...)
		return tcp:getstats(...)
	end

	p.listen=function(_,...)
--log("listen")
--log(...)
		return tcp:listen(...)
	end

	p.receive=function(_,...)
--log("receive")
--log(...)
		local r=tcp:receive(...)
		return r
	end

	p.send=function(_,...)
--log("send")
--log(...)
		return tcp:send(...)
	end

	p.setoption=function(_,...)
--log("setoption")
--log(...)
		return tcp:setoption(...)
	end

	p.setstats=function(_,...)
--log("setstats")
--log(...)
		return tcp:setstats(...)
	end

	p.settimeout=function(_,n)
--log("settimeout")
--log(n)
		tcp:settimeout(n*1000)
		return true
	end

	p.shutdown=function(_,...)
--log("shutdown")
--log(...)
		return tcp:shutdown(...)
	end

--log(wstr.dump(p))

	return p
end

]]

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

--[[
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
]]

--[[

function post(url,headers,body)
--	log("fetch.post:"..url)
	apis()
	count=count+1

	local res_body={}


	local suc, code , rheaders = assert(socket.http.request{
		create=create,
		url=url,
		method="POST",
		headers=headers,
		source = ltn12.source.string(body),
		sink = ltn12.sink.table(res_body),
	})
	
--	log("Received "..suc.." "..code.."\n") -- wstr.serialize(headers)
--	table.foreach(res_body,print)
	local ret={}
	ret.body=table.concat(res_body)
	ret.code=code
	ret.headers=rheaders

	apie()
	return ret
end



function get(url,headers,body)
	log("fetch.get:"..url)
	log(wstr.dump(headers))
	log(wstr.dump(body))
	apis()
	count=count+1

	local res_body={}

	body=body or ""
	headers=headers or {
		["User-Agent"]="Mozilla/5.0 (Windows NT 5.1; rv:8.0) Gecko/20100101 Firefox/8.0",
		["Content-Length"] = #body,
		["Referer"]=url,
	}

	local suc, code , rheaders = assert(socket.http.request{
		create=create,
		url=url,
		method="GET",
		headers=headers,
--		source = ltn12.source.string(body),
		sink = ltn12.sink.table(res_body),
	})
	
--	log("Received "..tostring(suc).." "..tostring(code).."\n") -- wstr.serialize(headers)
--	table.foreach(res_body,print)
	local ret={}
	ret.body=table.concat(res_body)
	ret.code=code
	ret.headers=rheaders
	log("Received "..tostring(ret.body).."\n")

	apie()
	return ret
end
]]


function get(url,headers,body)
--	log("NEWfetch.get:"..url)

local ret

	for i=1,10 do -- limit redirects

--	log("NEWfetch.get:"..url)

		ret=ngx.location.capture("/_proxy",{
			method=ngx.HTTP_GET,
			body=body,
			ctx={ headers = headers },
			vars={ _url = url },
		})
		ret.code=ret.status ret.status=nil -- rename status to code
		ret.mimetype=ret.header["Content-Type"]
		
		if ret.code>=300 and ret.code<400 then -- follow redirects
		
			url=ret.header.Location
			
		else
			break			
		end
		
	end

--	log("NEWReceived "..tostring(ret.body).."\n")

	return ret
end


function post(url,headers,body)
--	log("NEWfetch.post:"..url)


	local ret=ngx.location.capture("/_proxy",{
		method=ngx.HTTP_POST,
		body=body,
		ctx={ headers = headers },
		vars={ _url = url },
	})
	ret.code=ret.status ret.status=nil -- rename status to code
	ret.mimetype=ret.header["Content-Type"]

--	log("NEWReceived "..tostring(ret.body).."\n")

	return ret
end

