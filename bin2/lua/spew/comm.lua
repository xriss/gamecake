


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
module("spew.comm")


-- all connections look up by client
cons=cons or {}

-- waiting with data to read
active=active or {}

-- create table of connections for use in socket.select
recvt=recvt or {}



-----------------------------------------------------------------------------
--
-- create a con for this client
--
-----------------------------------------------------------------------------
function setup(client,opts)

	if not client then return nil end
	
opts=opts or {}

	local con={}
	cons[client]=con
	
	con.client=client
	con.format=opts.format
	con.linein={} -- lines come in here
	con.msg={} -- our current incoming msg for spew communication deltas

-- keep a table for connect to use
	local n=#recvt+1
	recvt[n]=client
	recvt[client]=n

	client:settimeout(0.00001) -- this is a hack fix?
	
	return con
end


-----------------------------------------------------------------------------
--
-- remove this client from active clients, destroying the con
--
-----------------------------------------------------------------------------
function clean(client)

local con=cons[client]

	client:close()
	
	local n=recvt[client]
	recvt[client]=nil
	table.remove(recvt,n)
	
	cons[client]=nil
	active[client]=nil

	return con
end

-----------------------------------------------------------------------------
--
-- get con from client
--
-----------------------------------------------------------------------------
function con(client)
	return cons[client]
end




-----------------------------------------------------------------------------
--
-- client connect
--
-----------------------------------------------------------------------------
function connected(client,format)

local con=cons[client]

	client:settimeout(0.00001) -- this is a hack fix?

	return con
end
				
-----------------------------------------------------------------------------
--
-- client disconnect
--
-----------------------------------------------------------------------------
function disconnect(client)
	return clean(client)
end


-----------------------------------------------------------------------------
--
-- send a line to a client
--
-----------------------------------------------------------------------------
function send(client,line)

local con=cons[client]

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
function received(client,line)

local con=cons[client]

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
		return clean(client) -- and close connection on spam
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

	if con.linein[1] then active[client]=con end -- flag the connection as waiting

	return con
end



