--
-- (C) 2013 Kriss@XIXs.com
--



local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os
local print=print



-----------------------------------------------------------------------------
--
--
--
-----------------------------------------------------------------------------
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
--module("spew.comm")


-----------------------------------------------------------------------------
--
-- create a packet handler
--
-----------------------------------------------------------------------------
function M.create(packets)

	packets=packets or {}

-- all connections look up by client
	packets.cons=packets.cons or {}

-- waiting with data to read
	packets.active=packets.active or {}

-- create table of connections for use in socket.select
	packets.recvt=packets.recvt or {}

-----------------------------------------------------------------------------
--
-- create a con for this client
--
-----------------------------------------------------------------------------
	function packets.setup(client,opts)


		if not client then return nil end
		
		opts=opts or {}

		local con={}
		packets.cons[client]=con
		
		con.client=client
		con.format=opts.format
		con.linein={} -- lines come in here
		con.msg={} -- our current incoming msg for spew communication deltas

	-- keep a table for connect to use
		local n=#packets.recvt+1
		packets.recvt[n]=client
		packets.recvt[client]=n

		client:settimeout(0.00001) -- this is a hack fix?
		
		return con
	end


-----------------------------------------------------------------------------
--
-- remove this client from active clients, destroying the con
--
-----------------------------------------------------------------------------
	function packets.clean(client)

	local con=packets.cons[client]

		client:close()
		
		local n=packets.recvt[client]
		packets.recvt[client]=nil
		table.remove(packets.recvt,n)
		
		packets.cons[client]=nil
		packets.active[client]=nil

		return con
	end

-----------------------------------------------------------------------------
--
-- get con from client
--
-----------------------------------------------------------------------------
	function packets.con(client)
		return packets.cons[client]
	end


-----------------------------------------------------------------------------
--
-- client connect
--
-----------------------------------------------------------------------------
	function packets.connected(client,format)

	local con=packets.cons[client]

		client:settimeout(0.00001) -- this is a hack fix?

		return con
	end
				
-----------------------------------------------------------------------------
--
-- client disconnect
--
-----------------------------------------------------------------------------
	function packets.disconnect(client)
		return packets.clean(client)
	end


-----------------------------------------------------------------------------
--
-- send a line to a client
--
-----------------------------------------------------------------------------
	function packets.send(client,line)

	local con=packets.cons[client]

		if ( not client ) or ( not line ) or ( not con ) then return end
		
		if line~="" then
		
			client:send(line)
			
	--print(line)

		end

		return con
	end

	
-----------------------------------------------------------------------------
--
-- receive some data from a client
--
-----------------------------------------------------------------------------
	function packets.received(client,line)

	local con=packets.cons[client]

	local line_term="\0" -- spew default

		if con.format=="spew" then -- default
	--		line_term="\0"
		elseif con.format=="telnet" then -- break on \n not \0
			line_term="\n"
		elseif con.format=="irc" then -- break on \n not \0
			line_term="\n"
		elseif con.format=="websocket" then -- break on \255 not \0
			line_term="\255"
		end
		
		if con.lineparts then -- continue our cache
		
			con.lineparts=con.lineparts..line			
			
		else -- start new cache
		
			con.lineparts=line
			
		end
		
		if string.len(con.lineparts)>16384 then -- catch large packets
			return packets.clean(client) -- and close connection on spam
		end
		
		local zero,linepart
		
		zero=string.find(con.lineparts,line_term)
		
		while zero do -- we have a command or more to split up
		
			if zero>1 then
			
				linepart=string.sub(con.lineparts,1,zero-1) -- command
				con.lineparts=string.sub(con.lineparts,zero+1) -- remainder
			
				table.insert(con.linein,linepart) -- handle this line later
			else
				con.lineparts=string.sub(con.lineparts,zero+1) -- remainder
			end
			
			zero=string.find(con.lineparts,line_term)
		end

		if con.linein[1] then packets.active[client]=con end -- flag the connection as waiting

		return con
	end

	return packets
end


