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


-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.tasks_functions={}
M.tasks_metatable={__index=M.tasks_functions}

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
	if it.type then
		tasks[it.type][it.id]=nil
	end
	tasks.ids[it.id]=nil
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
there is no point in createing multiple tasks as they all run on the 
calling thread anyway. the function is inside a coroutine and must 
yield regulary this yield will then continue on the next update.

	id

A unique id string to be used by lindas when sending messages into this 
task. The function is expected to sit in an infinite loop testing 
this linda socket and then yielding if there is nothing to do.

	code

A lua function to run inside a coroutine, this function will recieve 
tasks.linda and the task.id for comunication and an index of 1 so it 
has the same calling signature as a thread.

]]
M.tasks_functions.add_task=function(tasks,task)
	task=task or {}
	task.type="task"
	tasks:add_id(task)
	
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

	return tasks
end


M.test=function()

for n,v in pairs(lanes) do print(n,v) end

	print("testing tasks")
	
	local tasks=M.create()
	
	local thread=tasks:add_thread({
		count=10,
		globals={},
		id="thread",	-- send memos here
		code=function(linda,task_name,task_idx)

			print("starting linda")

			while true do

				local _,memo= linda:receive( nil , task_name ) -- wait for any memos coming into this thread
				
				print("memo task "..task_name..":"..task_idx)
				for n,v in pairs(memo) do print(n,v) end
			
			end
	
			print("finishing linda")
		end,
	})
	
	for i=1,10 do
		tasks.linda:send("thread",{id=i,poop="ok",name="this"..i})
	end
	
end

M.test()

