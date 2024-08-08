#!/bin/env gamecake

local global=_G ; pcall(function() global=require("global") end)

require("apps").default_paths() -- default search paths so things can easily be found


local msgp=require("wetgenes.tasks_msgp")

--print(tasks_msgp)

local test_server=function(tasks)

	tasks=tasks or require("wetgenes.tasks").create()

	local log,dump=require("wetgenes.logs"):export("log","dump")

	local baseport=2342
	local basepack=2342


	local lanes=require("lanes")

	local hosts={}
	
	for i=1,2 do
	
		local thread=tasks:add_global_thread({
			count=1,
			id="msgp"..i,
			code=msgp.msgp_code,
		})
		hosts[i]=tasks:do_memo({
			task="msgp"..i,
			cmd="host",
			baseport=baseport,
			basepack=basepack,
		})
		hosts[i].thread=thread
		hosts[i].task="msgp"..i
	--	dump(ret1)

	end

	tasks:do_memo({
		task="msgp2",
		cmd="join",
		addr=hosts[1].addr,
	})
--	dump(ret3)

	local dumpit=0
	local data=""

	while true do
			
		for i=1,2 do
			local task="msgp"..i
			local host=hosts[i]
			local other=hosts[1+(i%2)]
			
			data=string.sub(data..i..data,-0x100000)

			dumpit=dumpit-1
			if dumpit<=0 then
				dumpit=101
				tasks:do_memo({
					task=host.task,
					cmd="send",
					addr=other.addr,
					data=data:sub(1,math.random(1024,1024*1024)), -- random size from 1k to 1meg
				})
			else
				tasks:do_memo({
					task=host.task,
					cmd="send",
					addr=other.addr,
					data=string.sub(data,-2048)
				})
			end

			tasks:do_memo({
				task=host.task,
				cmd="pulse",
				addr=other.addr,
				data=string.sub(data,-1024)
			})

			-- send packet
			-- poll for new data
			local ret=tasks:do_memo({
				task=task,
				cmd="poll",
			})
			for i,msg in ipairs( ret.msgs or {} ) do
				msg.from=task
--				dump(msg)
			end
			lanes.sleep(0.050) -- take a little nap

		end

	end

end


test_server(tasks)
