--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

--[[#lua.wiretasks

	local wiretasks=require("wiretasks")

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local wiretasks=M

--[[#lua.wire.http_code

	wire.execute("http",4,"require('wire').http_code()")
	result = wire.memo({...}):resolve()
	wire.execute("http",0)

The code we run in http_tasks to handle http request via memos.

Note that this function requires external libraries that must be 
available in order to work.

Every memo is a http request, the keys set in memo control the type of 
request.

	memo.url = "http://google.com/"

The url we will be talking to.

	memo.get = { a=1 , b=2 }

If set then send a GET request with this table in the query string, eg 
append "?a=1&b=2" to the url.

	memo.json = { a=1 , b=2 }

If set then encode as json and send as body of a POST also setting 
"Content-Type" to "application/json" in memo.headers

	memo.post = { a=1 , b=2 }

If set then encode as a query string and send as body of a POST also 
setting "Content-Type" to "application/x-www-form-urlencoded" in 
memo.headers

	memo.body = "a=1&b=2"

Force a request body string.

	memo.method = "GET"

Force a request method.

	memo.headers = { ["Content-Type"]="application/json" , ... }

Force extra headers.

	result.body
	result.code
	result.headers
	result.status

The result of the request, may also raise an error for catastrophic 
failures, eg host not found. Otherwise check the result.code is 200 and 
then read the body returned if you are expecting data.

]]
wire.http_code=function()

	local js_http -- function call into javascript if we are an emcc build
	pcall( function() js_http = require("wetgenes.win.core").js_http end )

	local http = require("socket.http")
	local ltn12 = require("ltn12")
	local djon = require("djon") -- read write json

-- we need a special case for emcc as we can only use websockets or javascript requests
	local request_js
	if js_http then
		request_js=function(memo)

			local opts={}
			opts.method=memo.method
			opts.url=memo.url
			opts.headers=memo.headers or {}
			opts.body=memo.body
			
			local rets=js_http( djon.save(opts) )
			local ret=djon.load( rets or "{}" ) or {}
			if not ret.body then ret.fail=ret.code or true end

			return ret
		end
	end

	local function consume(memo)
	
		memo.headers=memo.headers or {}

		local urlencode=function(s)
			return tostring(s):gsub("([^%w_%%%-%.~])",
				function(c) return string.format("%%%02X", string.byte(c)) end )
		end

		-- check for values passed by table that we shouold encode
		
		if memo.get then -- add a ? and these values to the url
		
			local t={}
			for n,v in pairs(memo.post) do
				t[#t+1]=urlencode(n) .. "=" .. urlencode(v)
			end
			if string.find( memo.url, "?" , 1 , true ) then -- already a query
				local c=string.sub(memo.url,-1,1) -- last char
				if c~="?" and c~="&" then -- must be one a seperator
					memo.url=memo.url.."&"
				end
			else
				memo.url=memo.url.."?"
			end
			memo.url=memo.url..table.concat(t,"&")
		
		end
		
		if memo.json then -- we want to send all these values in a POST json body

			memo.body=djon.save(memo.json)
			memo.method=memo.method or "POST"
			memo.headers["Content-Type"]="application/json"
			
		end

		if memo.post then -- we want to send all these values in a POST body

			local t={}
			for n,v in pairs(memo.post) do
				t[#t+1]=urlencode(n) .. "=" .. urlencode(v)
			end
			memo.body=table.concat(t,"&")
			memo.method=memo.method or "POST"
			memo.headers["Content-Type"]="application/x-www-form-urlencoded"
			
		end
		
		memo.method=memo.method or "GET"

		if request_js then
			return request_js(memo)
		end

		local out = {}
		local req = {}

		req.sink = ltn12.sink.table(out)
		if memo.body then
			req.source = ltn12.source.string(memo.body)
			memo.headers["Content-Length"]=#memo.body
		end

		req.url=memo.url
		req.method=memo.method
		req.headers=memo.headers
		req.proxy=memo.proxy
		req.redirect=memo.redirect

		local body , code, headers, status = http.request(req)
		local result={}

		if not body then -- error message is in 2nd return ( code )
			result.fail=code or true
		else
			result.body=table.concat(out)
			result.code=code
			result.headers=headers
			result.status=status
		end
		
		return result
	end


	 -- this named fifo will have been created before this thread
	local fifo = wire.manifest("http")

	-- loop until our thread is asked to halt
	while wire.active( wire.thread_handle ) do
		local memo = fifo:pull()
		
		if memo then

			-- this will print a TRACEBACK on error but keep going
			local ok,result = xpcall( function() return consume( memo.data ) end , TRACEBACK )

			if ok then
				memo.result=result -- consume gave us a result to reply with
			else
				memo.result={ fail=(result or "fail") } -- reply with fail reason
			end
			memo:reply()
			memo:remove()

		else -- no memo, sleep a bit
		
			fifo:wait(1/16)

		end

	end
	-- we have gracefully halted so cleanup and return

end


--[[#lua.wetgenes.wire.sqlite_code

As we are opening an sqlite database here it wont help much to have 
more than one thread per database as they will just fight over file 
access.

]]
wire.sqlite_code=function(linda,task_id,task_idx)

	local lanes = require("lanes")
	local sqlite3 = lanes.require("lsqlite3")

	local db

	if sqlite_filename then	db = assert(sqlite3.open(sqlite_filename)) end -- auto open
	if sqlite_pragmas and db then db:exec(sqlite_pragmas) end -- auto configure

	local function request(memo)
	
		local ret={}
	
		if memo.cmd then -- this is a special cmd eg to close or open the database
		
			if memo.cmd=="close" then -- probably good to "try" and do this before exiting
				db:close()
				db=nil
				ret.rows={}
			end

		elseif memo.sql then -- execute some sql
		
			if not db then
				ret.error="no database"
				return ret
			end

			local rows={}
			
			
			local err
			
			if memo.binds or memo.blobs then -- use prepared statement
			
				local stmt = db:prepare(memo.sql)
				if not stmt then
					ret.error=db:errmsg()
					return ret
				end

				local bmax=stmt:bind_parameter_count()
				local bs={}
				for i=1,bmax do
					local n=stmt:bind_parameter_name(i)
					if n then
						bs[n]=i
						bs[n:sub(2)]=i
					end
				end

				
				local blobs=memo.blobs or {}
				for n,v in pairs( memo.binds or {} ) do
					if bs[n] and not blobs[n] then -- a blob might be in both places
						stmt:bind( bs[n] , v )
					end
				end
				for n,v in pairs( memo.blobs or {} ) do -- these binds should be treated as blobs
					if bs[n] then
						stmt:bind_blob( bs[n] , v )
					end
				end
				
				if memo.compact then
					rows.names=stmt:get_names()
					for it in stmt:rows() do
						rows[#rows+1]=it
					end
				else
					for it in stmt:nrows() do
						rows[#rows+1]=it
					end
				end

				err=stmt:finalize()
			
			else
			
				if memo.compact then -- return data in a slightly more compact format

					err=db:exec(memo.sql,function(udata,cols,values,names)
						rows.names=names
						rows[#rows+1]=values
						return 0
					end,"udata")
				
				else

					err=db:exec(memo.sql,function(udata,cols,values,names)
						local it={}
						for i=1,cols do it[ names[i] ] = values[i] end
						rows[#rows+1]=it
						return 0
					end,"udata")

				end

			end

			if err~=sqlite3.OK then
				ret.error=db:errmsg()
			else
				ret.rows=rows
			end

		end
		
		return ret
	end

	while true do

		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end

--[[#lua.wetgenes.wire.client_code

A basic function to handle (web)socket client connection.

]]
wire.sock_code=function(linda,task_id,task_idx)

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local wjson = lanes.require("wetgenes.json")
	local js_eval -- function call into javascript if we are an emcc build
	do
		local ok,lib=pcall(function() return lanes.require("wetgenes.win.core") end )
		if ok and lib then js_eval=lib.js_eval end
	end
	local js_call=function(script,opts)
		local js=[[
(function(opts){
	var ret={};
]]..script..[[
	return JSON.stringify(ret);
})(]]..wjson.encode(opts or {})..[[);
]]
		local rets=js_eval(js)
		return wjson.decode( rets or "{}" ) or {}
	end
	
	local socket = lanes.require("socket")
	local err
	local client
	if js_eval then -- js mode

		js_call([[

globalThis.wetgenes_tasks=globalThis.wetgenes_tasks || {};
globalThis.wetgenes_tasks[opts.task_id]=globalThis.wetgenes_tasks[opts.task_id] || {};

var data=globalThis.wetgenes_tasks[opts.task_id];
data.send=[];
data.recv=[];

data.onmessage=function(e){
	console.log("onmessage OK");
	data.recv.push(e.data);
}
data.onopen=function(e){
	console.log("onopen OK");
	console.log(e);
}
data.onclose=function(e){
	console.log("onclose OK");
	console.log(e);
}
data.onerror=function(e){
	console.log("onerror OK");
	console.log(e);
}

if(opts.url)
{
	data.sock=new WebSocket(opts.url);
	data.sock.onmessage=data.onmessage;
	data.sock.onopen=data.onopen;
	data.sock.onclose=data.onclose;
	data.sock.onerror=data.onerror;
console.log(data.sock);
}

]],{task_id=task_id,url=client_url})
	else
		if client_host and client_port then -- auto open a client connection
			client , err = socket.connect(client_host,client_port)
			if client then client:settimeout(0.00001) end
		end
	end

	local request=function(memo)
	
		if js_eval then -- need js mode
			local ret=js_call([[

var data=globalThis.wetgenes_tasks[opts.task_id];

if(opts.data)
{
console.log("QUEUE:"+opts.data);
	data.send.push(opts.data);
}

if(data.sock)
{
	if(data.sock.readyState==1)
	{
		while(data.send.length>0)
		{
console.log("SEND:"+send[0]);
			data.sock.send(data.send.shift());
		}
	}
	else
	{
//console.log("SOCK:"+data.sock.readyState);
	}
}
while(data.recv.length>0)
{
console.log("RECV:"+recv[0]);
	ret.data=(ret.data || "")+data.recv.shift();
}

]],{task_id=task_id,data=memo.data})

			return ret
		end -- end js mode
		
		local ret={}
	
		if memo.cmd then -- this is a special cmd eg to close or open a socket
			if     memo.cmd=="connect" and not client then
				client , err = socket.connect(memo.host,memo.port)
				if client then client:settimeout(0.00001) end
				if err then		ret.error=err
				else			ret.data=true
				end
			elseif memo.cmd=="close" and client then
				client:close()
				client=nil
				ret.data=true
			end
		end

		if memo.data then -- something to send
			if not client then return {error=err or true} end
			client:send(memo.data)
		end
		
		if client then -- try and read some data from server
			local part,e,part2=client:receive("*a")
			if e=="timeout" then ret.warning=e e=nil err=nil part=part or part2 end -- ignore timeouts, they are not errors just partial data
			if part~="" then ret.data=part end
			if e then ret.error=e end
		end
		
		return ret
	end
	
	while true do

		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end
	
end

