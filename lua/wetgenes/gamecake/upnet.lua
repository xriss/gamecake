--
-- (C) 2024 Kriss@XIXs.com
--


--[[

Manage basic network connection and transfer of input states between
clients.

]]
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


------------------------------------------------------------------------
do -- stop these locals from poisoning task functions
------------------------------------------------------------------------

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local Ox=function(n) return string.format("%012x",n or 0) end

local log,dump,dlog=require("wetgenes.logs"):export("log","dump","dlog")

local wgups=require("wetgenes.gamecake.ups")
local wstr=require("wetgenes.string")

local json_pack=require("wetgenes.json_pack")



local baseport=2342
local basepack=2342

local msgp=require("wetgenes.tasks_msgp")


M.bake=function(oven,upnet)

	upnet=upnet or {}

	-- create msgp handling thread if it does not exist
	oven.tasks:add_global_thread({
		count=1,
		id="msgp",
		code=msgp.msgp_code,
	})

	-- create upnet handling thread if it does not exist
	oven.tasks:add_global_thread({
		count=1,
		id="upnet",
		code=M.upnet_code,
	})
	
	oven.ups.subscribe("upnet/ups") -- request all ups to be sent here
	
	upnet.setup=function()
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="setup",
			args={}, -- oven.opts.args,
		})
	end
	
	upnet.clean=function()
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="clean",
		})
	end

	upnet.update=function()
		-- nothing to do, is 
	end

	upnet.catchup=function()
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="catchup",
		})
	end
	
	upnet.subscribe=function(subid)
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="subscribe",
			subid=subid,
		})
	end

	upnet.unsubscribe=function(subid)
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="unsubscribe",
			subid=subid,
		})
	end
	
	upnet.broadcast=function(data)
		oven.tasks:do_memo({
			task="upnet",
			id=false,
			cmd="broadcast",
			data=data,
		})
	end

	upnet.get_ticks=function()
		local r=oven.tasks:do_memo({
			task="upnet",
			cmd="get_ticks",
		})
		return r.ticks
	end

	upnet.get_ups=function(tick)
		local r=oven.tasks:do_memo({
			task="upnet",
			cmd="get_ups",
			tick=tick,
		})
		local ups={}
		for i,v in pairs(r.ups) do
			ups[1]=wgups.up.create()
			ups[1]:load(v)
		end
		return ups
	end

	upnet.set_hash=function()
	end
	
	return upnet
end


M.create=function(upnet)

	upnet=upnet or {}

	upnet.dmode=function(mode)
		local us=(upnet.us or 0)
		return us..mode..("\t\t\t\t\t\t"):rep(us-1)
	end

	local socket = require("socket")
	local now=function() return socket.gettime() end -- time now with sub second acuracy
	local nowticks=function() -- time now with sub second acuracy
		if not upnet.ticks.epoch then return -1 end
		return (now()-upnet.ticks.epoch)/upnet.ticks.length
	end

	upnet.now=now
	upnet.nowticks=nowticks

	local print=function(...)
		local idx=(upnet.us or 0)
		local tabs=string.rep("\t\t\t\t\t\t\t\t\t\t\t",idx-1)

		print(idx..tabs , ... )
	end
	upnet.print=print

	-- reset all connections
	upnet.reset=function()

		upnet.ticks={}

		-- seconds ( floats )
		upnet.ticks.time=0		-- now time in ticks (time-epoch)/length (probably now + a fraction)
		upnet.ticks.lag=1		-- number of ticks to lag input (0 will create glitch frames)
		upnet.ticks.length=1/16	-- time in seconds for each tick
		upnet.ticks.epoch=nil	-- start time of ticks in seconds
		upnet.ticks.pause=nil	-- if set, adjust epoch so ticks do not advance

		-- ticks ( integers )
		upnet.ticks.agreed=1	-- the tick all clients have state agreed as true
		upnet.ticks.input=1		-- the tick we have all inputs for
		upnet.ticks.now=1		-- the tick we have our input for
		upnet.ticks.base=1		-- the tick at the base of our arrays

		upnet.need_sync=false	-- client needs to sync data when this is set
		upnet.need_hash=false	-- if set then client must provide a hash for each frame

		-- we sync now to time and calculate input tick as data arrives
		-- you should set update and draw times when you update and draw
		-- and must advance base time as a frame os fully synced and no longer needed
		-- if things are laging then we may adjust the epoch to "skip" frames

		upnet.inputs={} -- 1st index is for ticks.base 2nd is .base+1 etc
		upnet.hashs={} -- 1st index is for ticks.base 2nd is .base+1 etc

		upnet.hooks={}

		upnet.us=nil -- we are this client idx

		upnet.host_inc=0 -- host incs per client
		upnet.clients={} -- clients by idx ( managed by host ) these are the live conected clients and array may have holes
		upnet.clients_addr={} -- clients by addr ( local name )
		upnet.clients_id={} -- clients by id ( unique name ) as reported by client so could be a lie

		upnet.clients_idx={} -- clients order provided by host, remembered from welcome msg

		upnet.upcache=wgups.up.create() -- local cached inputs

		upnet.subscribed={}
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
		client.join_tick=math.huge

		return client
	end

	-- send basic info from the host on client join
	upnet.client={}
	upnet.client.__index=upnet.client

	upnet.client.send=function(client,msg,cmd)

		upnet.tasks:send({
			task="msgp",
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

	upnet.client.send_done_welcome=function(client)
		local msg={ upnet="done_welcome" }

		local fh=upnet.hooks.send_done_welcome
		if fh then fh(client,msg) end -- user hooks can modify msg
		client:send(msg)
	end

	upnet.client.recv.done_welcome=function(client,msg)
		upnet.ticks.pause=nil
	end

	upnet.client.send_welcome=function(client)

		upnet.ticks.pause="host"

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

		msg.ticks=upnet.ticks.agreed

		client.join_tick=msg.ticks

		local fh=upnet.hooks.send_done
		if fh then fh(client,msg) end -- user hooks can modify msg
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
		upnet.ticks.base=upnet.ticks.now
		upnet.ticks.agreed=upnet.ticks.now
		upnet.ticks.input=upnet.ticks.now
		upnet.ticks.epoch=now()-(upnet.ticks.now*upnet.ticks.length)

		upnet.hashs={} -- reset hashes
		upnet.inputs={} -- reset inputs

		client.join_tick=msg.ticks


print("WELCOME",client.idx)
--dump(upnet.clients)

		upnet.ticks.pause=nil	-- pause over
		client:send_done_welcome()

	end

	upnet.client.send_pulse=function(client)

		local msg={ upnet="pulse" }

		msg.ticks_now=upnet.ticks.now -- our current time
		msg.ticks_base=upnet.ticks.base -- our current base
		msg.ticks_agreed=upnet.ticks.agreed -- we acknowledged inputs up to here

		if upnet.us then
			msg.inputs={}
			for i=1,#upnet.inputs do
				local hi=upnet.inputs[i] or {}
				msg.inputs[i]=hi[upnet.us] or {} -- might miss early frames
			end
			msg.hashs={}
			for i=1,#upnet.hashs do -- keep resending all our hashes untill we sync
				local hi=upnet.hashs[i] or {}
				msg.hashs[i]=hi[upnet.us]
			end
		end

		local fh=upnet.hooks.send_pulse
		if fh then fh(client,msg) end -- user hooks can modify msg
		client:send(msg,"pulse")

	end
	upnet.client.recv.pulse=function(client,msg)

--		print("pulse recv",msg.ticks,#msg.inputs,upnet.ticks.input,upnet.ticks.now)

		client.ticks_now=msg.ticks_now
		client.ticks_base=msg.ticks_base
		client.ticks_agreed=msg.ticks_agreed -- this update has been acknowledged

--		local cidx=1+upnet.ticks.now-msg.tick
		local fix=msg.ticks_base-upnet.ticks.base
		for idx=1,#msg.inputs do
			local c=upnet.inputs[idx+fix]
			local m=msg.inputs[idx]
			if c and m then -- accept new inputs but never change
				c[client.idx] = c[client.idx] or m
			end
		end

		local fix=msg.ticks_base-upnet.ticks.base -- adjust their base to our base
		for idx=1,#msg.hashs do
			local c=upnet.hashs[idx+fix]
			local m=msg.hashs[idx]
			if c and m then -- accept and overide old data with any new hashes
				if c[client.idx] and c[client.idx]~=m then
--dlog(upnet.dmode("newhash"),idx+msg.ticks_base-1,Ox(c[client.idx]),Ox(m))
				end
				c[client.idx] = m or c[client.idx]
			end
		end

	end


	upnet.clean=function()
		upnet.setup_done=false
		-- should disconnect clients etc?
	end
	
	upnet.setup=function()
	
		upnet.setup_done=true

		local args=upnet.args

		upnet.reset()


		if tonumber( args.host ) then baseport=tonumber( args.host ) end

		-- and tell it to start listening
		local host_ret=upnet.do_memo({
			task="msgp",
			cmd="host",
			baseport=baseport,
			basepack=basepack,
		})
		-- the client of this host
		local client=upnet.manifest_client(host_ret)
		client.us=true -- remember that this is us

		-- clients join the host
		if args.join then

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

--dump(upnet.clients)

	end

	-- try to make a new connection
	upnet.join=function(addr)

		upnet.ticks.pause="join"

print("joining",addr)
		local ret=upnet.do_memo({
			task="msgp",
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

				client:send_welcome()

--dump(upnet.clients)
			end


		elseif m.why=="data" or m.why=="pulse" then

			local msg=json_pack.from_data(m.data) -- unpack binary
			client.recv.all(client,msg)

		else

--			dump(m)

		end

	end

	-- get an ups array for the given tick
	-- each connected client.idx will have an up available for that idx
	upnet.get_ups=function(tick)
		tick=tick or upnet.ticks.now
--print("getups",tick,upnet.ticks.now)
		local ti=tick+1-upnet.ticks.base
--print("readup",tick,ti)

		local ups={}
		for ci,_ in pairs(upnet.clients) do
			local up=wgups.up.create()
			ups[ci]=up
			for ui=ti,1,-1 do -- find best state we have
				local h=upnet.inputs[ui]
				if h and h[ci] then
					up:load(h[ci]) -- fill with data
-- this will have set/clr flags locked on into future prediction frames so we should update to clear them?
					if ui<ti then -- we had to look backwards in timw
--print(tick,ci,ui,ti,#upnet.inputs,upnet.inputs[#upnet.inputs][1],upnet.inputs[#upnet.inputs][2])
						for i=ui+1,ti do
--							print("ups future",i)
							 -- predict next frame
							up:update(upnet.ticks.length)
						end
					end
					break -- and done
				end
			end
		end

		ups[0]=wgups.empty

--		print(upnet.us,tick,upnet.ticks.now,#upnet.inputs,ti,ups[1] and ups[1].all.lx,ups[2] and ups[2].all.lx)
--		dump(upnet.inputs)

		return ups
	end


	-- update the tick time of when we have all inputs available
	upnet.update_ticks_input=function()

--print("nowup", upnet.ticks.now , upnet.ticks.input )
		-- wait until it is time to sample new input
--		if not ( upnet.ticks.now>=upnet.ticks.input+2 ) then return end -- input should always be one frame behind now

		local ti=1+upnet.ticks.input-upnet.ticks.base	-- we have input for here
		local h=upnet.inputs[ti+1] -- and we want to check for all input here
		if not h then return end

--dump(upnet.inputs)
		for _,v in pairs(upnet.clients) do -- must have data for all clients
			if not h[v.idx] then return end
		end
--print("clients OK",#upnet.clients)
		upnet.ticks.input=upnet.ticks.input+1 -- we have input for this frame now
		return true
	end

	-- update the tick time of when we have matching checksums
	upnet.update_ticks_agreed=function()

		local ti=1+upnet.ticks.agreed-upnet.ticks.base	--  agreed tick
		if not need_hash then
			if upnet.ticks.agreed<upnet.ticks.now-1 then
				upnet.hashs[ti+1] = upnet.hashs[ti+1] or {}
				upnet.hashs[ti+1][upnet.us]=0
			end
		end
		local hash=upnet.hashs[ti+1] -- next tick after agreed tick
		if not hash then return end

		local h=hash[upnet.us] -- our hash
		if not h then return end
--local hs={} ; for i,v in pairs(hash) do hs[i]=(Ox(v)) end
--dlog(upnet.dmode("hashs"),upnet.ticks.agreed+1,unpack(hs))
--local hs={} ; for i,v in pairs(hash) do hs[i]=(Ox(v)) end
--dlog(upnet.dmode("sync"),upnet.ticks.agreed+1,unpack(hs))
		for _,v in pairs(upnet.clients) do -- all hashes must agree
			if not hash[v.idx] then return end -- no hash yet
			if h ~= hash[v.idx] then -- hash does not match
				upnet.need_sync=upnet.ticks.agreed+1 -- need to trigger a full resync for this frame
local hs={} ; for i,v in pairs(hash) do hs[i]=(Ox(v)) end
dlog(upnet.dmode("sync"),upnet.ticks.agreed+1,unpack(hs))
				return
			end
		end
		upnet.need_sync=false

		upnet.ticks.agreed=upnet.ticks.agreed+1
		return true
	end

	-- get lowest agreed frame across all clients
	upnet.get_client_agreed=function()
		local agreed=upnet.ticks.agreed
		for _,client in pairs(upnet.clients) do -- all clients must agree up to this point as well
			if client.ticks_agreed then
				if client.ticks_agreed < agreed then
					agreed=client.ticks_agreed
				end
			end
		end
		return agreed
	end

	-- move base one tick forwards deleting old data in cached arrays
	-- must be called by user to prevent buildup of our cached values
	-- user should obviously delete own cache at the same time
	upnet.inc_base=function()

		upnet.ticks.base=upnet.ticks.base+1

		-- adjust hashs table to new base
		table.remove(upnet.hashs,1)

		-- adjust inputs table to new base
		table.remove(upnet.inputs,1)

		return true

	end
	upnet.set_hash=function(idx,hash)

--dlog(upnet.dmode("set"),idx,Ox(hash))

		local hidx=1+idx-upnet.ticks.base
		if hidx<1 then return end

		for i=1,hidx do -- make sure array exists
			if not upnet.hashs[i] then upnet.hashs[i]={} end
		end

		upnet.hashs[hidx][upnet.us]=hash -- set hash
	end

	-- tick one tick forwards
	upnet.next_tick=function(f)

		upnet.ticks.now=upnet.ticks.now+1
		-- remember current up

		local iidx=1+upnet.ticks.now-upnet.ticks.base -- next frame
		-- current input is locked in for "next" frame not this frame
		-- that way we can tween local data into the near "future" without glitches
		iidx=iidx+upnet.ticks.lag
		if iidx<1 then return end

		for i=1,iidx do -- make sure full array exists
			if not upnet.inputs[i] then -- must exist
				upnet.inputs[i]={}
			end
			if not upnet.inputs[ i ][upnet.us] then -- fill in with our data
--print("writeup",upnet.ticks.base-1+i,i)
				upnet.inputs[ i ][upnet.us]=upnet.upcache:save() -- remember next ticks inputs
			end
		end
--		upnet.inputs[ iidx ][upnet.us]=upnet.upcache:save() -- remember new tick


-- remove all pulse flags so they will not be set on next save
		upnet.upcache:unpulse()

--print("inputs",upnet.us,#upnet.inputs)

		-- send current ups to network
		for _,client in pairs(upnet.clients) do
			if not client.us then
				client:send_pulse()
			end
		end

	end

	upnet.catchup=function()
		upnet.ticks.epoch=now()-(upnet.ticks.now*upnet.ticks.length)
	end

	-- manage msgs and pulse controller state
	upnet.join_wait=0
	upnet.update=function()
	
		if not upnet.setup_done then return end -- make sure we call setup first

-- keep reading inputs	
--oven.msgs() -- keep handling msgs

		local pause=upnet.ticks.pause -- need pause while connecting etc

		while upnet.get_client_agreed() > (upnet.ticks.base+1) do
			upnet.inc_base()
--			scene:do_pull()
		end

		if upnet.mode=="join" then
			if ( now() - upnet.join_wait ) > 5 then -- wait 5 secs to join before we try again

				local joined=false
				for _,client in pairs(upnet.clients) do
					if not client.us then
						joined=true
					end
				end

				if not joined then

					local args=upnet.args

					upnet.join( args.join )

				end

				upnet.join_wait=now()
			end
		end


--		local up=oven.ups.manifest(1)
--		local up=wgups.empty
		for memo in function() local _,memo= upnet.linda:receive( 0 , "upnet/ups" ) ; return memo end do
--print(wstr.dump(memo))
			if memo.states and memo.states[1] then
				upnet.upcache:merge( memo.states[1] ) -- merge as we update
--print("upsin", (now()-upnet.ticks.epoch)/upnet.ticks.length )
			end
		end

		repeat -- check msgs

			local _,memo= upnet.linda:receive( 0 , "msgp" ) -- wait for any memos coming into this thread
			if type(memo)=="table" then
				upnet.domsg(memo)
			else -- probably a timeout userdata
				memo=nil
			end

		until not memo

		if not pause then

		repeat until not upnet.update_ticks_input() -- update ticks.input
		repeat until not upnet.update_ticks_agreed() -- update ticks.agreed

		end

--assert( upnet.ticks.now - upnet.ticks.agreed <= 16 )

		if upnet.ticks.now - upnet.ticks.agreed > 64 then -- pause/glitch if we get way too far behind
			upnet.ticks.pause="timeout"
			-- todo unpause when we catchup ?
		end

		while upnet.ticks.epoch and upnet.us do -- we are ticking
			upnet.ticks.time=(now()-upnet.ticks.epoch)/upnet.ticks.length
			local f=upnet.ticks.time
			local t=math.floor(f)
			f=f-t -- fraction of a frame
			if t>upnet.ticks.now then
				if pause then
					upnet.ticks.epoch=now()-(upnet.ticks.now*upnet.ticks.length) -- reset epoch so we do not advance
					return
				else
					upnet.next_tick(f)
--					local dbg_hash=upnet.hashs[2+upnet.ticks.agreed-upnet.ticks.base] or {}
--					print("now:"..upnet.ticks.now,"inp:"..upnet.ticks.input,"agr:"..upnet.ticks.agreed,"bse:"..upnet.ticks.base,Ox(dbg_hash[1]),Ox(dbg_hash[2]))
--					dlog(upnet.dmode("tick"),"now:"..upnet.ticks.now,"inp:"..upnet.ticks.input,"agr:"..upnet.ticks.agreed,"bse:"..upnet.ticks.base)
				end
			else
				return
			end
		end

	end

	return upnet
end


------------------------------------------------------------------------
end -- The functions below are free running tasks and should not depend on any locals
------------------------------------------------------------------------

M.upnet_code=function(linda,task_id,task_idx)
	local M -- hide M for thread safety
	local global=require("global") -- lock accidental globals

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local wtasks=require("wetgenes.tasks")
	local wwin=require("wetgenes.win")
	local now=wwin.time -- function to get time now in seconds with ms accuracy, probs
	local wgups=require("wetgenes.gamecake.ups")
	local wgupnet=require("wetgenes.gamecake.upnet")

	-- create main state
	local upnet=wgupnet.create()
	upnet.linda=linda
	upnet.do_memo=function(memo,timeout)
		return wtasks.do_memo(linda,memo,timeout)
	end

	local request=function(memo)
		local ret={}
		
		if memo.cmd=="setup" then

			upnet.args=memo.args or {}
			upnet.setup()

		elseif memo.cmd=="clean" then

			upnet.clean()

		elseif memo.cmd=="catchup" then

			upnet.catchup()

		elseif memo.cmd=="subscribe" then

			upnet.subscribed[memo.subid]={}

		elseif memo.cmd=="unsubscribe" then

			upnet.subscribed[memo.subid]=nil

		elseif memo.cmd=="broadcast" then

			for _,client in pairs(upnet.clients) do
				if not client.us then
					client:send(memo.data)
				end
			end

		elseif memo.cmd=="get_ticks" then
		
			ret.ticks=upnet.ticks

		elseif memo.cmd=="get_ups" then
		
			local ups=upnet.get_ups(memo.tick)
			ret.ups={}
			for i,up in pairs(ups) do
				ret.ups[i]=up:save()
			end

		end

		return ret
	end

	while true do
		upnet.update() -- probably getting called every 1ms ish
		local timeout=0.001 -- first receive will be 1ms or less
		repeat
			local _,memo= linda:receive( timeout , task_id ) -- wait for any memos coming into this thread
			timeout=0 -- repeat receive are instant
			if memo then
				local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
				if not ok then ret={error=ret or true} end -- reformat errors
				if memo.id then -- result requested
					linda:send( nil , memo.id , ret )
				end
			end
		until not memo
	end

end

