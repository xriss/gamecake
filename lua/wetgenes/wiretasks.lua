--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

--[[#lua.wetgenes.wiretasks

	local wiretasks=require("wetgenes.wiretasks")

This is a small collection of useful tasks for running via wire. Each 
task has its own specific module dependencies, eg lua socket is needed 
for http fetching. This module is usually used in a task start code 
string passed to wire.tasks. So 
"require('wetgenes.wiretasks').http_code()" would be used to start the 
http task.

We also include some wrapper functions that make use of memos and 
require certain tasks to already be running.

]]

local wire=require("wire")
local djon=require("djon")

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local wiretasks=M

--[[#lua.wetgenes.wiretasks.http_code

	wire.tasks("http",4,"require('wetgenes.wiretasks').http_code()")

	result = wire.memo({
		fifo=wire.fifo("http"),
		data={
			url="http://google.com/",
		},
		on_result=function() end,
	}):resolve()

The code we run in "http" tasks to handle http requests via memos.

This function requires the following libraries to be available.

	require("socket.http")
	require("ltn12")

Every memo sent is a http fetch request, the keys set in memo.data 
control the type of request.

	data.url = "http://google.com/"

The url we will be talking to. This is all you need for a basic GET.

	data.get = { a=1 , b=2 }

If this exists then we send a GET request with this table in the query 
string, eg append "?a=1&b=2" to the url.

	data.json = { a=1 , b=2 }

If this exists then we encode as json and send as body of a POST also 
setting "Content-Type" to "application/json" in data.headers

	data.post = { a=1 , b=2 }

If this exists then we encode as a query string and send as body of a 
POST also setting "Content-Type" to "application/x-www-form-urlencoded" 
in data.headers

	data.body = "a=1&b=2"

Force a request body string.

	data.method = "GET"

Force a request method.

	data.headers = { ["Content-Type"]="application/json" , ... }

Force extra headers.

	result.code
	result.status
	result.headers
	result.body

The result of the http.request, may also raise an error for 
catastrophic failures, eg host not found.

Many things can go wrong so always check that result.fail is not set 
and the result.code is as expected ( eg 200 ) and only then is it safe 
to read the result.body

]]
wiretasks.http_code=function()

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
	local fifo = wire.fifo(wire_tasks_name)

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


--[[#lua.wetgenes.wiretasks.sqlite_code

	wire.tasks("sqlite",1,"require('wetgenes.wiretasks').sqlite_code()",{
			sqlite_filename = "./test.sql" ,
			sqlite_pragmas = "" ,
		})

	result = wire.memo({
		fifo=wire.fifo("sqlite"),
		data={
			sql="SELECT name FROM sqlite_master WHERE type='table';",
		},
		on_result=function() end,
	}):resolve()

The code we run in "sqlite" tasks to handle sqlite requests via memos.

This function requires the following libraries to be available.

	require("lsqlite3")

As we are opening an sqlite database here it probably wont help you 
much to have more than one thread per database as they will just fight 
over file access. To open multiple sqlite databases create multiple 
tasks with different names.

Globals passed into the thread control the startup actions, primarily 
you should provide an sqlite_filename to open.

	sqlite_filename="./test.sql"

The sqlite database to open when this thread starts.

Most memos are simple SQL queries, but some extra commands exist via 
data.action

	data.action="close"

Shut down the sqlite database, this combined with waiting for the 
thread to halt allows sqlite to clean up before exiting the process.

When data.action is not set we are performing an SQL query.

	data.sql="SELECT name FROM sqlite_master WHERE type='table';",

Will return result.rows filled with data.

	data.compact=true

If this boolean is set then we will return compact rows, data.rows will 
contain rows that are arrays and data.names will be an array of column 
names. If not set then we return rows as [name]=value objects.

	data.binds={ name=value , ... }

Bind these values to an sqlite statement.
	
	data.blobs={ name=value , ... }

Bind these values to an sqlite statement as blobs. This must be used 
for sending binary data.



]]
wiretasks.sqlite_code=function()

	local sqlite3 = require("lsqlite3")

	-- only if available , will error if you try and use when nil
	local wgetsql ; pcall( function() wgetsql=require("wetgenes.getsql") end )

	local db

--	if sqlite_filename then	db = assert(sqlite3.open(sqlite_filename)) end -- auto open
--	if sqlite_pragmas and db then db:exec(sqlite_pragmas) end -- auto configure

	local opendb=function(filename,pragmas,tables,autoset)

		if not filename then -- for wasm problems
			db = assert(sqlite3.open_memory())
		else
			db = assert(sqlite3.open(filename))
		end
		
		if pragmas then
			db:exec(pragmas)
		end
		
		if tables then
			if not wgetsql then error("wetgenes.getsql module not available") end
			for tabname,tab in pairs(tables) do
				local sql=wgetsql.sqlite.create_table(tab.name,tab)
				db:exec(sql)
				for _,sql in ipairs( wgetsql.sqlite.alter_table(tab.name,tab) ) do
					db:exec(sql)
				end
				for _,sql in ipairs( wgetsql.sqlite.create_table_indexs(tab.name,tab) ) do
					db:exec(sql)
				end
			end
		end
		
		if autoset then -- auto set data if not exists
		
			for tabname,tab in pairs( autoset ) do
				local keys={}
				for key in db:urows([[
					SELECT ]]..tab.keyname..[[ FROM ]]..tabname..[[ ;
				]]) do
					keys[key]=true
				end
				for _,row in pairs(tab.rows) do
					local key=row[ tab.keyname ]
					if not keys[key] then
						local names={}
						for n,v in pairs(row) do names[#names+1]=":"..n end
						names=table.concat(names," , ")
						local stmt = db:prepare[[ INSERT INTO ]]..tabname..[[ VALUES (]]..names..[[) ]]
						stmt:bind_names(row)
						stmt:step()
						stmt:finalize()
					end
				end
--[=[
				local keys={}
				for key in db:urows([[
					SELECT key FROM config ;
				]]) do
					keys[key]=true
				end
			
				for key,value in pairs(tab.values) do
					if not keys[key] then
						local stmt = db:prepare[[ INSERT INTO config VALUES (:key, :value) ]]
						stmt:bind_names{  key = key,  value = value    }
						stmt:step()
						stmt:finalize()
					end
				end
]=]
			end

		end
		
		return db
	end
	opendb( sqlite_filename , sqlite_pragmas , sqlite_tables , sqlite_autoset ) -- auto open and pragma and create tables


	local consume=function(data)
	
		local result={rows={}}
	
		if data.action then -- this is a special action eg to close or open the database

			if data.action=="open" then -- gonna need to do this first or set sqlite_filename to auto open

				if db then -- auto close
					db:close()
					db=nil
				end
				opendb( memo.filename , memo.pragmas , memo.tables , memo.autoset )

			elseif data.action=="close" then -- probably good to "try" and do this before exiting

				if db then db:close() end
				db=nil

			else
				result={fail="unknown action"}
			end
			
			return result

		elseif data.sql then -- execute some sql
		
			if not db then
				result.fail="no database"
				return result
			end

			local rows={}
			
			
			local err
			
			if data.binds or data.blobs then -- use prepared statement
			
				local stmt = db:prepare(data.sql)
				if not stmt then
					result.fail=db:errmsg()
					return result
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

				
				local blobs=data.blobs or {}
				for n,v in pairs( data.binds or {} ) do
					if bs[n] and not blobs[n] then -- a blob might be in both places
						stmt:bind( bs[n] , v )
					end
				end
				for n,v in pairs( data.blobs or {} ) do -- these binds should be treated as blobs
					if bs[n] then
						stmt:bind_blob( bs[n] , v )
					end
				end
				
				if data.compact then
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
			
				if data.compact then -- return data in a slightly more compact format

					err=db:exec(data.sql,function(udata,cols,values,names)
						rows.names=names
						rows[#rows+1]=values
						return 0
					end,"udata")
				
				else

					err=db:exec(data.sql,function(udata,cols,values,names)
						local it={}
						for i=1,cols do it[ names[i] ] = values[i] end
						rows[#rows+1]=it
						return 0
					end,"udata")

				end

			end

			if err~=sqlite3.OK then
				result.fail=db:errmsg()
			else
				result.rows=rows
			end

		end
		
		return result
	end

	 -- this named fifo will have been created before this thread
	local fifo = wire.fifo(wire_tasks_name)

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
