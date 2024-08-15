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

	-- reset all connections
	upnet.reset=function()
	
		upnet.hooks={}
		
		upnet.host_idx=0
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

		return client
	end

	-- send basic info from the host on client join
	upnet.client={}
	upnet.client.__index=upnet.client

	upnet.client.send=function(client,msg)
	
		oven.tasks:send({
			task=upnet.task_id,
			cmd="send",
			addr=client.addr,
			data=json_pack.into_data(msg),
		})

	end

	upnet.client.recv=function(client,msg)
	
		if msg.upnet=="welcome" then
		
			client:welcome_recv(msg)

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
		for i,c in ipairs(upnet.clients) do -- send all clients idx
			local v={}
			v.id=c.id
			v.idx=c.idx
			v.ip4=c.ip4
			v.ip6=c.ip6
			v.port=c.port
			v.name=c.name
			msg.clients[i]=v
		end
		
		client:send(msg)
	
	end
	upnet.client.welcome_recv=function(client,msg)
	
		print("WELCOME",client.idx)

		upnet.clients_idx=msg.clients
		for i,c in ipairs(upnet.clients_idx) do -- assign clients idx
			local v=upnet.clients_id[c.id]
			if v then
				v.idx=i
				upnet.clients[i]=v
			else -- need to join this client...
			end
		end


dump(upnet.clients)
	
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

		-- everyone must enable network with a hear
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
				upnet.host_idx=upnet.host_idx+1
				client.idx=upnet.host_idx
				upnet.clients[client.idx]=client

			end

		end
		
dump(upnet.clients)

	end
	
	-- try to make a new connection
	upnet.join=function(addr)

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
				upnet.host_idx=upnet.host_idx+1
				client.idx=upnet.host_idx
				upnet.clients[client.idx]=client
				
				client:welcome_send()
			
dump(upnet.clients)
			end
		

		elseif m.why=="data" or m.why=="pulse" then
		
			local msg=json_pack.from_data(m.data) -- unpack binary
			client:recv(msg)

		else
				
			dump(m)

		end
		
	end
	
	-- manage msgs and pulse controller state
	upnet.update=function()

		repeat
		
			local _,memo= oven.tasks.linda:receive( 0 , upnet.task_id_msg ) -- wait for any memos coming into this thread
			
			if memo then
				upnet.domsg(memo)
			end
		
		until not memo

	end
	
	return upnet
end
