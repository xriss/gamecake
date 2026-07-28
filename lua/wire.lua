--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

local cmsgpack = require("cmsgpack")

--[[#lua.wire

	local wire=require("wire")

We use wire as the local name of this library. So we can, uhh, wire 
things together?

Wire provides multiple long lived lua states managed via standard c11 
threads and mutexs with simple fifo message handling between threads.

May try and change to atomic fifos in the future to reduce the 
possibility of deadlocks but for now I think mutexs are the right 
choice. Partially due to the lua overhead and partially because this is 
not intended for sending millions of packets per second between tightly 
coupled threads.

A small compatibility library is needed for building for windows using 
mingw. Native windows builds might work if your compiler supports the 
C11 standard.

We are not doing anything clever, so beware of the following:
 
Only data is passed and that data must fit in messagepack, eg json like 
but binary strings are allowed.

Do not use lua tables that are both arrays and objects. Pick one or the 
other it will probably work but do not do it in case I have to break 
that in the future.

No userdata, no functions.

When in doubt, send a string, then unpack it in the thread. Binary 
strings will sometimes need to be escaped into lua code, eg \0 for null 
etc but this only happens to globals passed into a thread. Data is 
usually passed in message pack streams.

Any lua libs required in a thread *must* be thread safe or they will 
break in strange and uncomfortable ways. So check that any library you 
are using has luaopen_ functions do not try and initialize anything 
twice if opened in another thread.

This library does not reuse handles ( as this is intended for long 
lived threads ) and the number of handles is a hard limit set in the C 
code. Defaults to 4096 threads and 4096 fifos. Bump it at compile time 
if you need more.

You can reuse handles, but it must be self managed and must be done 
explicitly.

EG: You might want to shut down threads, clear out any pending data and 
then restart game logic threads using the same handles and names. This 
is possible but it is up to you to do it right. For instance the thread 
must be written to cooperate with the shut down.

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

-- active memos for this thread grouped by state. 
wire.memos={
	all={},
	setup={},
	sent={},
	reply={},
	result={},
	got={},
}

--[[#lua.wire.table_to_data

	data = wire.table_to_data( table )

Convert a lua table into a data (string).

We currently use cmsgpack but this may change to another system and can 
not be relied upon. It is even possible that data may not be a string 
in future versions.

This is a local function for local people. It should not be used by 
you.

]]
wire.table_to_data = cmsgpack.pack


--[[#lua.wire.data_to_table

	table = wire.data_to_table( data )

Convert a data (string) into a lua table.

We currently use cmsgpack but this may change to another system and can 
not be relied upon. It is even possible that data may not be a string 
in future versions.

This is a local function for local people. It should not be used by 
you.

]]
wire.data_to_table = function(data)
	-- cmsgpack packs an empty table as nil...
	return cmsgpack.unpack(data) or {}
end


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

--[[#lua.wire.active

	active = wire.active(handle)
	active = wire.active(thread)

Check if this handle is an active thread, Eg the thread is running and 
should be dealing with memos.

returns false if not a thread or thread has halted or thread has been 
asked to halt.

returns true if thread exists and is running.

This function should be used in a running thread to check if that 
thread has been asked to halt.

]]
wire.active=function(handle)

	if type(handle)=="table" then handle=handle.handle end
	
	local status = core.thread_status(handle)

	if not status then return false end -- unknown thread
	
	if status>0 then return true end -- thread is running

	return false -- thread is halting or halted
end

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
		memo:send()
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
of the start string. So binary data strings are not ideal but will 
work.

	opts.preloadlibs=wire.preloadlibs
	
Must be a cfunction to be run in new thread. Use false to not preload 
any custom libs. When running under gamecake this is automatically 
filled in and makes all the internal modules available. If you want to 
use this module outside of gamecake ( possible but not overly tested ) 
then you may need to provide your own.

	opts.header=...
	opts.footer=...

internal setup so best not to change this. It is used by 
wire.prepare_start to wrap your start code string with helpful error 
handlers etc.

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

We will raise an error if no fifo is found with that handle or name.

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

memo.state will be set to "setup"

CRAZY HAX TBH : memo.id is set to a light userdata of the memo tables 
pointer, this should be unique across multiple lua states and threads 
in the same process, for as long as the table stays on the stack. A 
future lua garbage collection system could break this by moving tables 
around in system memory but I believe this is currently "safe" as of 
2026 in all available versions of lua/luajit and if that changes I will 
fix it.

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

--[[#lua.wire.memo.remove

	memo:remove()

Remove a memo from wire.memos

memo.state will be changed to "removed" and it can no longer be found 
in wire.memos .

This means it is no longer a live memo as far as wire is concerned.

You may of course keep this memo around if you need it.

]]
wire.memo_functions.remove=function( memo )

	assert( memo.state~="removed" )
	wire.memos.all[ memo.id ]=nil
	wire.memos.all[ memo.state ]=nil

end

--[[#lua.wire.memo.status

	memo:status(state)

Change status of memo, moving it between wire.memos[ memo.state ] 
caches.

This is a local function for local people. It should not be used by 
you.

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

Send a memo to memo.fifo , memo.fifo may be a fifo/thread or a handle.

memo.state will be changed to "sent"

]]
wire.memo_functions.send=function( memo )

	assert(memo.fifo) -- memo must have destination fifo / handle
	local handle=0
	if type(memo.fifo)=="number" then handle=memo.fifo else handle=memo end

	local data=wire.table_to_data(memo.data)
	core.fifo_push( handle , data , wire.threads.us.handle , memo.id )
	memo:status("sent")

end

--[[#lua.wire.memo.send

	memo:reply()

Reply to a memo, this uses the senders handle as a fifo without it 
needing to be wrapped in a full fifo/thread.

memo.state will be changed to "reply"

]]
wire.memo_functions.send=function( memo )

	local data=wire.table_to_data(memo.result)
	core.fifo_push( memo.sender , data , memo.sender , memo.id )
	memo:status("reply")

end


--[[#lua.wire.memo.resolve

	result = memo:resolve()

Wait for a result on our thread fifo ( wire.threads.us ) before 
returning.

Other memos may have been pulled and ignored for now. These memos can 
be found later in wire.memos

result is returned and can also be found in memo.result this should 
always be a table containing the result of the memo provided by the 
thread.

If a result was returned with result.fail set then the thread replied 
with a hard fail. A hard fail means the thread could not deal with your 
memo, so lua code broke, memo was invalid etc. This is a hard fail and 
will raise an error rather than return a result.

If the thread does not reply but is still running then this will lock.

]]
wire.memo_functions.resolve=function( memo )

	-- wait for result , memos we ignore here can be handled later
	while wire.active( wire.threads.us.handle ) do
		local m = wire.threads.us:pull()

		if memo.result then -- got a result
			if memo.result.fail then error(memo.result.fail) end -- hard fail
			memo:remove() -- this memo is done
			return memo.result
		end

		if not m then -- we pulled nothing
			wire.threads.us:wait(1/1024) -- wait for new memos
		end
	end
	
	error("thread asked to halt")

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

If this is a memo for us to deal with memo.state will be set to "got"

If this is a result we will return the sent memo with memo.result set 
to the reply data and the memo state will be changed to "result".

memo may be ignored for now and dealt with later by finding it in 
wire.memos.

This is useful if you are waiting for a specific reply and want to keep 
pulling until you find it.

Returns nil if there is no memo available.

]]
wire.fifo_functions.pull=function( fifo )

	local data,sender,id=core.fifo_pull( fifo.handle )
	if not data then return end
	data=wire.data_to_table(data) -- unpack
	
	-- this is a reply sent back to us as it used our handle
	if wire.threads.us.handle == sender then

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

Wait upto the given number of seconds for a new memo to arrive in this 
fifo.

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
	while wire.active( wire.threads.us.handle ) do
		local memo = wire.threads.us:pull()
		
		if memo then

			-- this will print a TRACEBACK on error but keep going
			local ok,result = xpcall( function() return do_memo( memo.data ) end , TRACEBACK )

			if ok then
				memo.result=result -- do_memo gave us a result to reply with
			else
				memo.result={ fail=(result or "fail") } -- reply with fail reason
			end
			memo:reply()
			memo:remove()

		else

			wire.threads.us:wait(1/1024)

		end
	end
	-- we have been gracefully halted so cleanup and return

end



--[[#lua.wetgenes.tasks.http_code

A basic subtask to handle http memos.

Note that this function requires external libraries that must be 
available in order to work.

One of the main reasons to use threads is so you can do multiple non 
blocking http requests.

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

	-- loop until our thread is asked to halt
	while wire.active( wire.threads.us.handle ) do
		
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
	-- we have been gracefully halted so cleanup and return

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

-- make sure threads shortcuts exist
wire.threads.main  = assert( wire.thread( -1 ) )
wire.threads.house = assert( wire.thread( -2 ) )
wire.threads.us    = assert( wire.thread( our_thread_handle ) )

if our_thread_handle == -1 then
	wire.do_start() -- set thready globals on main thread
--UMP(wire)
end

