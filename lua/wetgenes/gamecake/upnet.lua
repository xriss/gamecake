--
-- (C) 2024 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local json_pack=require("wetgenes.json_pack")

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
	
	upnet.task_id=upnet.task_id or "msgp"
	upnet.task_id_msg=upnet.task_id..":msg"

	local socket = require("socket")
	local now=function() return socket.gettime() end -- time now with sub second acuracy
	local nowticks=function() return (now()-upnet.ticks.epoch)/upnet.ticks.length end -- time now with sub second acuracy
	
	upnet.now=now
	upnet.nowticks=nowticks

	local print=function(...)
		local idx=(upnet.us or 0)
		local tabs=string.rep("\t\t\t\t\t\t\t\t\t\t\t",idx-1)
		
		print( idx..tabs , ... )
	end
	upnet.print=print
	
	-- reset all connections
	upnet.reset=function()
	
		-- we will control the ups
		oven.ups.auto_advance=false

	
		upnet.ticks={}

		-- seconds ( floats )
		upnet.ticks.length=1/16	-- time in seconds for each tick
		upnet.ticks.epoch=nil	-- start time of ticks in seconds

		-- ticks ( integers )
		upnet.ticks.input=0		-- the tick we have all inputs for
		upnet.ticks.update=0	-- the tick you have updated
		upnet.ticks.now=0		-- the tick we have our input for
		upnet.ticks.draw=0		-- the tick you have drawn 
		
		-- tween from 0 to 1 with 1 being the latest frame used when drawing so 
		-- this+upnet.ticks.draw-1 would be the fractional tick we are drawing
		upnet.ticks.draw_tween=1

		-- we sync now to time and calculate input tick as data arrives
		-- you should set update and draw times when you update and draw
		-- if things are laging then we may adjust the epoch to "skip" frames

		upnet.history={} -- 1st index is for ticks.now , 2nd is .now-1, etc

		upnet.hooks={}
		
		upnet.us=nil -- we are this client idx

		upnet.host_inc=0 -- host incs per client
		upnet.clients={} -- clients by idx ( managed by host ) these are the live conected clients and array may have holes
		upnet.clients_addr={} -- clients by addr ( local name )
		upnet.clients_id={} -- clients by id ( unique name ) as reported by client so could be a lie

		upnet.clients_idx={} -- clients order provided by host, remembered from welcome msg

	end
	upnet.reset() -- make sure we are always tables

	upnet.manifest_client=function(m)
	
		local client=upnet.clients_addr[m.addr]
		
		if client then return client end
		
		-- create
		client={}
		client.addr=m.addr -- this may be different per client
		client.ip4=m.ip4 or m.handshake.ip4
		client.ip6=m.ip6 or m.handshake.ip6
		client.port=m.port or m.handshake.port
		client.name=m.name or m.handshake.name
		
		-- a unique id which is [ip6]:port or ip4:port:name
		if client.ip6 then
			client.id="["..client.ip6.."]:"..client.port
		else
			client.id=client.ip4..":"..client.port..":"..client.name
		end
		
		-- remember by
		upnet.clients_addr[client.addr]=client
		upnet.clients_id[client.id]=client

		-- auto funcs
		setmetatable(client,upnet.client)
		
		-- find idx maybe
		for i,c in ipairs(upnet.clients_idx) do
			if c.id==client.id then
				client.idx=i
				upnet.clients[i]=client
			end
		end
		
		client.ack=0

		return client
	end

	-- send basic info from the host on client join
	upnet.client={}
	upnet.client.__index=upnet.client

	upnet.client.send=function(client,msg,cmd)
	
		oven.tasks:send({
			task=upnet.task_id,
			cmd=cmd or "send",
			addr=client.addr,
			data=json_pack.into_data(msg),
		})

	end

	upnet.client.recv={}
	upnet.client.recv.all=function(client,msg)
	
		if msg.upnet then
			local f=upnet.client.recv[msg.upnet]
			if f then
				f(client,msg)
			end
		end
		
		for n,f in pairs(upnet.hooks) do
			if msg[n] then -- if this key is set then the hook wants the msg
				f(client,msg)
			end
		end
		
	end

	upnet.client.welcome_send=function(client)
		local msg={ upnet="welcome" }
		
		msg.clients={}
		for i,c in pairs(upnet.clients) do -- send all clients idx
			local v={}
			v.id=c.id
			v.idx=c.idx
			v.ip4=c.ip4
			v.ip6=c.ip6
			v.port=c.port
			v.name=c.name
			msg.clients[#msg.clients+1]=v
		end
		
		msg.ticks=upnet.ticks.input
		
		client:send(msg)
	
	end
	upnet.client.recv.welcome=function(client,msg)
	
		upnet.clients_idx=msg.clients
		for i,c in ipairs(upnet.clients_idx) do -- assign clients idx
			local v=upnet.clients_id[c.id]
			if v then
				v.idx=c.idx
				upnet.clients[i]=v
			else -- need to join this client...
			end
		end

		for i,c in pairs(upnet.clients) do
			if c.us and c.idx then
				upnet.us=c.idx
			end
		end

		upnet.ticks.now=msg.ticks
		upnet.ticks.input=upnet.ticks.now
		upnet.ticks.update=upnet.ticks.now
		upnet.ticks.draw=upnet.ticks.now
		upnet.ticks.epoch=now()-(upnet.ticks.now*upnet.ticks.length)

--[[
		-- send current ups to network
		for _,client in pairs(upnet.clients) do
			if not client.us then
				client:pulse_send()
			end
		end
]]

print("WELCOME",client.idx)
dump(upnet.clients)
	
	end

	upnet.client.pulse_send=function(client)
	
		local msg={ upnet="pulse" }
		
		msg.ticks=upnet.ticks.now
		msg.ack=upnet.ticks.update -- acknowledged up to here
		
		if upnet.us then
			local hs={}
			for i=1,#upnet.history do
				if upnet.ticks.now+1-i <= client.ack then break end
				local h={}
				hs[#hs+1]=h
				h[upnet.us]=upnet.history[i][upnet.us] or {} -- might miss early frames
			end
			msg.history=hs
		end

		client:send(msg,"pulse")
	
	end
	upnet.client.recv.pulse=function(client,msg)
	
--		print("pulse recv",msg.ticks,#msg.history,upnet.ticks.input,upnet.ticks.now)

		client.ack=msg.ack -- this update has been acknowledged

		local cidx=1+upnet.ticks.now-msg.ticks
		for midx=1,#msg.history do
			local c=upnet.history[cidx]
			local m=msg.history[midx]
			if c and m then -- accept inputs
				c[client.idx] = c[client.idx] or m[client.idx]
			end
			cidx=cidx+1
		end
	end

	-- reduce old history we no longer need
	upnet.shrink_history=function()

--dump(upnet.ticks.now,upnet.history)

		local cidx=1+upnet.ticks.now-upnet.ticks.update -- discard used input
		
		while #upnet.history>cidx+16 do upnet.history[#upnet.history]=nil end -- trim

	end

	upnet.setup=function()

		local args=oven.opts.args

		-- create msgp handling thread
		upnet.thread=oven.tasks:add_global_thread({
			count=1,
			id=upnet.task_id,
			code=msgp.msgp_code,
		})
		
		upnet.reset()

		-- everyone must enable network with a host
		if args.host then
		
			if tonumber( args.host ) then baseport=tonumber( args.host ) end
		
			-- and tell it to start listening
			local host_ret=oven.tasks:do_memo({
				task=upnet.task_id,
				cmd="host",
				baseport=baseport,
				basepack=basepack,
			})
			-- the client of this host
			local client=upnet.manifest_client(host_ret)
			client.us=true -- remember that this is us

			-- clients join the host
			if args.join then
			
				upnet.join( args.join )
				upnet.mode="join"

			else -- and one host just waits for clients to join

				upnet.mode="host"
				-- we are client 1
				upnet.host_inc=upnet.host_inc+1
				client.idx=upnet.host_inc
				upnet.clients[client.idx]=client
				
				upnet.us=client.idx
				
				upnet.ticks.epoch=now()-(upnet.ticks.now*upnet.ticks.length)

			end

		end
		
dump(upnet.clients)

	end
	
	-- try to make a new connection
	upnet.join=function(addr)
print("joining",addr)
		local ret=oven.tasks:do_memo({
			task=upnet.task_id,
			cmd="join",
			addr=addr,
		})

	end

	upnet.domsg=function(m)

		local client=upnet.clients_addr[m.addr]	-- may be nil
	
		if m.why=="connect" then
		
			client=upnet.manifest_client(m) -- create client
				
			if upnet.mode=="host" then -- assign idx
				
				-- next client
				upnet.host_inc=upnet.host_inc+1
				client.idx=upnet.host_inc
				upnet.clients[client.idx]=client
				
				client:welcome_send()
			
dump(upnet.clients)
			end
		

		elseif m.why=="data" or m.why=="pulse" then
		
			local msg=json_pack.from_data(m.data) -- unpack binary
			client.recv.all(client,msg)

		else
				
			dump(m)

		end
		
	end

	-- get an ups array for the given tick
	-- each connected client.idx will have an up available for that idx
	upnet.get_ups=function(tick)
		tick=tick or upnet.ticks.now
		local ti=1+upnet.ticks.now-tick
		
		local ups={}
		for idx,_ in pairs(upnet.clients) do
			local up=oven.ups.create()
			ups[idx]=up
			if idx==upnet.us and tick>upnet.ticks.now then -- use live input
				up:load( oven.ups.manifest(1):save() ) -- fill
				up:update(upnet.ticks.length)
			else
				for ui=ti,#upnet.history do -- find best state we have
					local h=upnet.history[ui]
					if h and h[idx] then
						up:load(h[idx]) -- fill
						break -- and done
					end
				end
			end
		end
		
		ups[0]=oven.ups.empty
		
--		print(upnet.us,tick,upnet.ticks.now,#upnet.history,ti,ups[1] and ups[1].all.lx,ups[2] and ups[2].all.lx)
--		dump(upnet.history)

		return ups
	end
	

	-- update the tick time of when we have all inputs
	upnet.update_ticks_input=function()

		local ti=1+upnet.ticks.now-upnet.ticks.input	-- we have input for here
		local h=upnet.history[ti-1]
		if not h then return end

		for _,v in pairs(upnet.clients) do -- must have data for all clients
			if not h[v.idx] then return end
		end
		
		upnet.ticks.input=upnet.ticks.input+1
		return true
	end

	-- tick one tick forwards
	upnet.next_tick=function() 
	
		local up=oven.ups.manifest(1) -- we are this input

		upnet.ticks.now=upnet.ticks.now+1
		up:update(upnet.ticks.length)
		-- remember current up
		table.insert( upnet.history , 1 , { [upnet.us]=up:save() } ) -- remember new tick
--print("history",upnet.us,#upnet.history)
		
		-- send current ups to network
		for _,client in pairs(upnet.clients) do
			if not client.us then
				client:pulse_send()
			end
		end

		upnet.shrink_history()
		
	end
	
	-- manage msgs and pulse controller state
	upnet.update=function()

		repeat -- check msgs
		
			local _,memo= oven.tasks.linda:receive( 0 , upnet.task_id_msg ) -- wait for any memos coming into this thread
			
			if memo then
				upnet.domsg(memo)
			end
		
		until not memo
		
		repeat until not upnet.update_ticks_input() -- update ticks.input

		if upnet.ticks.epoch and upnet.us then -- we are ticking
			local t=((now()-upnet.ticks.epoch)/upnet.ticks.length) -- floor this to reduce latency but get janky predictions
			while t>upnet.ticks.now do -- without floor we have 1 tick of controller latency
				upnet.next_tick()
			end
		end
		
	end
	
	return upnet
end
