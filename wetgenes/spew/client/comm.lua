--
-- (C) 2013 Kriss@XIXs.com
--


local comm=require("spew.comm")
local util=require("spew.util")

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


local spew_host="swf.wetgenes.com"
--local spew_host="swf.wetgenes.local"
local spew_port=5223

-----------------------------------------------------------------------------
--
-- basic client communication with a spew server
-- uses lua sockets to connect
--
-----------------------------------------------------------------------------

module("spew.client.comm")

-- callback hooks
hooks={}


-----------------------------------------------------------------------------
--
-- setup and connect to the spew server
--
-----------------------------------------------------------------------------
function setup()

	con=comm.setup( socket.connect(spew_host,spew_port) , {format="spew"} ) -- connect
	if not con then
		print( "failed to connect to "..spew_host )
	else
		if con.error then print(con.error) end
	end
end


-----------------------------------------------------------------------------
--
-- clean and disconnect from the spew server
--
-----------------------------------------------------------------------------
function clean()

	if con then
		comm.clean(con.client)
	end
	
end

-----------------------------------------------------------------------------
--
-- This handles any incoming data waiting but does not block
-- so call it a few times a second, every frame should be good
-- the data will be processed and you will receive callbacks for events
--
-----------------------------------------------------------------------------

function update()

	local tab,_,err=socket.select(comm.recvt,nil,0.00001)
-- read from sockets
	for i,v in ipairs(tab or {}) do
	
			local p1, error ,p2 = v:receive("*a")
			local line=p1 or p2
			
			if error=="timeout" then -- a timeout is not an error, it is actually success
				error=nil
			end

			if error then -- error causes disconenct
			
				comm.disconnect(v)
				
			elseif line then -- got data
			
				comm.received(v , line)
				
			end
	end
	
-- handle any input

	for client,con in pairs(comm.active) do
		for i,line in ipairs(con.linein) do

-- as a client we only have one connection
			util.str_to_msg(line,con.msg)
			
			got_amsg(con.msg)
--			print(con.msg)
		end
		con.linein={} -- readit
	end
	comm.active={} -- clear active table

end

-- send a msg
function send(msg)

	comm.send( con.client , util.msg_to_str(msg).."\0" )

	if hooks.sent then hooks.sent(msg) end
end

-- all msgs
function got_amsg(msg)
	if hooks.amsg then hooks.amsg(msg) end
	if msg.cmd=="ville" then
		got_vmsg(msg) 
	else
		got_cmsg(msg)
	end
end

-- chat msgs
function got_cmsg(msg)
--	print(msg.cmd)
	if hooks.cmsg then hooks.cmsg(msg) end
end

-- game msgs
function got_gmsg(msg)
--	print(msg.gcmd)
	if hooks.gmsg then hooks.gmsg(msg) end
end

-- ville msgs
function got_vmsg(msg)
--	print(msg.vcmd)
	if hooks.vmsg then hooks.vmsg(msg) end
end



