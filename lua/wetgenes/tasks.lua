--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

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

-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.tasks_functions={}
M.tasks_metatable={__index=M.tasks_functions}

M.colinda_functions={}
M.colinda_metatable={__index=M.colinda_functions}

--[[#lua.wetgenes.tasks.add_id

	tasks:add_id(it)

Internal function to manage creation of all objects with unique ids.

]]
M.tasks_functions.add_id=function(tasks,it)
	it=it or {}
	it.id=it.id or #tasks.ids+1 -- auto generate id
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
		it.id=nil
	end
	return it
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
	tasks:del_id(thread)	
	return thread
end

--[[#lua.wetgenes.tasks.add_task

	local thread=tasks:add_task({
		id="test",
		code=function(linda,task_id,task_idx)
			while true do
				local _,memo= linda:receive( 0 , task_id )
				if memo then
					...
				else
					coroutine.yield()
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
tasks.linda and the task.id for comunication and an index so we know 
which of the count tasks we are (mostly for debugging)

]]
M.tasks_functions.add_task=function(tasks,task)
	task=task or {}
	task.type="task"
	tasks:add_id(task)
	
	task.count=task.count or 1
	task.errors={}
	task.handles={}
	for idx=1,task.count do
		task.handles[idx]=coroutine.create(task.code)
		local ok , err = coroutine.resume( task.handles[idx] , task.colinda , task.id , idx ) -- first call passing in args
		if not ok then task.errors[idx]=err end
	end

	return task
end

--[[#lua.wetgenes.tasks.run_task

	tasks:run_task(task)
	
Resume all the coroutines in this task.

]]
M.tasks_functions.run_task=function(tasks,task)

	for idx=1,task.count do
		if coroutine.status( task.handles[idx] )=="suspended" then
			local ok , err = coroutine.resume( task.handles[idx] )
			if not ok then task.errors[idx]=err end
		end
	end

	return task
end

--[[#lua.wetgenes.tasks.del_task

	tasks:del_thread(task)
	
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

	result = tasks:receive(memo,timeout)
	result = tasks:receive(memo)
	
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

--[[#lua.wetgenes.tasks.delete

	tasks:delete()
	
Force stop all threads and delete all data.

Failure to call this will allow any created threads to continue to run 
until program termination.

]]
M.tasks_functions.delete=function(tasks)
	for idx,thread in pairs(tasks.thread) do
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
	
	tasks.linda=lanes.linda()
	tasks.colinda=M.create_colinda(tasks.linda)

	return tasks
end


--[[#lua.wetgenes.tasks.http_code

A basic function to handle http memos.

]]
M.http_code=function(linda,task_id,task_idx)

	local http = lanes.require("socket.http")
	local ltn12 = lanes.require("ltn12")

	local function request(memo)

		local out = {}
		local req = {}

		req.sink = ltn12.sink.table(out)
		if memo.body then
			req.source = ltn12.source.string(memo.body)
		end

		req.url=memo.url
		req.method=memo.method or "GET"
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
			local ok,ret=pcall(function() return request(memo) end) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			linda:send( nil , memo.id , ret ) -- always respond to each memo with something
		end

	end

end

--[[#lua.wetgenes.tasks.http_memo

Create send and return a http memo result.

]]
M.tasks_functions.http=function(tasks,memo)

	if type(memo) == "string" then memo={url=memo} end
	memo.task=memo.task or "http"
	
	tasks:receive(memo)

	if memo.error then return nil,memo.error end
	return memo.result
end



--[[#lua.wetgenes.tasks.test

test

]]
M.test=function()

	print("testing tasks")

	local tasks=M.create()
	
	tasks:add_thread({
		count=8,
		id="http",
		code=M.http_code,
	})

	local task=tasks:add_task({
		count=4,
		code=function(linda,task_id,task_idx)

			local ret = tasks:http("https://xixs.com/index.html")
			print("####",ret)
			for n,v in pairs(ret) do print(n,#tostring(v)>128 and "#"..#v or v) end

		end,
	})
	
	while true do
		tasks:update()
--		print(task.errors[1])
	end

	
end
-- M.test()

