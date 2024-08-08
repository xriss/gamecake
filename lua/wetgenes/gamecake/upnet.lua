--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[

Manage basic network connection and transfer of input states between 
clients.

]]

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local baseport=2342
local basepack=2342

local msgp=require("wetgenes.tasks_msgp")

M.bake=function(oven,upnet)

	upnet=upnet or {}
	
	upnet.taskid=upnet.taskid or "msgp"

	upnet.setup=function()

		-- create msgp handling thread
		upnet.thread=oven.tasks:add_global_thread({
			count=1,
			id=upnet.taskid,
			code=msgp.msgp_code,
		})

		-- and tell it to start listening
		upnet.host=oven.tasks:do_memo({
			task=upnet.taskid,
			cmd="host",
			baseport=baseport,
			basepack=basepack,
		})
		
		upnet.reset()

	end

	-- reset all connections
	upnet.reset=function()
	
		upnet.clients={}

	end
	
	-- try to make a new connection
	upnet.join=function(addr)

		tasks:do_memo({
			task=upnet.taskid,
			cmd="join",
			addr=addr,
		})

	end

	upnet.domsg=function(m)

		dump(m)

	end
	
	-- manage msgs and pulse controller state
	upnet.update=function()

		-- poll for new data
		local ret=tasks:do_memo({
			task=upnet.taskid,
			cmd="poll",
		})
		for i,msg in ipairs( ret.msgs or {} ) do
			upnet.domsg(m)
		end

	end
	
	return upnet
end
