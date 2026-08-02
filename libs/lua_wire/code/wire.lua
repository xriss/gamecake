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

-- generic callbacks, referenced by name
wire.callbacks={}

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
wire.data_to_table = cmsgpack.unpack


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

--[[#lua.wire.time

	secs = wire.time()

Get the current TIME_UTC via a timespec_get and converted into a 
double.

]]
wire.time=core.time -- copy the core cfunction

--[[#lua.wire.timeres

	secs = wire.timeres()

Get the TIME_UTC resolution via a timespec_getres and converted into a 
double.

]]
wire.timeres=core.timeres -- copy the core cfunction

--[[#lua.wire.active

	active = wire.active(handle)
	active = wire.active(thread)

If handle is nil then use wire.thread_handle

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
	if not handle then handle=wire.thread_handle end
	
	local status = core.thread_status(handle)

	if not status then return false end -- unknown thread
	
	if status>0 then return true end -- thread is running

	return false -- thread is halting or halted
end

--[[#lua.wire.wrap

	it = wire.wrap(name,handle)

We *know* that this handle exists. So either return a previously cached 
fifo/thread or create a new one with the given name and return that.

The returned value may be a fifo or a thread, it.is will be "thread" or 
"fifo" for the different results.

Always returns a valid table.

]]
wire.wrap=function(name,handle)

	-- first try our cache
	if handle<0 then
		if wire.cache_threads[handle] then
			return wire.cache_threads[handle]
		end
	else
		if wire.cache_fifos[handle] then
			return wire.cache_fifos[handle]
		end
	end

	-- create wrapper for handle that we know exists
	if handle<0 then
		local thread = {}
		setmetatable(thread,wire.thread_metatable)
		thread.handle=handle
		thread.name=name

		wire.cache_threads[thread.handle]=thread
		wire.cache_names[name]=thread.handle
		wire.threads[name]=thread

		return thread
	else
		local fifo = {}
		setmetatable(fifo,wire.fifo_metatable)
		fifo.handle=handle
		fifo.name=name

		wire.cache_fifos[fifo.handle]=fifo
		wire.cache_names[name]=fifo.handle
		wire.fifos[name]=fifo

		return fifo
	end

end

--[[#lua.wire.manifest

	it = wire.manifest(name)
	it = wire.manifest(handle)

Return a fifo or thread if it exists, this is a way of wrapping a 
handle/name that you hope already exists but you are not 100% sure.

Will returns nil if the name or handle does not exist.

We may talk to the house thread to check for the existence of name or 
handle, so this can block for a while before returning.

The returned value may be a fifo or a thread, it.is will be "thread" or 
"fifo" for the different results.

]]
wire.manifest=function(name)

	local handle
	if type(name)=="number" then
		handle=name
		name=nil
	else
		handle=wire.cache_names[ name ] -- try our cache
	end

	if handle then -- try this handle
		if handle<0 then
			if wire.cache_threads[handle] then
				return wire.cache_threads[handle]
			end
		else
			if wire.cache_fifos[handle] then
				return wire.cache_fifos[handle]
			end
		end
	end

	-- check with house if we only have a handle or a name
	if ( ( name and ( not handle ) ) or ( handle and ( not name ) ) )
		and ( wire.thread_handle ~= -2 ) then -- unless we are house
		
		local result=wire.memo({
			fifo=wire.threads.house,
			data={
				action="handle",
				name=name,handle=handle, -- one of these is nil
			},
		}):resolve()
		
		handle=result.handle
		name=result.name
		
		if name and handle then -- house says it exists, so make it so
			return wire.wrap(name,handle)
		end
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

		thread.handle=core.thread_create( thread.handle or 0 )
		 -- auto generate name 
		if not name then name="thread"..thread.handle end
		
		core.thread_start( thread.handle , name , start , preloadlibs or nil )

	end
	
	if not core.handle( thread.handle ) then
		error("wire thread creation failed")
	end
	
	wire.cache_threads[thread.handle]=thread
	if name then
		thread.name=name
		wire.cache_names[name]=thread.handle
		wire.threads[name]=thread

		-- if the thread is not main or house
		if thread.handle<-2 then
			-- unless we are house
			if wire.thread_handle ~= -2 then
				wire.memo({
					fifo=wire.threads.house,
					callback=wire.callbacks.remove, -- auto remove
					data={
						action="handle",
						name=thread.name,
						handle=thread.handle,
					},
				}):send()
			end
		end
	end
	
	return thread
end

wire.thread_start=function(opts)

	local name=opts.name
	local preloadlibs=opts.preloadlibs -- set to false for no preload
	if type(preloadlibs)=="nil" then preloadlibs=wire.preloadlibs end

	local start=wire.prepare_start(opts)
	assert( loadstring(start) , "wire thread start string" ) -- make sure the code is valid

	core.thread_start( opts.handle , name , start , preloadlibs or nil )

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
		fifo.name=name
		wire.cache_names[name]=fifo.handle
		wire.fifos[name]=fifo

		-- tell house about this unless we are house
		if wire.thread_handle ~= -2 then
			wire.memo({
				fifo=wire.threads.house,
				callback=wire.callbacks.remove, -- auto remove
				data={
					action="handle",
					name=fifo.name,
					handle=fifo.handle,
				},
			}):send()
		end
	end

	return fifo
end


--[[#lua.wire.update

	wire.update()

Fetch and process memos, should be called at least once a second to 
deal with memo replies.

memo callbacks will be launched from this function.

]]
wire.update=function()

	-- pull all waiting memos ?
	while wire.active( wire.thread_handle ) do
		local m
		-- this may raise an error, just print and continue
		local suc,err=xpcall(
			function() m = wire.threads.us:pull() end ,
			TRACEBACK )
		if suc and not m then break end -- we pulled nothing
	end
	
	-- memos with results
	for id , memo in pairs(wire.memos.result) do
		if memo.callback then
			-- this may raise an error, just print and continue
			local suc,err=xpcall(
				function() memo.callback(memo) end ,
				TRACEBACK )
			memo:remove() -- do not call again
		end
	end

end

--[[#lua.wire.memo

	memo = wire.memo( )
	memo = wire.memo( {} )
	memo = wire.memo( { sender=handle , id=id , callback=callback } )

Create a memo and return it, if you pass in a new table with settings 
then that is the table we will modify and return.

memo.sender can be provided or it will be set to our handle ( which is 
wire.thread.us.handle )

memo.id can be provided or it will be generated.

The memo will also be placed in wire.memos for later processing.

memo.state will be set to "setup"

if memo.callback is provided then it will be called during wire.update 
after we get a reply. It will be called like so:
	
	memo.callback(memo)


FYI HAX TBH : memo.id is set to a light userdata of the memo tables 
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
		memo.sender = wire.thread_handle -- we is who we is
	end

	wire.memos.all[ memo.id ]=memo
	memo:status("setup")

	return memo
end

--[[#lua.wire.memo.remove

	memo:remove()

Remove a memo from wire.memos.

memo.state will be changed to "removed" and it can no longer be found 
in wire.memos

This means it is no longer a live memo as far as wire is concerned.

You may of course keep this memo around if you need it.

]]
wire.memo_functions.remove=function( memo )

	wire.memos.all[ memo.id ]=nil
	if memo.state and wire.memos[ memo.state ] then
		wire.memos[ memo.state ][ memo.id ]=nil
	end
	memo.state="removed"

	return memo
end

--[[#lua.wire.callbacks.discard

	wire.memo({ callback=wire.callbacks.discard })

This is a generic callback to be used when you do not care about a 
memos result.

This callback will remove the memo, eg it will call memo:remove()

If you do not remove memos when you are finished with them then they 
will just accumulate inside the wire.memos table. So it is important to 
always use this callback if you are not going to check on a memo result 
later.

]]
wire.callbacks.discard=wire.memo_functions.remove


--[[#lua.wire.memo.status

	memo:status(state)

Change status of memo, moving it between wire.memos[ memo.state ] 
caches.

This is a local function for local people. It should not be used by 
you.

]]
wire.memo_functions.status=function( memo , state )
	if memo.state and wire.memos[ memo.state ] then
		wire.memos[ memo.state ][ memo.id ]=nil
	end
	memo.state=state
	if memo.state and wire.memos[ memo.state ] then
		wire.memos[ memo.state ][ memo.id ]=memo
	end
	return memo
end

--[[#lua.wire.memo.send

	memo:send()

Send a memo to memo.fifo , memo.fifo may be a fifo/thread or a handle.

memo.state will be changed to "sent"

]]
wire.memo_functions.send=function( memo )
	assert(memo.fifo) -- memo must have destination fifo / handle
	local handle=0
	if type(memo.fifo)=="number" then handle=memo.fifo else handle=memo.fifo.handle end

	local data=wire.table_to_data(memo.data)
	core.fifo_push( handle , data , wire.thread_handle , memo.id )
	memo:status("sent")
	-- remember the time we sent
	-- old memos waiting on a reply can produce debug warnings, eventually
	memo.send_time=wire.time()

	return memo -- chainable
end

--[[#lua.wire.memo.send

	memo:reply()

Reply to a memo, this uses the senders handle as a fifo without it 
needing to be wrapped in a full fifo/thread.

memo.state will be changed to "reply"

]]
wire.memo_functions.reply=function( memo )

	local data=wire.table_to_data(memo.result)
	core.fifo_push( memo.sender , data , memo.sender , memo.id )
	memo:status("reply")

	return memo
end


--[[#lua.wire.memo.resolve

	result = memo:resolve()
	result = wire.memo(...):resolve()

If memo.state is "setup" then we memo:send() the memo. "setup" is the 
state a newly created memo will be in.

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

If the thread does not reply then this will lock.

If the currently running thread is asked to halt then this will raise 
an error rather than return.

]]
wire.memo_functions.resolve=function( memo )

	if memo.state=="setup" then -- auto send
		memo:send()
	end

	-- wait for result , memos we ignore here can be handled later
	while wire.active( wire.thread_handle ) do
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
	
	error("thread halted in memo:resolve")

end


--[[#lua.wire.fifo.peek

	bool = fifo:peek()
	bool = thread:peek()

Peek and see if there is a memo to pull but do not pull it.

Note that a pull, even immediately after a successful peek has no 
guarantee to work.

If you wish to actually look at the contents of the memo then pull a 
memo and then push it back into the fifo. This will of course change 
the order of memos in the fifo.

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
	if wire.thread_handle == sender then

		local memo=assert( wire.memos.all[ id ] ) -- must match a sent memo

		memo.reply_time=wire.time() -- so we can compare with send_time
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
	core.fifo_push( fifo.handle , data , wire.thread_handle , memo.id )
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
	-- pass paths into children
	fout("package.path = ", wire.serialize(package.path) )
	fout("package.cpath = ", wire.serialize(package.cpath) )

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

--[[#lua.wire.do_start

Wrapper code used in a new thread to set things up and handle errors 
etc.

]]
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
	global.TRACEBACK=function(err) LOG( "TASK" , debug.traceback( err ) ) return err end

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

--[[#lua.wire.tasks

	wire.tasks(name,count,code)
	wire.tasks(name,0)

Start or halt the named tasks, we plan to be running count number of 
named tasks after calling this.

name is the type of task, eg "http". A fifo of that name will be 
created when creating tasks ( if one does not already exist ) for 
sending mwmo work request to and all tasks created will have their 
names prefixed with this name.

count is the number of tasks we want, call with 0 and we will halt all 
named tasks.

code is the string of lua code to run in each task, eg for http tasks 
it would be "require('wire').http_code()" to run the wire.http_code 
function in each task. If count zero, this may be skipped.

We will try and reuse old tasks, if stopping and starting, but really 
all you should need to do is call at startup and then you are good to 
go until the process shuts down.

]]
wire.tasks=function(name,count,code)

	-- easiest to just run on the house thread, so auto promote
	if wire.threads.us.name~="house" then
		return wire.memo({
			fifo=wire.threads.house,
			data={
				action="tasks",
				name=name,
				count=count,
				code=code,
			},
		}):resolve()
	end
	-- we are now house so do the thing
	
	local result={}

	if not wire.cache_tasks then wire.cache_tasks={} end
	
	if wire.cache_tasks[name] then
		result.task=wire.cache_tasks[name]
	else
		result.task={count=0,name=name,code=code,list={}}
		wire.cache_tasks[name]=result.task -- remember task
	end
	local task=result.task

	if task.count==count then --nothing to change
		return result
	end

	-- ask the extra tasks to halt but we do not wait for them
	for idx,handle in ipairs( task.list ) do
		if idx>count then
			core.thread_status(handle,-1) -- ask to halt
		end
	end

	-- create new threads
	for idx=1,count do
		local handle=task.list[idx]
		if not handle then -- create task
			local thread=wire.thread( {
				name=name.."-"..idx ,
				start=code } )
			task.list[idx]=thread.handle
		else -- task exists but may not be running, ask it to start again
			local thread=wire.thread(handle) -- get thread from handle
			if not wire.active(handle) then -- need to restart
				wire.thread_start({
						name=name.."-"..idx ,
						handle=handle,
						start=code,
					})
			end
		end
	end

	-- create new fifo unless it exists
	if not wire.fifos[name] then
		wire.fifo({name=name})
	end

	return result
end

--[[#lua.wire.house_code

The message loop code we run in the house thread (-2) to synchronize 
process wide task names and handles.

]]
wire.house_code=function()

	local consume=function(data)
		local result={fail="unknown"} -- unknown error

			if data.action=="handles" then -- get all the names->handles we know			

				result.handles=wire.cache_names

			elseif data.action=="handle" then -- get/set a single handle/name pair

				if data.name and data.handle then

					wire.cache_names[ data.name ] = data.handle
					
					result.name=data.name
					result.handle=data.handle
					
					result.fail=nil -- success

				elseif data.name and (not data.handle) then

					result.name=data.name
					result.handle=wire.cache_names[ data.name ]

					result.fail=nil -- success

				elseif (not data.name) and data.handle then

					result.handle=data.handle

					result.fail=nil -- success

				end

			elseif data.action=="tasks" then -- wire.tasks
			
				result=wire.tasks( data.name , data.count , data.code )

			end


		return result
	end

	-- deal with memos sent directly to us ( -2 ) 
	while wire.active( wire.thread_handle ) do
		local memo = wire.threads.us:pull()
		
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

			wire.threads.us:wait( 1/16 )

		end
	end
	-- we have gracefully halted so cleanup and return

end



------------------------------------------------------------------------
-- auto setup

-- thread -1 will always exist and thread -2 will be auto created here
do
	-- use global ( if it exists ) to protect globals from accidental use
	local global=_G ; pcall(function() global=require("global") end)

	-- get our thread handle and name
	wire.thread_handle , wire.thread_name = core.thread_handle()
	global.TASK_NAME=wire.thread_name -- used by log code

	-- if we are main thread do some thread setup
	if wire.thread_handle == -1 then
		wire.do_start() -- set thready globals on main thread
		if wire.timeres()>0.001 then -- check for crazy OS
			PRINT( string.format("WARNING wire.time resolution (%.9f) is greater than 0.001",wire.timeres()) )
		end
	end

	-- create main thread
	wire.wrap( "main" , -1 ) -- Wrap the main thread
	if wire.thread_handle == -1 then
		wire.threads.us = wire.threads.main -- we are the main thread
	end

	-- create house thread
	if core.handle( -2 ) then -- check if house thread exists
		wire.wrap( "house" , -2 ) -- Wrap the house thread
	else -- Create the house thread ( we are the main thread and no other threads exist yet )
		assert( wire.thread_handle == -1 ) -- main thread check
		wire.thread( { handle=-2 , name="house" , start="require('wire').house_code()" , globals={house="yes"} , } )
	end
	if wire.thread_handle == -2 then
		wire.threads.us = wire.threads.house -- we are the house thread
	end

	-- create us thread
	if wire.thread_handle<-2 then -- we are not main or house
		wire.threads.us = wire.wrap( wire.thread_name , wire.thread_handle )
	end

	-- name may have changed
	global.TASK_NAME=wire.thread_name

end
