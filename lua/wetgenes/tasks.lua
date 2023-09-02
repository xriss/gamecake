--
-- (C) 2022 Kriss@XIXs.com
--

-- should not cache this stuff when using lanes just in case we try and share upvalues across threads

--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
--     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.tasks_functions={}
M.tasks_metatable={__index=M.tasks_functions}

--[[#lua.wetgenes.tasks.thread_code

Handle global tasks, starting and stopping and preventing the starting 
of multiple copies of the same task.

]]
M.tasks_functions.thread_code=function(linda,task_id,task_idx)
set_finalizer(function(err,stack)
	print("LANES:",err,table.concat(stack,"\n"))
end)

	local log,dump=require("wetgenes.logs"):export("log","dump")

	local tasks=require("wetgenes.tasks").create({linda=linda}) -- another sub tasks

	while true do
		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread

		if memo then
			local ret={result=false}
			
			if     memo.cmd=="add" then
				if not tasks.thread[memo.thread.id] then -- only create once
					tasks:add_thread(memo.thread)
					ret.result=true
					log("thread","created",memo.thread.id)
				else
					log("thread","already exists",memo.thread.id)
				end
			elseif memo.cmd=="del" then
				local id=memo.thread and memo.thread.id
				if id then -- end a single task
					if tasks.thread[id] then -- must exist to end
						log("thread","destroyed",id)
						tasks:del_thread(tasks.thread[id])
					else
						log("thread","does not exist",id)
					end
				else -- end all tasks
					for id,thread in pairs(tasks.thread) do
						tasks:del_thread(thread)
						log("thread","destroyed",id)
					end
				end
			end
			
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end
	end
end


--[[#lua.wetgenes.tasks.global_code

A basic function to handle global memos to get/set data shared amongst multiple tasks.

]]
M.tasks_functions.global_code=function(linda,task_id,task_idx)
set_finalizer(function(err,stack)
	print("LANES:",err,table.concat(stack,"\n"))
end)

	local data={}

	while true do

		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			local ret={result=false}
			
			if     memo.cmd=="claim" then
				if memo.name and memo.value then
					if not data[memo.name] then	-- must be empty
						data[memo.name]=memo.value
						ret.result=memo.value
					end
				end
			elseif memo.cmd=="eject" then
				if memo.name then
					if data[memo.name] then -- must exist
						ret.result=data[memo.name]
						data[memo.name]=nil
					end
				end
			elseif memo.cmd=="fetch" then
				if memo.name then
					ret.result=data[memo.name]
				end
			end
			
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end


--[[#lua.wetgenes.tasks.http_code

A basic function to handle http memos.

]]
M.tasks_functions.http_code=function(linda,task_id,task_idx)
set_finalizer(function(err,stack)
	print("LANES:",err,table.concat(stack,"\n"))
end)

	local js_eval -- function call into javascript if we are an emcc build
	do
		local ok,lib=pcall(function() return lanes.require("wetgenes.win.emcc") end )
		if ok and lib then js_eval=lib.js_eval end
	end

	local lanes = require("lanes")
	local http = lanes.require("socket.http")
	local ltn12 = lanes.require("ltn12")

	local wjson = lanes.require("wetgenes.json")

-- we need a special case for emcc as we can only use websockets or javascript requests
	local function request_js(memo)
	
		local opts={}
		
		opts.method=memo.method
		opts.url=memo.url
		opts.headers=memo.headers or {}
		opts.body=memo.body
		
--		dump(opts)

		local js=[[
(function(opts){

	var request = new XMLHttpRequest();
	request.open( opts.method , opts.url , false );
	for(const key in opts.headers)
	{
		request.setRequestHeader( key , opts.headers[key] );
    }
  	request.send(opts.body);

	var ret={};
	ret.body=request.responseText;
	ret.code=request.status;
	ret.headers=request.getAllResponseHeaders();
	
	return JSON.stringify(ret);

})(]]..wjson.encode(opts)..[[);
]]
	
		local rets=js_eval(js)
		local ret=wjson.decode( rets or "{}" ) or {}
		if not ret.body then ret.error=ret.code or 0 end

--		dump(ret)
		
		return ret
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

			memo.body=wjson.encode(memo.json)
			memo.method="POST"
			memo.headers["Content-Type"]="application/json"
			
		end

		if memo.post then -- we want to send all these values in a POST body

			local t={}
			for n,v in pairs(memo.post) do
				t[#t+1]=urlencode(n) .. "=" .. urlencode(v)
			end
			memo.body=table.concat(t,"&")
			memo.method="POST"
			memo.headers["Content-Type"]="application/x-www-form-urlencoded"
			
		end
		
		memo.method=memo.method or "GET"

		if js_eval then return request_js(memo) end

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


	while true do

		local _,memo= linda:receive( nil , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			local ok,ret=xpcall(function() return request(memo) end,function(err) return debug.traceback(err) end) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end

--[[#lua.wetgenes.tasks.sqlite_code

A basic function to handle sqlite memos.

As we are opening an sqlite database here it wont help much to have 
more than one thread per database as they will just fight over file 
access.

]]
M.tasks_functions.sqlite_code=function(linda,task_id,task_idx)
set_finalizer(function(err,stack)
	print("LANES:",err,table.concat(stack,"\n"))
end)

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
			local ok,ret=xpcall(function() return request(memo) end,function(err) return debug.traceback(err) end) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end

end

--[[#lua.wetgenes.tasks.client_code

A basic function to handle (web)socket client connection.

]]
M.tasks_functions.client_code=function(linda,task_id,task_idx)

set_finalizer(function(err,stack)
	print("LANES:",err,table.concat(stack,"\n"))
end)

	set_debug_threadname(task_id)

	local wjson = lanes.require("wetgenes.json")
	local js_eval -- function call into javascript if we are an emcc build
	do
		local ok,lib=pcall(function() return lanes.require("wetgenes.win.emcc") end )
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
			local ok,ret=xpcall(function() return request(memo) end,function(err) return debug.traceback(err) end) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

	end
	
end






local log,dump=require("wetgenes.logs"):export("log","dump")

--[[#lua.wetgenes.tasks

	local tasks=require("wetgenes.tasks").create()

Manage tasks that should be performed on seperate threads so as not to block the main thread.

Manage coroutines that can then easily call into these threads by yeilding

]]

local lanes=require("lanes")
if lanes.configure then -- first time configuration
	lanes=lanes.configure(
		{
			with_timers = false ,
			on_state_create = require("wetgenes.gamecake.core").preloadlibs ,	-- this makes lanes.require work on internal modules inside a new thread
		}
	)
end


--[[#lua.wetgenes.tasks.do_memo

	-- just grab this function 
	local do_memo=require("wetgenes.tasks").do_memo
	
	local r=do_memo(linda,{task="taskname"})
	local r=do_memo(linda,{task="taskname"},timeout)

A basic memo send and receive that can be used when all you have is the 
tasks.linda and you know you are in a thread ( or do not care about 
blocking while waiting for another thread ) Pretty much everything else 
in this module is about managing coroutines to pretended to be threads 
and keeping track of state for debugging but this is all you actually 
need to use to simply communicate with a task. Especially useful if you 
are in a task and want to talk to another task.

]]
M.do_memo=function(linda,memo,timeout)
	memo.id=tostring(memo) -- should be a unique string, the address of the memo table
--log("memo",memo.task,memo.id)
	if linda:send( timeout , memo.task , memo ) then -- send on memo.task (a public name of another task)
		local ok,r=linda:receive( timeout , memo.id ) -- receive the result on memo.id
--log("memo",memo.task,memo.id,"done")
		return r
	end
end


--[[#lua.wetgenes.tasks.cocall

	require("wetgenes.tasks").cocall(f1,f2,...)
	require("wetgenes.tasks").cocall({f1,f2,...})

Manage simple coroutines that can poll each others results and wait on 
them.

Turn a table of "setup" functions into a table of coroutines that can 
yield waiting for other coroutines to complete and run them all.

You still need to be carefull with race conditions but it allows you to 
write code in such away that setup order is no longer important. Setup 
functions can coroutine.yield waiting for another setup to finish first.

]]

M.cocall=function(...)
	local functions={...}
	if type(functions[1])=="table" then functions=functions[1] end -- a table of functions rather than a list

	local coroutines={}

	for n,f in pairs(functions) do
		coroutines[n]=coroutine.create(f)
	end

	local start_time=os.time()
	repeat
		local dump=false
		local time=os.time()-start_time
		if time >=10 then
			start_time=os.time()
			dump=true
		end
		
		local runcount=0
		for n,c in pairs(coroutines) do
			if coroutine.status( c )=="suspended" then

if dump then
	print( debug.traceback( c , "long time waiting" ) )
end
				runcount=runcount+1
				local ok , err = coroutine.resume( c )
				assert( ok , debug.traceback( c , err ) )
			end
		end
	until runcount==0

end




M.colinda_functions={}
M.colinda_metatable={__index=M.colinda_functions}

--[[#lua.wetgenes.tasks.add_id

	tasks:add_id(it)

Internal function to manage creation of all objects with unique ids.

]]
M.tasks_functions.add_id=function(tasks,it)
	it=it or {}
	it.id=it.id or tostring(it) -- auto generate id? ( this nay be unsafe ... )
	tasks.ids[it.id]=it or true -- keep everything in ids
	if it.type then tasks[it.type][it.id]=it end -- and each type has its own table
	return it
end

--[[#lua.wetgenes.tasks.del_id

	tasks:del_id(it)

Internal function to manage deletion of all objects with unique ids.

]]
M.tasks_functions.del_id=function(tasks,it)
	if it.id then
		if it.type then
			tasks[it.type][it.id]=nil
		end
		tasks.ids[it.id]=nil
	end
	return it
end

--[[#lua.wetgenes.tasks.claim_global

	tasks:claim_global(name,value)

Claim a name using the global linda socket, returns value on success 
or false on failure.

The value will be associated with the name on success.

This will block waiting on a result but should be fast.

]]
M.tasks_functions.claim_global=function(tasks,name,value)
	value=value or true -- value must must be trueish
	local memo={}
	memo.id=tostring(memo) -- should be a unique string, the address of the memo table
	memo.task="global"
	memo.cmd="claim"
	memo.name=name
	memo.value=value
	if tasks.linda:send( nil , memo.task , memo ) then -- send on memo.task (a public name of another task)
		local ok,r=tasks.linda:receive( nil , memo.id ) -- receive the result on memo.id
		return r.result
	end
end

--[[#lua.wetgenes.tasks.eject_global

	tasks:eject_global(name)

Eject a name using the global linda socket, returns value associated 
with name on success or false on failure.

The value will no longer be associated with the name when this succeeds.

This will block waiting on a result but should be fast.

]]
M.tasks_functions.eject_global=function(tasks,name)
	local memo={}
	memo.id=tostring(memo) -- should be a unique string, the address of the memo table
	memo.task="global"
	memo.cmd="eject"
	memo.name=name
	if tasks.linda:send( nil , memo.task , memo ) then -- send on memo.task (a public name of another task)
		local ok,r=tasks.linda:receive( nil , memo.id ) -- receive the result on memo.id
		return r.result
	end
end

--[[#lua.wetgenes.tasks.fetch_name

	tasks:fetch_global(name)

Fetch value associated with the name using the global linda socket or 
false on failure. You can not associated nil or false with a global value.

This will block waiting on a result but should be fast.

]]
M.tasks_functions.fetch_global=function(tasks,name)
	local memo={}
	memo.id=tostring(memo) -- should be a unique string, the address of the memo table
	memo.task="global"
	memo.cmd="fetch"
	memo.name=name
	if tasks.linda:send( nil , memo.task , memo ) then -- send on memo.task (a public name of another task)
		local ok,r=tasks.linda:receive( nil , memo.id ) -- receive the result on memo.id
		return r.result
	end
end

--[[#lua.wetgenes.tasks.add_memo

	local memo=tasks:add_memo({})
	
Create a memo with a unique auto generated numerical id for linda 
comunication.

]]
M.tasks_functions.add_memo=function(tasks,memo)
	memo=memo or {}
	memo.type="memo"
	memo.state="setup"
	tasks:add_id(memo)
	return memo
end

--[[#lua.wetgenes.tasks.del_memo

	tasks:del_memo(memo)
	
Delete a memo.

]]
M.tasks_functions.del_memo=function(tasks,memo)
	tasks:del_id(memo)	
	return memo
end

--[[#lua.wetgenes.tasks.add_global_thread

	tasks:add_global_thread(thread)

This runs an add_thread inside a named thread so all threads are kept 
together no matter which thread tried to start it.

thread is the same as add_thread

if the thread id already exists then it will not be added again.

]]
M.tasks_functions.add_global_thread=function(tasks,thread)
	local memo={
		task="thread",
		cmd="add",
		thread=thread,
	}
	tasks:do_memo(memo)
end

--[[#lua.wetgenes.tasks.del_global_thread

	tasks:del_global_thread({id="threadname"})

Destroy a given thread.id or all the threads if thread is nil.

]]
M.tasks_functions.del_global_thread=function(tasks,thread)
	local memo={
		task="thread",
		cmd="del",
		thread=thread,
	}
	tasks:do_memo(memo)
end

--[[#lua.wetgenes.tasks.add_thread

	local thread=tasks:add_thread({
		id="test",
		count=1,
		code=function(linda,task_id,task_idx)
			while true do
				local _,memo= linda:receive( nil , task_id )
				if memo then
					...
				end
			end
		end,
	})
	
Create a thread with various preset values:

	id

A unique id string to be used by lindas when sending messages into this 
task. The function is expected to sit in an infinite loop waiting on 
this linda socket.

	count

The number of threads to create, they will all use the same instanced 
code function so should be interchangable and it should not matter 
which thread we are actually running code on. If you expect the task to 
maintain some state between memos, then this must be 1 .

	code

A lua function to run inside each thread, this function will recieve 
tasks.linda and the task.id for comunication and an index so we know 
which of the count threads we are (mostly for debugging)


]]
M.tasks_functions.add_thread=function(tasks,thread)
	thread=thread or {}
	thread.type="thread"
	tasks:add_id(thread)

	thread.count=thread.count or 1
	thread.start=lanes.gen( "*" , { ["globals"]=thread.globals } , thread.code ) -- prepare task
	thread.handles={}
	for idx=1,thread.count do thread.handles[idx]=thread.start( tasks.linda , thread.id , idx ) end -- start tasks
	
	return thread
end

--[[#lua.wetgenes.tasks.del_thread

	tasks:del_thread(thread)
	
Delete a thread.

]]
M.tasks_functions.del_thread=function(tasks,thread)
	for i=#thread.handles,1,-1 do -- cancel all the handles
		thread.handles[i]:cancel()--1/10,true) -- give it a chance
		thread.handles[i]=nil
	end
	tasks:del_id(thread)	
	return thread
end

--[[#lua.wetgenes.tasks.add_task

	local thread=tasks:add_task({
		id="test",
		code=function(linda,task_id,task_idx,task)
			while true do
				local _,memo= linda:receive( 0 , task_id )
				if memo then
					...
				end
			end
		end,
	})
	
Create a task with various preset values similar to a thread except 
inside a coroutine on the calling thread. As this function is inside a 
coroutine you must yield regulary this yield will then continue on the 
next update. Probably called once ever 60th of a second.

	id

A unique id string to be used by lindas when sending messages into this 
task. The function is expected to sit in an infinite loop testing 
this linda socket and then yielding if there is nothing to do.

	count

The number of tasks to create, they will all use the same instanced 
code function so should be interchangable and it should not matter 
which task we are actually running code on. If you expect the task to 
maintain some state between memos, then this must be 1 .

	code

A lua function to run inside a coroutine, this function will recieve 
tasks.linda (which is a colinda) and the task.id for comunication and 
an index so we know which of the count tasks we are (mostly for 
debugging) and finally the task table itself which make sense to share 
with coroutines.

]]
M.tasks_functions.add_task=function(tasks,task)
	if type(task)=="function" then task={code=task} end -- the simplest form is to just pass in a function
	task=task or {}
	task.type="task"
	tasks:add_id(task)
	
	task.count=task.count or 1
	task.errors={}
	task.handles={}
	for idx=1,task.count do
		task.handles[idx]=coroutine.create(task.code)
		local ok , err = coroutine.resume( task.handles[idx] , tasks.colinda , task.id , idx , task ) -- first call passing in args
		if not ok then task.errors[idx]=err end
	end

	return task
end

--[[#lua.wetgenes.tasks.run_task

	tasks:run_task(task)
	
Resume all the coroutines in this task.

Any errors will be logged with a backtrace.

If the tasks have finished running (returned or crashed) then we will 
tasks:del_task(task) this task. Check task.id which will be nil after 
this task has finished.

]]
M.tasks_functions.run_task=function(tasks,task)

	local runcount=0

	for idx=1,task.count do
		if coroutine.status( task.handles[idx] )=="suspended" then
			runcount=runcount+1
			local ok , err = coroutine.resume( task.handles[idx] )
			if not ok then
				task.errors[idx]=err
				log("tasks" , debug.traceback( task.handles[idx] , err ) )
			end
		else
			if task.errors[idx] then
				log("tasks" , debug.traceback( task.handles[idx] , task.errors[idx] ) )
			end
		end
	end
	
	if runcount==0 then -- coroutines are not runnig
		tasks:del_task(task)
	end

	return task
end

--[[#lua.wetgenes.tasks.del_task

	tasks:del_task(task)
	
Delete a task.

]]
M.tasks_functions.del_task=function(tasks,task)
	tasks:del_id(task)	
	return task
end

--[[#lua.wetgenes.tasks.update

	tasks:update()
	
Resume all current coroutines and wait for them to yield.

]]
M.tasks_functions.update=function(tasks)
	for idx,task in pairs(tasks.task) do
		tasks:run_task(task)
	end
end

--[[#lua.wetgenes.tasks.send

	memo = tasks:send(memo,timeout)
	memo = tasks:send(memo)
	
Send a memo with optional timeout.

This is intended to be run from within a coroutine task on the main 
thread. It will work outside of a task but that may block the main 
thread waiting to send.

if memo.id is not set then we will auto call add_memo to create it.

Check memo.error for posible error, this will be nil if everything went 
OK.

]]
M.tasks_functions.send=function(tasks,memo,timeout)
	if memo.error then return memo end
	if not memo.id then tasks:add_memo(memo) end -- auto add
	memo.state="sending"
	local ok=tasks.colinda:send( timeout , memo.task , memo )
	memo.state="sent"
	if not ok then memo.error="send failed" return memo end
	return memo
end

--[[#lua.wetgenes.tasks.receive

	memo = tasks:receive(memo,timeout)
	memo = tasks:receive(memo)
	
Recieve a memo with optional timeout.

This is intended to be run from within a coroutine task on the main 
thread. It will work outside of a task but that will block the main 
thread waiting for a response.

The memo will be deleted after being recieved (ie we will have called 
del_memo) so as to free up its comunication id for another memo.

if the memo has not yet been sent or even been through add_memo (we 
check state for "setup" or nil) then it will be autosent with the same 
timeout before we try and receive it.

After calling check if memo.error is nil then you will find the result in 
memo.result

]]
M.tasks_functions.receive=function(tasks,memo,timeout)
	if memo.error then return memo end
	if memo.state=="setup" or not memo.state then -- autosend
		tasks:send(memo,timeout)
	end
	memo.state="receiving"
	local ok,result=tasks.colinda:receive( timeout , memo.id )
	memo.state="done"
	tasks:del_memo(memo)

	if not ok then memo.error="receive failed" return memo end
	memo.result=result

	return memo
end

--[[#lua.wetgenes.tasks.do_memo

	result = tasks:do_memo(memo,timeout)
	result = tasks:do_memo(memo)

Similar to calling tasks:receive but without the problems that come 
from me trying to remember how to spell receive and it returns 
memo.result instead of memo so slightly less mess. This will assert on 
finding a memo.error so less need to check for errors.

]]
M.tasks_functions.do_memo=function(tasks,memo,timeout)
	tasks:send(memo,timeout)
--	log("memo",memo.task,memo.id)
	tasks:receive(memo,timeout)
--	log("memo",memo.task,memo.id,"done")
	assert(not memo.error,memo.error)
	return memo.result
end

--[[#lua.wetgenes.tasks.delete

	tasks:delete()
	
Force stop all threads and delete all data.

Failure to call this will allow any created threads to continue to run 
until program termination.

]]
M.tasks_functions.delete=function(tasks)
	if tasks.thread.thread then -- this is the global tasks
		tasks:del_global_thread()
	end
end

--[[#lua.wetgenes.tasks.create_colinda

	local colinda=require("wetgenes.tasks").colinda(linda)
	
Create a colinda which is a wrapper around a linda providing 
replacement functions to be used inside a coroutine so it will yield 
(and assume it will be resumed) rather than wait.

This should be a dropin replacement for a linda and will fallback to 
normal linda use if not in a coroutine.

If linda is nil then we will create one, the linda used in this colinda 
can be found in colinda.linda if you need raw access.

What we are doing here is wrapping the send/receive functions so that

	colinda:send(time,...)
	colinda:recieve(time,...)

will be replaced with functions that call

	linda:send(0,...)
	linda:recieve(0,...)

and use coroutines.yield to mimic the original timeout value without 
blocking.

]]
M.create_colinda=function(linda)
	if not linda then linda=lanes.linda() end
	local colinda={linda=linda}
	setmetatable(colinda,M.colinda_metatable)
	return colinda
end
	
M.colinda_functions.set=function(colinda,...)
	return colinda.linda:set(...)
end

M.colinda_functions.get=function(colinda,...)
	return colinda.linda:get(...)
end

M.colinda_functions.count=function(colinda,...)
	return colinda.linda:count(...)
end

M.colinda_functions.dump=function(colinda,...)
	return colinda.linda:cancel(...)
end

M.colinda_functions.cancel=function(colinda,...)
	return colinda.linda:cancel(...)
end

local checktimeout=function(timeout,...)
	local aa={...}
	local t=type(timeout)
	if ( t~="nil" and t~="number" ) then -- is the first arg a valid timeout
		timeout=nil
		table.insert(aa,1,timeout)
	end
	return timeout,aa
end

M.colinda_functions.send=function(colinda,...)
	if not coroutine.running() then
		return colinda.linda:send(...)
	end
	local timeout,aa=checktimeout(...)
	if timeout==0 then return colinda.linda:send(0,unpack(aa)) end -- no need to yield
	local timestart
	if timeout then timestart=os.time() end
	
	local ret
	repeat
		ret={ colinda.linda:send(0,unpack(aa)) }
		if ret[1] then break end -- got a result
		if timeout and os.time() > timestart+timeout then break end
		coroutine.yield()
	until false
	
	return unpack(ret)
end

M.colinda_functions.receive=function(colinda,...)
	if not coroutine.running() then
		return colinda.linda:receive(...)
	end
	local timeout,aa=checktimeout(...)
	if timeout==0 then return colinda.linda:receive(0,unpack(aa)) end -- no need to yield
	local timestart
	if timeout then timestart=os.time() end
	
	local ret
	repeat
		ret={ colinda.linda:receive(0,unpack(aa)) }
		if ret[1] then break end -- got a result
		if timeout and os.time() > timestart+timeout then break end
		coroutine.yield()
	until false

	return unpack(ret)
end

--[[#lua.wetgenes.tasks.create

Create a tasks group to contain all associated threads and coroutines 
along with their comunications.

]]
M.create=function(tasks)
	tasks=tasks or {}
	setmetatable(tasks,M.tasks_metatable)
	
	tasks.ids={}		-- unique ids within tasks
	tasks.memo={}		-- data sent between tasks
	tasks.thread={}		-- preemptive tasks
	tasks.task={}		-- cooperative tasks
	
	if not tasks.linda then -- create a new linda
		tasks.linda=lanes.linda()
		tasks.colinda=M.create_colinda(tasks.linda)
		tasks.colinda.tasks=tasks -- link back
		tasks:add_thread({
			count=1,
			id="thread",
			code=tasks.thread_code
		})
		tasks:add_thread({
			count=1,
			id="global",
			code=tasks.global_code
		})
	else
		tasks.colinda=M.create_colinda(tasks.linda)
		tasks.colinda.tasks=tasks -- link back
	end

	return tasks
end


--[[#lua.wetgenes.tasks.http

Create send and return a http memo result.

Returns either the result or nil,error so can be used simply with an 
assert wrapper.

]]
M.tasks_functions.http=function(tasks,memo,timeout)

	if type(memo) == "string" then memo={url=memo} end
	memo.task=memo.task or "http"
	
	tasks:send(memo,timeout)
--	log("memo",memo.task,memo.id)
	tasks:receive(memo,timeout)
--	log("memo",memo.task,memo.id,"done")

	if memo.error then return nil,memo.error end
	if memo.result and memo.result.error then return nil,memo.result.error end
	return memo.result
end

--[[#lua.wetgenes.tasks.sqlite

Create send and return a sqlite memo result.

Returns either the result.rows or nil,error so can be used simply with an 
assert wrapper.

Note that rows can be empty so an additional assert(rows[1]) might be 
needed to check you have data returned.

]]
M.tasks_functions.sqlite=function(tasks,memo,timeout)

	if type(memo) == "string" then memo={sql=memo} end
	memo.task=memo.task or "sqlite"
	
	tasks:send(memo,timeout)
--	log("memo",memo.task,memo.id)
	tasks:receive(memo,timeout)
--	log("memo",memo.task,memo.id,"done")

	if memo.error then return nil,memo.error end
	if memo.result.error then return nil,memo.result.error end
	return memo.result.rows
end



--[[#lua.wetgenes.tasks.client

Send and/or recieve a (web)socket client memo result.

returns nil,error if something went wrong or returns result if 
something went right.

]]
M.tasks_functions.client=function(tasks,memo,timeout)

	if type(memo) == "string" then memo={data=memo} end
	memo=memo or {}
	memo.task=memo.task or "client"
	
	tasks:send(memo,timeout)
--	log("memo",memo.task,memo.id)
	tasks:receive(memo,timeout)
--	log("memo",memo.task,memo.id,"done")

	if memo.error then return nil,memo.error end
	if memo.result.error then return nil,memo.result.error end
	return memo.result
end


