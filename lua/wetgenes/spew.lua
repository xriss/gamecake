--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

--[[#lua.wetgenes.spew

	local spew=require("wetgenes.spew").connect(oven.tasks)

You must pass in a previously created tasks object that you will be 
calling update on regually so that our coroutines can run. eg oven.tasks

Connect and talk to the wetgenes spew server using wetgenes.tasks 
task for meta state and a thread for lowlevel sockets.

You push messages in and can pull messages out or, setup a hook 
function to auto pull and process messages in our coroutine as they are 
received.

]]

-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.id=0 -- unique incremental id for new tasks etc

--[[#lua.wetgenes.spew.connect

	local spew=require("wetgenes.spew").connect(oven.tasks)
	local spew=require("wetgenes.spew").connect(oven.tasks,host)
	local spew=require("wetgenes.spew").connect(oven.tasks,host,port)

You must pass in an active tasks object.

Create task and thread connection to the host. host and port 
default to wetgenes.com and 5223 so can be left blank unless you want 
to connect to a local host for debugging.

There are now 3 ways to handle msgs

	spew.push(msg)

To send a msg

	local msg,s=spew.pull()

To receive a message, will return nil if no messages available. The 
second return value is the input string packet for this decoded 
message.

	spew.hook=function(msg,s) print(msg) end

Set a hook functions to auto pull all available messages durring tasks 
update. Note that if you do this spew.pull() will no longer work as all 
messages are auto pulled and sent to this hook function.

Note when receiving a msg you must not alter or cache the table you 
are given as it is internal data and is reused. You must duplicate it 
if you want to keep it arround.

]]

M.connect=function(tasks,host,port)

	M.id=M.id+1

	local spew={}
	
	spew.id=M.id
	
	spew.push_stack={}
	spew.pull_data=""
	spew.pull_state={}
	
	local url_decode=function(str)
		return string.gsub(str, "%%(%x%x)", function(hex)
			return string.char(tonumber(hex, 16))
		end)
	end

	local url_encode=function(str)
		return string.gsub(str, "([&=%%])", function(c)
			return string.format("%%%02x", string.byte(c))
		end)
	end

	local strmsg=function(it)
		local s="&"
		for n,v in pairs(it) do
			s=s..url_encode(n).."="..url_encode(v).."&"
		end
		return s.."\0"
	end

	spew.push=function(m)
		spew.push_stack[#spew.push_stack+1]=strmsg(m)
	end

	spew.pull=function()
		local a=string.find(spew.pull_data,"\0")
		if a then
			local cs=string.sub(spew.pull_data,1,a) -- command string
			spew.pull_data=string.sub(spew.pull_data,a+1) -- remainder
			
			for ps in string.gmatch(cs,"([^&]+)") do -- all the bits between &
				local eq=string.find(ps,"=") -- which must have an =
				if eq then
					local sa=url_decode(ps:sub(1,eq-1))
					local sb=url_decode(ps:sub(eq+1))
					spew.pull_state[sa]=sb
				end
			end

			return spew.pull_state,cs -- return the current state and the string that triggered it
		end
	end

	spew.thread=tasks:add_thread({
		count=1,
		id="spew_thread_"..spew.id,
		globals={ client_host=(server or "wetgenes.com") , client_port=(port or 5223) },
		code=tasks.client_code,
	})

	spew.task=tasks:add_task({
		code=function(linda,task_id,task_idx)

			while true do
			
				if spew.push_stack[1] then -- send and recv
					while spew.push_stack[1] do
						local r=tasks:client({
							data=table.remove(spew.push_stack,1),
							task="spew_thread_"..spew.id,
						})
						if r.data then spew.pull_data=spew.pull_data..r.data end
					end
				else -- just recv
					local r=tasks:client({
						task="spew_thread_"..spew.id,
					})
					if r.data then spew.pull_data=spew.pull_data..r.data end
				end
				
				if spew.hook then -- we will auto pull
					for m,s in spew.pull do
						spew.hook(spew,m,s)
					end
				end
				
				coroutine.yield()
			end

		end,
	})

	return spew
end


--[[#lua.wetgenes.spew.test


test

]]

M.test=function()

	local tasks=require("wetgenes.tasks").create()
	local spew=require("wetgenes.spew").connect(tasks)

	spew.hook=function(spew,msg,str) -- will be called with new data from within the coroutine during tasks:update
		print(str)
--		dump(msg)
	end

	spew.push({cmd="note",note="playing",arg1="unzone",arg2="",arg3="",arg4=""})
	spew.push({cmd="login",name="tester"})
	spew.push({cmd="join",room="public.tv"})

	while true do
		tasks:update()
	end

end
