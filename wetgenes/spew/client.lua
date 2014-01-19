--
-- (C) 2013 Kriss@XIXs.com
--

local wsutil=require("wetgenes.spew.util")
local wstr=require("wetgenes.string")

local socket = require("socket")

local _G=_G

local table=table
local ipairs=ipairs
local pairs=pairs
local string=string
local math=math
local os=os
local print=print

local unpack=unpack
local tostring=tostring



-----------------------------------------------------------------------------
--
-- basic client communication with a spew server
-- uses lua sockets to connect
--
-----------------------------------------------------------------------------

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
--module("spew.client.comm")

-----------------------------------------------------------------------------
--
-- create a client conection
--
-----------------------------------------------------------------------------
function M.create(client)

	client=client or {}

-- set host defaults
	client.host=client.host or "swf.wetgenes.com"
	client.port=client.port or 5223

-- callback hooks
	client.hooks={}
	local hooks=client.hooks

-- packet handling
	client.packets=require("wetgenes.spew.packets").create()	
	local packets=client.packets
	
-----------------------------------------------------------------------------
--
-- setup and connect to the spew server
--
-----------------------------------------------------------------------------
	function client.setup()
		client.con=packets.setup( socket.connect(client.host,client.port) , {format="spew"} ) -- connect
		if not client.con then
			print( "failed to connect to "..client.host )
		else
			if client.con.error then print(client.con.error) end
		end
		return client
	end


-----------------------------------------------------------------------------
--
-- clean and disconnect from the spew server
--
-----------------------------------------------------------------------------
	function client.clean()

		if client.con then
			client.packets.clean(client.con.client)
		end
		
	end

-----------------------------------------------------------------------------
--
-- This handles any incoming data waiting but does not block
-- so call it a few times a second, every frame should be good
-- the data will be processed and you will receive callbacks for events
--
-----------------------------------------------------------------------------
	function client.update()

		local tab,_,err=socket.select(client.packets.recvt,nil,0.00001)
-- read from sockets
		for i,v in ipairs(tab or {}) do
		
				local p1, error ,p2 = v:receive("*a")
				local line=p1 or p2
				
				if error=="timeout" then -- a timeout is not an error, it is actually success
					error=nil
				end

				if error then -- error causes disconenct
				
					client.packets.disconnect(v)
					
				elseif line then -- got data
				
					client.packets.received(v , line)
					
				end
		end
		
	-- handle any input

		for _,con in pairs(packets.active) do
			for i,line in ipairs(con.linein) do

	-- as a client we only have one connection
				wsutil.str_to_msg(line,con.msg)
				
				client.got_amsg(con.msg)
	--			print(con.msg)
			end
			con.linein={} -- readit
		end
		packets.active={} -- clear active table

	end

-- send a msg
	function client.send(msg)

		packets.send( client.con.client , wsutil.msg_to_str(msg).."\0" )

		if hooks.sent then hooks.sent(msg) end
	end

-- all msgs
	function client.got_amsg(msg)

--		print(wstr.dump(msg))
	
		if hooks.amsg then hooks.amsg(msg) end
		if msg.cmd=="ville" then
			client.got_vmsg(msg) 
		else
			client.got_cmsg(msg)
		end
	end

-- chat msgs
	function client.got_cmsg(msg)
		if hooks.cmsg then hooks.cmsg(msg) end
	end

-- game msgs
	function client.got_gmsg(msg)
		if hooks.gmsg then hooks.gmsg(msg) end
	end

-- ville msgs
	function client.got_vmsg(msg)
		if hooks.vmsg then hooks.vmsg(msg) end
	end


	return client
end

