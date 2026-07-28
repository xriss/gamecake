--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

--[[#lua.wire

	local wire=require("wire")

We use wire as the local name of this library. So we can uhh, wire 
things together?

Wire provides multiple long lived lua states managed via standard c11 
threads and mutexs with simple fifo message handling between threads.

A small compatibility library is needed for building for windows using 
mingw. Native windows builds might work if your compiler supports the 
C11 standard.

We are not doing anything clever, so beware of the following:
 
Only data is passed and that data must fit in messagepack, eg json like 
but binary strings are allowed.

Do not use lua tables that are both arrays and objects. Pick one or the 
other.

No userdata, no functions.

When in doubt, send a string, then unpack it in the thread. Binary 
strings will be escaped into lua code, eg \0 for null etc.

Any lua libs required in a thread *must* be thread safe or they will 
break in strange and uncomfortable ways. So check that luaopen_ 
functions do not try and initialise anything twice if opened in another 
thread.

This library does not reuse handles ( as this is intended for long 
lived threads ) and the number of handles is a hard limit set in the C 
code. Defaults to 4096 threads and 4096 fifos. Bump it at compile time 
if you need more.

You can reuse handles, but it must be self managed and must be done 
explicitly.

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local wire=M

-- auto preloadlibs on new threads if gamecake is available
pcall( function() wire.preloadlibs=require("wetgenes.gamecake.core").preloadlibs end )

local core=require("wire.core")

-- meta methods bound to the various objects

wire.thread_functions={is="thread"}
wire.thread_metatable={__index=wire.thread_functions}

wire.fifo_functions={is="fifo"}
wire.fifo_metatable={__index=wire.fifo_functions}

wire.memo_functions={is="memo"}
wire.memo_metatable={__index=wire.memo_functions}

-- keep cache of threads and fifos by handle
wire.cache_threads={}
wire.cache_fifos={}

-- cache names to handles. The house thread syncs this data
-- and we wrap the handles locally. threads and fifos exist
-- in the same name space as every thread is also a fifo.
wire.cache_names={} 

-- cache names to thread for easy access of named resources.
wire.threads={}

-- cache names to fifo for easy access of named resources.
wire.fifos={}

--[[#lua.wire.table_to_data

	data = wire.table_to_data( table )

Convert a lua table into a data string.

]]
wire.table_to_data = require("cmsgpack").pack


--[[#lua.wire.data_to_table

	table = wire.data_to_table( data )

Convert a data string into a lua table.

]]
wire.data_to_table = require("cmsgpack").unpack


--[[#lua.wire.sleep

	wire.sleep(secs)

Take a nap for at least the given amount of seconds, probably a little 
bit longer.

]]
wire.sleep=core.sleep -- copy the core cfunction

--[[#lua.wire.wait

	wire.wait(secs)

Take a nap for upto the given amount of seconds. We will wakeup 
whenever a new msg is sent to any fifo or when this time has passed.

]]
wire.wait=core.wait -- copy the core cfunction

-- active memos for this thread grouped by status
wire.memos={
	all={},
	setup={},
	sent={},
	reply={},
	result={},
	got={},
}


--[[#lua.wire.manifest

	fifo = wire.manifest(name)

Get a named fifo/thread from the house thread.

	fifo = wire.manifest(handle)

Manifest a fifo/thread with this handle

Note that the returned value may be a fifo or a thread.

]]
wire.manifest=function(name)

	-- try local cache
	if type(name)=="number" then
		if name<0 then
			if wire.cache_threads[name] then
				return wire.cache_threads[name]
			end
		else
			if wire.cache_fifos[name] then
				return wire.cache_fifos[name]
			end
		end
	else -- turn name into handle
		local memo=wire.memo({
			fifo=wire.fifos.house,
			data={
				cmd="handle",
				name=name,
			},
		})
		memo:resolve()
	end

end

--[[#lua.wire.thread

	thread = wire.thread(handle)
	thread = wire.thread(name)
	thread = wire.thread(opts)

Create or get a thread table. Threads are not GC'd they must be created 
and destroyed explicitly.

When called with a handle (number) we return a previously created 
thread with that handle.

When called with a name (string) we return a previously created 
thread with that name.

We will assert if no thread is found with that handle or name, so no 
need to check returns.

When called with opts (table) we do the following based on the 
contents.

	opts.handle=nil

The handle to wrap or use in creation. If a valid handle we will simply 
wrap it with a thread table and return that new table.

	opts.start=nil

Run this string as lua code in a new thread. Usually just a require and 
a call to a function in that module. eg "require('modname').funcname()"

	opts.globals={}

Optional globals for the new thread. Will be serialized and become part 
of the start string.

	opts.preloadlibs=wire.preloadlibs
	
Must be a cfunction to be run in new thread. Use false to not preload 
any custom libs.

	opts.header=...
	opts.footer=...

internal setup so best not to change this. It uses wire.do_start to 
wrap your code with error handlers etc.

]]
wire.thread=function(opts)
	do
		local t=type(opts)
		if t=="number" then
			return assert(wire.cache_threads[opts])
		elseif t=="string" then
			return assert(wire.threads[opts])
		end
	end
	
	local name=opts.name -- may be nil
	local preloadlibs=opts.preloadlibs -- set to false for no preload
	if type(preloadlibs)=="nil" then preloadlibs=wire.preloadlibs end
	
	local thread = {}
	setmetatable(thread,wire.thread_metatable)
	
	thread.handle=opts.handle

	if not core.handle( thread.handle ) then -- we must create a new thread
	
		assert(opts.start) -- must have start code to start

		local start=wire.prepare_start(opts)
		assert( loadstring(start) , "wire thread start string" ) -- make sure the code is valid

		thread.handle=core.thread_create( thread.handle or 0 , start , preloadlibs or nil )
		
		 -- auto generate name , might get changed later by house thread
		if not name then name="thread"..thread.handle end
	end
	
	if not core.handle( thread.handle ) then
		error("wire thread creation failed")
	end
	
	wire.cache_threads[thread.handle]=thread
	if name then
		thread.name=name
		wire.cache_names[name]=thread.handle
		wire.threads[name]=thread
	end
	
	return thread
end

--[[#lua.wire.fifo

	fifo = wire.fifo(handle)
	fifo = wire.fifo(name)
	fifo = wire.fifo(opts)

Create or get a fifo. Fifos are not GC'd they must be created 
and destroyed explicitly.

When called with a handle (number) we return a previously created 
fifo with that handle.

When called with a name (string) we return a previously created 
fifo with that name.

We will assert if no fifo is found with that handle or name, so no 
need to check returns.

When called with opts (table) we do the following based on the 
contents.

	opts.handle=nil

The handle to wrap or use in creation. If a valid handle we will simply 
wrap it with a fifo.

]]
wire.fifo=function(opts)
	do
		local t=type(opts)
		if t=="number" then
			return assert( wire.cache_fifos[opts] )
		elseif t=="string" then
			return assert( wire.fifos[opts] )
		end
	end

	local fifo={}
	setmetatable(fifo,wire.fifo_metatable)

	local name=opts.name -- may be nil
	fifo.handle=opts.handle -- may be nil or 0

	if not core.handle( fifo.handle ) then -- we must create a new fifo

		fifo.handle=core.fifo_create( fifo.handle or 0 )

		 -- auto generate name , might get changed later by house thread
		if not name then name="fifo+"..fifo.handle end
	end
	
	if not core.handle( fifo.handle ) then
		error("wire fifo creation failed")
	end

	wire.cache_fifos[fifo.handle]=fifo
	if name then
		wire.cache_names[name]=fifo.handle
		wire.fifos[name]=fifo
	end

	return fifo
end

--[[#lua.wire.memo

	memo = wire.memo( { sender=handle , id=id } )

Create a memo and return it.

memo.sender can be provided or it will be set to our handle ( which is 
wire.thread.us.handle )

memo.id can be provided or it will be generated.

The memo will also be placed in wire.memos for later processing.

]]
wire.memo=function(memo)

	if not memo then memo={} end
	setmetatable(memo,wire.memo_metatable)
	
	-- auto fill if missing
	if not memo.id then
		memo.id=core.pointer(memo) -- address of table
	end
	if not memo.sender then
		memo.sender = wire.threads.us.handle -- we is who we is
	end

	wire.memos.all[ memo.id ]=memo
	memo:status("setup")

	return memo
end

--[[#lua.wire.memo.status

	memo:status(state)

Change status of memo, moving it between caches.

This is an internal function.

]]
wire.memo_functions.status=function( memo , state )

	if memo.state then
		wire.memos[ memo.state ][ memo.id ]=nil
	end
	memo.state=state
	wire.memos[ memo.state ][ memo.id ]=memo

end

--[[#lua.wire.memo.send

	memo:send()

Send a memo

]]
wire.memo_functions.send=function( memo )

	if memo.result then -- reply

		local data=wire.table_to_data(memo.result)
		core.fifo_push( memo.sender , data , memo.sender , memo.id )
		memo:status("reply")

	else
		assert(memo.fifo) -- memo must have destination fifo
		memo.fifo:push(memo)
	end

end


--[[#lua.wire.memo.resolve

	result = memo:resolve(fifo)

Push the memo into the fifo and then wait for a result on our thread 
fifo before returning.

result is returned and can also be found in memo.result

]]
wire.memo_functions.resolve=function( memo )

	memo:send()
	
	-- wait for result , memos we ignore here will be handled by thread later
	while true do
		local m = wire.threads.us:pull()

		if memo.result then -- got a result
			if memo.result.fail then error(memo.result.fail) end -- hard fail
			return memo.result
		end

		if not m then -- only wait if we pulled nothing
			wire.threads.us:wait(1/1024)
		end
	end

end


--[[#lua.wire.fifo.peek

	bool = fifo:peek()
	bool = thread:peek()

Peek and see if there is a memo to pull but do not pull it.

Note that a pull, even immediately after a successful peek has no 
guarantee to work.

]]
wire.fifo_functions.peek=function( fifo )

	local data,sender,id=core.fifo_peek( fifo.handle )

	return id and true or false

end
wire.thread_functions.peek=wire.fifo_functions.peek


--[[#lua.wire.fifo.pull

	memo = fifo:pull()
	memo = thread:pull()

Pull memo out of this fifo.

If this is a result we will return the sent memo filled with result data.

Returns nil if there is no memo available.

]]
wire.fifo_functions.pull=function( fifo )

	local data,sender,id=core.fifo_pull( fifo.handle )
	if not data then return end
	data=wire.data_to_table(data) -- unpack
	
	if wire.threads.us.handle == sender then -- this is a reply back to us

		local memo=assert( wire.memos.all[ id ] ) -- must match a sent memo

		memo.result=data
		memo:status("result")
		return memo

	else

		local memo=wire.memo({data=data,sender=sender,id=id}) -- new incoming memo
		memo:status("got")
		return memo

	end
end
wire.thread_functions.pull=wire.fifo_functions.pull


--[[#lua.wire.fifo.push

	fifo:push(memo)
	thread:push(memo)

Push memo into this fifo.

]]
wire.fifo_functions.push=function( fifo , memo )

	local data=wire.table_to_data(memo.data)
	core.fifo_push( fifo.handle , data , wire.threads.us.handle , memo.id )
	memo:status("sent")

end
wire.thread_functions.push=wire.fifo_functions.push

--[[#lua.wire.fifo.wait

	fifo:wait(secs)
	thread:wait(secs)

Wait upto the given number of seconds for a new memo to arrive.

]]
wire.fifo_functions.wait=function( fifo , secs )

	core.fifo_wait( fifo.handle , secs )

end
wire.thread_functions.wait=wire.fifo_functions.wait


--[[#lua.wire.serialize

	luastr = wire.serialize(tab)
	luastr = wire.serialize(tab,{ indent="\t" , newline="\n" , errors=true  })

Turn a table into a valid lua string that will recreate the table.

Optionally pass in an opts table that we will use during serialization 
so must be unique.

Possible options are.

	indent=" "
	
Indention per level. set to "" for compact output.

	newline="\n"
	
Line ending, set to "" for overly compact output.

	errors=nil
	
Set this to true and we will raise errors rather than simply adding 
comments to the output. EG unknown types or recursion.

]]
wire.serialize = function(o,opts)
	opts=opts or {}

	-- set defaults
	local opts_indent=opts.indent or " "
	local newline=opts.newline or "\n"
	local errors=opts.errors

	local dedupe={}
	local ret={}
	local fout=function(...)
		for i,v in ipairs({...}) do ret[#ret+1]=tostring(v) end
	end

	-- recursive
	local serialize ; serialize=function(o,indent)
		local t=type(o)
		if t == "number" then
		
			return fout(o)
			
		elseif t == "boolean" then
		
			if o then return fout("true") else return fout("false") end
			
		elseif t == "string" then
		
			return fout(string.format("%q", o))
			
		elseif t == "table" then
		
			local indent2=indent..opts_indent -- next indent level
			
			if dedupe[o] then
				if errors then
					error("recursive table "..tostring(o))
				else
					fout("{ --[[ ",tostring(o)," ]] }")
					return
				end
			else
				dedupe[o]=true
			
				fout("{",newline)
							
				local maxi=0
				
				for k,v in ipairs(o) do -- dump number keys in order
					fout(indent2)
					serialize(v,indent2)
					fout(",",newline)
					maxi=k -- remember top
				end
				
				for k,v in pairs(o) do
					 -- skip what we already dumped
					if (type(k)~="number") or (k<1) or (k>maxi) or (math.floor(k)~=k) then
						fout(indent2,"[")
						serialize(k,indent2)
						fout("]=")
						serialize(v,indent2)
						fout(",",newline)
					end
				end
				fout(indent,"}")
				dedupe[o]=false
				return
			end
		elseif t == "nil" then	
			return fout("nil")
		else
			if errors then
				error("cannot serialize a " .. type(o))
			else
				return fout("nil --[[",tostring(o),"]]")
			end
		end
	end

	serialize(o,"")
	fout(newline)
	return table.concat(ret)
end


--[[#lua.wire.prepare_start

	start = wire.prepare_start( { start="..." , globals={} } )

	opts.start=nil
	opts.globals={}
	opts.header="..."
	opts.footer="..."

See wire.thread for documentation on these opts, we just pass the opts 
table down into this function to prepare the start string.

]]
wire.prepare_start=function( opts )

	local ret={}
	local fout=function(...)
		for i,v in ipairs({...}) do ret[#ret+1]=v end
	end

-- build simple data header for globals
	for n,v in pairs( opts.globals or {} ) do
		fout(n," = ", wire.serialize(v) ) -- simple global data
	end

	fout(opts.header or [[

require("wire").do_start(function()

]])
	fout(opts.start) -- your actual start code
	fout(opts.footer or [[

end )

]])

	-- returns globals and a wrapped user function in a single string
	return table.concat(ret)
end

wire.do_start=function( func )

	-- optional global module so we can protect them from accidental use
	local global=_G ; pcall(function() global=require("global") end)

-- replace print with a new print that is a little bit more thready
	global.print=function(...) -- if we concat first we have less threads fighting over output
		local t={}
		for i=1,select("#", ...) do
			t[i]=tostring( select(i, ...) )
		end
		io.write(table.concat(t,"\t").."\n")
	end

-- set dumb log shortcuts as fallback when missing a logs module
	global.PRINT=print
	global.DUMP=print
	global.LOG=print
	global.TASKNAME=wire.threads.us.name
	global.TRACEBACK=function(err) LOG( TASKNAME , debug.traceback( err ) ) return err end

-- try and setup wetgenes.logs
	pcall( function()
		local logs=require("wetgenes.logs")
		if OVEN_OPTS and OVEN_OPTS.args then logs.setup(OVEN_OPTS.args) end
		global.PRINT=logs.print
		global.DUMP=logs.dump
		global.LOG=logs.log
	end )

	-- maybe we just wanted to set some globals so function is optional
	if func then xpcall( func , TRACEBACK )	end

end

wire.do_start_house=function()

	local do_memo=function(data)
		local result={}

DUMP(data)
		
		return result
	end

	-- deal with memos sent directly to us ( -2 )
	while true do
		local memo = wire.threads.us:pull()
		
		if memo then

			local ok,result = pcall( function() return do_memo( memo.data ) end , TRACEBACK )

			if ok then
				memo.result=result
			else
				memo.result={ fail=result }
			end
			memo:send()

		else

			wire.threads.us:wait(1/1024)

		end
	end

end



--[[#lua.wetgenes.tasks.http_code

A basic subtask to handle http memos.

Note that this function requires external libraries that must be 
available in order to work.

One of the main reasons to use threads is so you can do non blocking 
http requests.

]]
wire.do_start_http=function()

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
			if not ret.body then ret.error=ret.code or true end

			return ret
		end
	end

	local function request(memo)
	
		memo.headers=memo.headers or {}

		local urlencode=function(s)
			return tostring(s):gsub("([^%w_%%%-%.~])", function(c) return string.format("%%%02X", string.byte(c)) end )
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
		local ret={}

		if not body then -- error message is in code
			ret.error=code or true
		else
			ret.body=table.concat(out)
			ret.code=code
			ret.headers=headers
			ret.status=status
		end
		
		return ret
	end


	 -- this named fifo will have been created before this thread
	local fifo = wire.manifest("http")

	while true do
		
		local memo 
--[[
		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end
]]

	end

end



-- thread -1 will always exist and thread -2 will be auto created here

local our_thread_handle = assert( core.thread_handle() )-- get our thread handle 

wire.thread( { handle=-1 , name="main"} ) -- Wrap the main thread

if core.handle( -2 ) then -- check if house thread exists

	wire.thread( { handle=-2 , name="house" } ) -- Wrap the house thread

else -- Create the house thread ( we are the main thread and no other threads exist yet )

	assert( our_thread_handle == -1 ) -- main thread check
	wire.thread( { handle=-2 , name="house" , start="require('wire').do_start_house()" , globals={house="yes"} , } )

end

if our_thread_handle<-2 then -- we are not main or house so also wrap our thread handle

	wire.thread( { handle=our_thread_handle } ) -- just wrap

end

-- make sure global thread shortcuts exist
wire.threads.main  = assert( wire.thread( -1 ) )
wire.threads.house = assert( wire.thread( -2 ) )
wire.threads.us    = assert( wire.thread( our_thread_handle ) )

if our_thread_handle == -1 then
	wire.do_start() -- set thready globals on main thread
DUMP(wire)
end

