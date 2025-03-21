--
-- (C) 2024 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M

--[[#lua.wetgenes.tasks_msgp

	local msgp=require("wetgenes.tasks_msgp")

A stream of lowlevel udp packets with automatic resend ( data will get
there eventual ) but hopefully without clogging up an ongoing pulse of
smaller packets that contain EG controller state.

We use lua sockets and lanes with some help from wetgenes.tasks to
manage the lanes.


Simple UDP data packets with auto resend, little endian with this 6 byte header.

	u16		idx			//	incrementing and wrapping idx of this packet
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated

A maximum packet size of 63k seems to give good throughput, letting the
lower level code split and recombine packets is probably going to be
more efficient than us. Note that we care about best case speed, when
shit lags and packets drop we care less about performance and more
about maintaining a connection with correct state. IE its important
that we recover not that we pretend there is no problem. At 255 bits *
63k bit size we can seen a data of about 15.6m.

Technically 63k UDP packets might just get dropped as too big but I
suspect modern hardware is not really going to cause problems.

bit/bits only goes up to 255. the 0 value in either of these is
reserved as a flag for possible slightly strange packets in the future
eg maybe we need a ping? and should be ignored as a bad packet if you
do not understand them.

An ack of 0x1234 also implies that 0x9234 to 0x1233 are in the past and
0x1234 to 0x9233 are in the future.

Also if we have nothing new to say after 100ms but have received new
packets then we can send a packet with empty data (0 length) as an ack
only packet.

When connecting to a port for the first time, the idx value must start
at a special number, that number is configurable and should be
different for each app. An extra bit of sanity during introductions to
indicate that the data stream will be understood by both parties.

Special packets that do not add to the stream of user data but instead
are used internally by this protocol.

	id		bits	bit
	PING	0x00	0x02
	PONG	0x00	0x03
	HAND	0x00	0x04
	SHAKE	0x00	0x05
	RESEND	0x00	0x08
	PULSE	0x00	0x10

A PING packet will be accepted and acknowledged after handshaking.
Sending a PING packet will cause a PONG response with the same data.

A PONG packet will be accepted and acknowledged after handshaking, its
data should be the same as the PING it is responding too.

A HAND packet begins handshaking, the payload data is a string array.
Consisting of a series of null terminated utf8 strings.

A SHAKE packet ends handshaking The payload data is a string array.
Consisting of a series of null terminated utf8 strings.

Both the HAND and the SHAKE packets contain the same payload data which
consists of the following utf8 strings, each terminated by a 0 byte.

	host name
	host ip4
	host ip6
	host port
	client addr

When received, each of the strings should to be clamped to 255 bytes
and the values validated or replaced with empty strings before use.

host name is maybe best considered a random string, it could be anything.

host ip4 will be the hosts best guess at their ip4, it is the ip4 they
are listening on.

host ip6 will be the hosts best guess at their ip6, it is the ip6 they
are listening on.

host port will be the local port the host is listening on, ip4 and ip6,
but as they may be port forwarding we might connect on a different
port.

client addr is the client ip and port the sender sent this packet too.
If ipv4 it will be a string of the format "1.2.3.4:5" and if 1pv6 then
"[1::2]:3" So the ip possibly wrapped in square brackets (ipv6) and
then a colon followed by the port in url style format.

A RESEND packet consists of a payload of little endian 16bit idxs to
packets we would like to be resent to fill in missing data.

A PULSE packet contains user data but does not get resent or
acknowledged, its IDX must be set to 0 and this IDX should be ignored
when received as this is out of stream data. Pulse packets are small
regular packets of user data, eg current client input state. Data sent
is included as part of the normal received data stream but there is no
guarantee that it will be delivered or when it will be delivered
relative to other data. The data must be small enough to fit in a
single packet. Think of this as something of a raw UDP packet in terms
of how it works.

]]



-- only cache this stuff on main thread
do
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)

-- ids we use for special packets bit, when bits is 0

M.PACKET={}
M.PACKET.PING   = 0x02
M.PACKET.PONG   = 0x03
M.PACKET.HAND   = 0x04
M.PACKET.SHAKE  = 0x05
M.PACKET.RESEND = 0x08
M.PACKET.PULSE  = 0x10

-- max size of each data packet we will size, maybe 8kish chunks and 2megish in total?
-- 63k chunks give better data throughput but might be unsafe?
-- most chunks will be small anyway I think large chunks will be faster
M.PACKET_HEAD       = 6
M.PACKET_SIZE_RAW   = (1024*63.75)
M.PACKET_SIZE       = M.PACKET_SIZE_RAW-M.PACKET_HEAD
M.PACKET_TOTAL_SIZE = 255*M.PACKET_SIZE

-- do not set this value except for testing, it will cause random packet drops
--M.PACKET_DROP_TEST  = 0.1

-- limit packets sent per ms to
M.PACKET_PER_MS = 3
-- so PACKET_PER_MS*1000 packets per second max
-- and PACKET_SIZE*PACKET_PER_MS*1000 bytes per second max

-- time between actions in seconds
M.TIME={}
M.TIME.SELECT = 0.001   -- select timeout
M.TIME.UPDATE = 0.100	-- update clients
M.TIME.SEND   = 0.100	-- wait at least this long before resending
M.TIME.ACK    = 0.200	-- force an ack for unacked packets
M.TIME.PING   = 60		-- perform a ping to measure latency


M.ENCODE5="0123456789abcdefghjkmnpqrtuvwxyz" -- 32 chars 5bits
-- intended for human typing or other communication
-- avoiding "OILS" as these letters may be easily mistaken for numerals
-- any "OILS" can be assumed to mistyped "0115" numbers
-- lowercase helps but can not be guaranteed



--[[#lua.wetgenes.tasks_msgp.clean_name

Clean a hostname so it becomes upercase letters and numbers with
possible underscores where any other chars would be, eg whitespace.

we also try not to start or end with an _

]]
M.functions.clean_name=function(name)
	name=tostring(name)
	name=name:gsub("%W+","_") -- replace non alpha/numbers with single _
	name=name:match( "^_*(.-)_*$" ) -- do not start or end with _
	name=name:upper() -- force uppercase
	name=name:sub(1,32) -- make sure string is not huge
	return name
end


--[[#lua.wetgenes.tasks_msgp.ip6_to_addr

Parse an array of 8 numbers into an ip6 address with :: in the first
longest run of zeros if needed and lowercase hex letters.

]]
M.functions.ip6_to_addr=function(list)

	local zer=0 -- idx to collapse
	local rep=0 -- number of 0s to remove
	local cnt=0
	local check_best_and_reset=function(i)
		if cnt>rep then -- better
			rep=cnt
			zer=i-cnt
		end
		cnt=0 -- reset
	end
	for i=1,8 do
		if list[i]==0 then cnt=cnt+1 else check_best_and_reset(i) end
	end
	check_best_and_reset(9)

	local s=""
	local i=1
	while i<=8 do
		if rep>=2 and i==zer then -- 2 or more zeros
			s=s.."::"
			i=i+rep
		else
			if i>1 and i~=zer+rep then
				s=s..":"
			end
			s=s..string.format("%x",list[i])
			i=i+1
		end
	end
	return s
end

--[[#lua.wetgenes.tasks_msgp.list_to_addr

Parse an array of numbers into an ip address and maybe port.

	#4 {1,2,3,4} -- ip4
	#5 {1,2,3,4,5} -- ip4:port
	#8 {1,2,3,4,5,6,7,8} -- ip6
	#9 {1,2,3,4,5,6,7,8,9} -- [ip6]:port

]]
M.functions.list_to_addr=function(list)

	local len=#list
	if     len==4 then -- ip4
		return string.format("%d.%d.%d.%d",list[1],list[2],list[3],list[4])
	elseif len==5 then -- ip4:port
		return string.format("%d.%d.%d.%d:%d",list[1],list[2],list[3],list[4],list[5])
	elseif len==8 then -- ip6
		return M.ip6_to_addr(list)
	elseif len==9 then -- [ip6]:port
		return string.format("[%s]:%d",M.ip6_to_addr(list),list[9])
	end

end

--[[#lua.wetgenes.tasks_msgp.addr_to_list

parse an ip address + maybe port encoded as a string into a list of
numbers, the length of the list represents the type so

	#4 {1,2,3,4} -- ip4
	#5 {1,2,3,4,5} -- ip4:port
	#8 {1,2,3,4,5,6,7,8} -- ip6
	#9 {1,2,3,4,5,6,7,8,9} -- [ip6]:port

Optionally include a numeric port to add/replace in the list after
parsing.

]]
M.functions.addr_to_list=function(addr,port)

	local ipv4=false
	local ipv6=false
	local zer
	local first,last
	local list={}
	local idx=0
	for a,b in string.gmatch(addr,"([%p]*)([^%p]*)") do
		if a=="" and b=="" then break end
		idx=idx+1
		if last then return nil,"too many numbers" end -- too many numbers
		if not first then
			if a=="[" or a=="" or a=="::" or a=="[::" or a=="[::]:" then
				first=a
				if     first=="[::]:" then -- 8 0s and a port
					last=""
					ipv6=true
					list={0,0,0,0,0,0,0,0}
				elseif first=="::" or first=="[::"then
					ipv6=true
					zer=#list
				elseif first=="[" then
					ipv6=true
				end
			else
				return nil,"invalid start"
			end
		else
			if not ( ipv4 or ipv6 ) then
				if     a=="." then            ipv4=true
				elseif a==":" or a=="::" then ipv6=true
				else
					return nil,"invalid separator"
				end
			end
			if ipv4 then
				if a~="." then
					if a==":" then last=a
					else
						return nil,"invalid ipv4 separator"
					end
				end
			end
			if ipv6 then
				if a~=":" then
					if a=="]:" then
						last=a
					elseif a=="::" then
						if zer then return nil,"multiple ::" end
						zer=#list
					elseif a=="::]:" then
						if zer then return nil,"multiple ::" end
						zer=#list
						last=a
					else
						return nil,"invalid ipv6 separator"
					end
				end
			end
		end
		if b~="" then
			list[#list+1]=b
		end
	end

	if zer then -- need to insert some zeros
		local pad=8-#list
		if last then pad=pad+1 end -- we have a port so pad to 9 values
		if pad<2 then return nil,"invalid ipv6 ::" end
		for i=1,pad do
			table.insert(list,zer+1,0)
		end
	end

	if port and ipv4 then list[5]=port end -- force port
	if port and ipv6 then list[9]=port end
	local len=#list
	local p=((last or port) and 1 or 0)
	if ipv4 and len<4   then return nil,"too few ipv4 numbers" end
	if ipv4 and len>4+p then return nil,"too many ipv4 numbers" end
	if ipv8 and len<8   then return nil,"too few ipv6 numbers" end
	if ipv8 and len>8+p then return nil,"too many ipv6 numbers" end

	local bracks=0
	if first=="[::" or first=="[" then
		bracks=bracks+1
	end
	if last=="]:" or last=="::]:" then
		bracks=bracks+1
	end
	if bracks==1 then return nil,"invalid []" end

	if ipv4 then -- convert strings and check range
		for i=1,4 do
			if type(list[i])=="string" then
				list[i]=tonumber(list[i],10)
			end
			if list[i]~=list[i] then return nil,"NAN" end
			if list[i]<0 or list[i]>255 then return nil,"number out of range" end
		end
		if list[5] then -- port
			if type(list[5])=="string" then
				list[5]=tonumber(list[5],10)
			end
			if list[5]~=list[5] then return nil,"NAN" end
			if list[5]<0 or list[5]>65535 then return nil,"port out of range" end
		end
	end

	if ipv6 then -- convert strings and check range
		for i=1,8 do
			if type(list[i])=="string" then
				list[i]=tonumber(list[i],16)
			end
			if list[i]~=list[i] then return nil,"NAN" end
			if list[i]<0 or list[i]>65535 then return nil,"number out of range" end
		end
		if list[9] then -- port
			if type(list[9])=="string" then
				list[9]=tonumber(list[9],10)
			end
			if list[9]~=list[9] then return nil,"NAN" end
			if list[9]<0 or list[9]>65535 then return nil,"port out of range" end
		end
	end

	return list
end

--[[#lua.wetgenes.tasks_msgp.addr_to_ip_port

parse an ip string and port number from an addr/addr+port/addr_list

returns addr,port that we can then use with luasocket

]]
M.functions.addr_to_ip_port=function(addr,port)
	local addr_list
	if type(addr)=="table" then -- assume already parsed
		addr_list=addr
	else
		addr_list=M.functions.addr_to_list(addr,port)
	end
	local ip={unpack(addr_list)} -- split into ip and port
	if #ip>5 then
		port=ip[9]
		ip[9]=nil
	else
		port=ip[5]
		ip[5]=nil
	end
	return M.functions.list_to_addr(ip),port
end

--[[#lua.wetgenes.tasks_msgp.ipsniff

Use ( google by default ) public dns servers to check if we can connect
to the internet and if we have ipv4 and/or ipv6 available.

To use alternative dns pass in an ipv4 and ipv6 address as the first
two args, eg to use cloudflare.

	ipsniff("1.1.1.1","2606:4700:4700::1111")

returns the ipv4,ipv6 address of this host when connecting to that dns
or nil if no connection possible.

This result can be used as a bool to indicate working ipv6 or ipv4
internet and is also a best guess as to our ip when connecting to other
devices.

Note that this will only work if we are connected to the internet can
reach the dns servers etc etc, standard networks can be crazy
disclaimer applies.

So great if this works but still need a fallback plan, eg assuming
local ipv4 network is available.

]]
M.functions.ipsniff=function(dns4,dns6)

	dns4=dns4 or "8.8.8.8"				-- default to google dns ipv4
	dns6=dns6 or "2001:4860:4860::8888" -- default to google dns ipv6

	local ipv4="0.0.0.0"
	local ipv6="::"

	local socket = require("socket")

	pcall(function()
		local udp4 = socket.udp4()
		udp4:settimeout(0)
		udp4:setpeername(dns4,53)
		ipv4=udp4:getsockname() -- return value
		udp4:close()
	end)

	pcall(function()
		local udp6 = socket.udp6()
		udp6:settimeout(0)
		udp6:setpeername(dns6,53)
		ipv6=udp6:getsockname() -- return value
		udp6:close()
	end)

	-- convert ip0 strings to nil
	if ipv4=="0.0.0.0" then ipv4=nil end
	if ipv6=="::"      then ipv6=nil end

	return ipv4,ipv6
end


--[[#lua.wetgenes.tasks_msgp.pack

If given aa udp data packet string, convert it to a table.

If given table, convert it to a udp data packet string. little endian

	u16		idx			//	incrementing and wrapping idx of this packet
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated

]]
M.functions.pack=function(a)
	local ta=type(a)
	if ta=="string" then

		local b={ string.byte(a,1,6) }
		if #b<6 then return end -- bad header

		return {
			idx  = b[1]+b[2]*256	,
			ack  = b[3]+b[4]*256	,
			bit  = b[5]				,
			bits = b[6]				,
			data = string.sub(a,7)	}

	elseif ta=="table" then
		return string.char(
					   a.idx      %256	,
			math.floor(a.idx /256)%256	,
					   a.ack      %256	,
			math.floor(a.ack /256)%256	,
					   a.bit			,
					   a.bits			)..(a.data or "")

	else
		return {
			idx  = 0	,
			ack  = 0	,
			bit  = 0	,
			bits = 0	}
	end
end


------------------------------------------------------------------------
end -- The functions below are free running tasks and should not depend on any locals
------------------------------------------------------------------------

--[[#lua.wetgenes.tasks_msgp.msgp_code

lanes task function for handling msgp communication.

]]
M.functions.msgp_code=function(linda,task_id,task_idx)
	local M -- hide M for thread safety
	local global=require("global") -- lock accidental globals

	local task_id_msg=task_id..":msg"

	local lanes=require("lanes")
	if lane_threadname then lane_threadname(task_id) end

	local msgp=require("wetgenes.tasks_msgp")
	local socket = require("socket")
	local now=function() return socket.gettime() end -- time now

	local baseport=2342
	local basepack=2342
	local hostname=msgp.clean_name( socket.dns.gethostname() ) -- get a clean hostname
	local ip4,ip6=msgp.ipsniff()
	local udp4,udp6
	local hostport
	local clients={} -- client state, reset on host cmd
	local update_time=now() -- update clients / gc etc

	-- return a-b with idx wrapping fixed
	local diff=function(a,b)
		local d=a-b
		if d>=0x8000 then d=d-0x10000 end
		if d<-0x8000 then d=d+0x10000 end
		return d
	end
	local manifest_client=function(ip,port,reset)

		-- parse ip:port and rebuild it to a standard url format
		local addr_list=assert( msgp.addr_to_list(ip,port) )
		local addr=msgp.list_to_addr(addr_list)
		local addr_ip,addr_port=msgp.addr_to_ip_port(addr_list)

		local client=clients[addr] -- client already exists?
		if reset then client=nil end -- we want to reset the client
		if client then return client end -- found it

		-- build new client and remember it
		client={}
		clients[addr]=client
		client.state="new" -- this is a new client
		client.addr_list=addr_list
		client.addr=addr
		client.ip=addr_ip
		client.port=addr_port
		client.ack=0
		client.ping=0
		client.pong=""
		client.ping_time=0
		client.send_time=0
		client.send_idx=basepack
		client.send_ack=basepack
		client.send={}
		client.recv_time=0
		client.recv_max=basepack
		client.recv_idx=basepack
		client.recv_gc=basepack
		client.recv={}
		return client
	end
	local send_client=function(client,idx,pack)
		local t=now() -- do not resend too fast
		if pack then -- prepare first send
			client.send[idx]={ 0 , pack }
			client.send_time=t -- only first send counts
		end
		if not client.send[idx] then return end -- can not resend
		if ( t - client.send[idx][1] ) > msgp.TIME.SEND then
			if msgp.PACKET_PER_MS then -- limit
				local ms=math.floor(t*1000)
				if ms~=client.send_ms_stamp then -- advance
					client.send_ms_stamp=ms
					client.send_ms_count=1
				else
					client.send_ms_count=client.send_ms_count+1
				end
				if client.send_ms_count>=msgp.PACKET_PER_MS then
					lanes.sleep(0.001)
				end
			end
			local udp=(#client.addr_list>5) and udp6 or udp4
--			socket.select(nil,{udp}) -- wait till safe to send? nope, does not help...
			udp:sendto( client.send[idx][2] , client.ip , client.port )
			client.send[idx][1]=t
		end
	end
	local send_packet=function(client,p)
		p.bit=p.bit or 0
		p.bits=p.bits or 0
		if not p.idx then -- auto send_idx
			p.idx=(client.send_idx+1)%0x10000
		end
		if diff( p.idx , client.send_idx ) > 0 then -- advance idx
			client.send_idx=p.idx
		end
		p.ack=client.recv_idx
		send_client(client,p.idx,msgp.pack(p))
		return p
	end
	local send_packet_pulse=function(client,p)
		p.idx=0 -- idx must be 0 as this is out of stream
		p.ack=client.recv_idx -- correct ack
		p.bits=0
		p.bit=msgp.PACKET.PULSE
		local udp=(#client.addr_list>5) and udp6 or udp4
		udp:sendto( msgp.pack(p) , client.ip , client.port ) -- always send now
		return p
	end
	local ack_packet=function(client,idx)
		local p=client.send[idx]
		if not p then return end -- no packet?
		client.ack=now()-p[1]
		client.send[idx]=nil -- remove on final ack
	end
	-- advance ack and ack packet
	local advance_ack=function(client,p)
		-- advance send ack and remove old packets
		local d=diff(p.ack,client.send_ack)
		if d>=0 then
			client.recv_time=now()
		end
		if d>0 then -- advance client ack and flag packets
			for idx=client.send_ack,client.send_ack+d-1 do
				ack_packet(client,idx%0x10000) -- ack and remove
			end
			client.send_ack=p.ack
		end
	end
	-- returns p if we should process it
	local recv_packet=function(client,p)
		if diff( p.idx , client.recv_idx ) < 0 then return end -- old packet, ignore it
		client.recv[p.idx]=p
		if diff( p.idx , client.recv_max ) > 0 then
			client.recv_max=p.idx	-- remember max idx received
		end
		if p.bits>1 then -- grab all the data from each bit
			p.datas={} -- if this is set then we have all the bits
			for i = p.idx+0x10001-p.bit , p.idx+0x10000+p.bits-p.bit do
				local r=client.recv[ i%0x10000 ]
				if r then
					p.datas[r.bit]=r.data
				else
					p.datas=nil -- fail
					break
				end
			end
		end
		while client.recv[ client.recv_idx ] do -- advance
			local r=client.recv[ client.recv_idx ]
			-- remove packets if complete
			if r.bits<2 then -- single packet
				client.recv[ client.recv_idx ]=nil -- remove
			elseif r.bit==r.bits then -- remove all when we get the last bit
				for i = r.idx+0x10001-r.bit , r.idx+0x10000+r.bits-r.bit do
					client.recv[ i%0x10000 ]=nil -- remove
				end
			end
			client.recv_idx=(client.recv_idx+1)%0x10000
		end
		advance_ack(client,p)
		return p
	end
	local send_msg=function(msg)
		linda:send( nil , task_id_msg , msg )
	end
	local update_client=function(client)
		if client.state=="msg" or client.state=="handshake" then -- connected
			local t=now()

			-- resend
			local d=diff( client.recv_max , client.recv_idx )
			if d > 0 then -- we have holes
				local list={}
				for idx = client.recv_idx , client.recv_idx+d do
					if not client.recv[idx%0x10000] then
						local b=idx%0x10000
						list[#list+1]=string.char(b%256)
						list[#list+1]=string.char(math.floor(b/256)%256)
					end
				end
				if #list>0 then -- request resends
					send_packet( client , {
						bit=msgp.PACKET.RESEND,
						data=table.concat(list),
					} )
				end
			end

			-- ping
			if	(t-client.ping_time) > msgp.TIME.PING then
				client.ping_time=t
				client.pong=tostring(client.ping_time) -- the response we expect
				send_packet( client , {
					bit=msgp.PACKET.PING,
					data=client.pong,
				} )
			end

			-- keep alive
			if	(client.send_time<client.recv_time) and (t-client.recv_time) > msgp.TIME.ACK then
			-- ack last sent packet with empty data
				send_packet( client , {
					bit=1,
					bits=1,
					data="",
				} )
			end

			do
				local scnt=0 ; for i,v in pairs(client.send) do scnt=scnt+1 end
				local rcnt=0 ; for i,v in pairs(client.recv) do rcnt=rcnt+1 end
				if (scnt+rcnt) > 500 then -- something went wrong with the cleanup
					print("counts",task_id,scnt,rcnt)
				end
			end
		end
	end

	local request=function(memo)
		local ret={}

		if memo.cmd=="host" then

			clients={} -- forget all clients

			if memo.baseport then
				baseport=memo.baseport
			end
			if memo.basepack then
				basepack=memo.basepack
			end

			if ip6 then
				if not udp6 then
					udp6 = socket.udp6()
					udp6:settimeout(0)
				end
			end
			if ip4 or ( not ip6 ) then
				if not udp4 then
					udp4 = socket.udp4()
					udp4:settimeout(0)
				end
			end

			hostport=nil
			for i=0,1000 do -- try baseport to baseport+1000
				if ( not udp4 ) or ( udp4:setsockname("*", baseport+i) ) then
					if ( not udp6 ) or ( udp6:setsockname("*", baseport+i) ) then
						hostport=baseport+i
						break
					end
				end
			end
			if not hostport then ret.error="could not bind to port" end

			ret.port=hostport
			ret.name=hostname
			ret.ip4=ip4
			ret.ip6=ip6

			-- preferred connection addr
			local list=msgp.addr_to_list(ip6 or ip4,hostport)
			ret.addr=msgp.list_to_addr(list)

		elseif memo.cmd=="join" then

			assert(hostport) -- must be connected

			local client=manifest_client(memo.addr)

			ret.client={}
			ret.client.addr=client.addr
			ret.client.ip=client.ip
			ret.client.port=client.port

			client.state="handshake"
			send_packet( client , {
				idx=basepack,
				bit=msgp.PACKET.HAND,
				data=hostname.."\0"..ip4.."\0"..ip6.."\0"..tostring(hostport).."\0"..client.addr.."\0",
			} )

		elseif memo.cmd=="pulse" then

			local client=manifest_client(memo.addr)

			send_packet_pulse( client , {
				data=memo.data,
			} )

		elseif memo.cmd=="send" then

			local client=manifest_client(memo.addr)

			local size=string.len(memo.data)

			if size<=msgp.PACKET_SIZE then -- send a single bit

				send_packet( client , {
					bit=1,
					bits=1,
					data=memo.data,
				} )

			else -- send in multiple bits

				local bits=math.ceil(size/msgp.PACKET_SIZE)
				assert(bits<256)

				local base_idx=client.send_idx
				for bit=1,bits do
					local idx=(base_idx+bit)%0x10000
					send_packet( client , {
						idx=idx,
						bit=bit,
						bits=bits,
						data=string.sub( memo.data , 1+((bit-1)*msgp.PACKET_SIZE) , (bit*msgp.PACKET_SIZE) ),
					} )
				end

			end
		end

		return ret
	end

	local packet=function(dat,ip,port)

		local p=msgp.pack(dat)
		if not p then return end -- bad header
		p.time=now() -- time we received packet
		local client=manifest_client(ip,port)
		local indent=""

		if false then
			local bitid=p.bit.."/"..p.bits
			local pings=math.ceil(client.ping*1000).."-"..math.ceil(client.ack*1000)
			local tims=math.floor(p.time*1000)%100000
			local idx=string.format("%d",p.idx)
			local ack=string.format("%d",p.ack)
			if diff(p.ack,client.send_ack) <= 0 then ack="-" end -- old ack

			if port==2342 then
				print("",ack,"","","","",idx,"",bitid,tims,pings)
			else
				print(idx,"",bitid,tims,pings,"","",ack)
			end

		end

		local client_handshake=function()
			client.handshake={}

			-- parse strings from packet
			local i=0
			for s,_ in string.gmatch(p.data,"([^%z]*)(%z)") do
				i=i+1
				if     i==1 then
					client.handshake.name=msgp.clean_name(s)
				elseif i==2 then
					client.handshake.ip4=msgp.list_to_addr( msgp.addr_to_list(s) )
				elseif i==3 then
					client.handshake.ip6=msgp.list_to_addr( msgp.addr_to_list(s) )
				elseif i==4 then
					client.handshake.port=tonumber(s)
					if not client.handshake.port or client.handshake.port~=client.handshake.port then client.handshake.port=nil end
				elseif i==5 then
					client.handshake.metoyou=msgp.list_to_addr( msgp.addr_to_list(s) )
				end
				if i>=5 then break end
			end

		end

		if p.bits==0 then -- protocol packet

			-- packets that we do not recognize here will be ignored

			if		p.bit==msgp.PACKET.HAND
			and		p.idx==basepack
			and		client.state~="handshake"
			then
			-- HAND packet

				client=manifest_client(ip,port,true) -- reset client

				client.state="msg"
				if not recv_packet(client,p) then return end

				send_packet( client , {
					idx=basepack,
					bit=msgp.PACKET.SHAKE,
					data=hostname.."\0"..ip4.."\0"..ip6.."\0"..tostring(hostport).."\0"..client.addr.."\0",
				} )

				client_handshake()

				send_msg({
					why="connect",
					addr=client.addr,
					handshake=client.handshake,
				})

			elseif	p.bit==msgp.PACKET.SHAKE
			and		p.idx==basepack
			and		client.state=="handshake"
			then
			-- SHAKE packet

				client.state="msg"
				if not recv_packet(client,p) then return end

				client_handshake()

				send_msg({
					why="connect",
					addr=client.addr,
					handshake=client.handshake,
				})

			elseif	p.bit==msgp.PACKET.PING
			and		client.state=="msg"
			then
			-- PING packet

				if not recv_packet(client,p) then return end

				send_packet( client , {
					bit=msgp.PACKET.PONG,
					data=p.data,
				} )

			elseif	p.bit==msgp.PACKET.PONG
			and		client.state=="msg"
			then
			-- PONG packet

				if not recv_packet(client,p) then return end

				if p.data==client.pong then -- expected
					local t=tonumber(p.data)
					if t and t==t then
						client.ping=now()-t
					end
				end

			elseif	p.bit==msgp.PACKET.RESEND
			then
				if not recv_packet(client,p) then return end
				local data=p.data
				for i=1,#data,2 do
					local a=string.byte(data,i,i)
					local b=string.byte(data,i+1,i+1)
					local idx=a+b*256
					send_client(client,idx)
				end

			elseif	p.bit==msgp.PACKET.PULSE
			and		p.idx==0
			then

				advance_ack(client,p)

				send_msg({
					why="pulse",
					addr=client.addr,
					data=p.data,
				})

			end

		else -- data packet

			if not recv_packet(client,p) then return end

			local data
			if p.bits>1 and p.datas then
				data=table.concat(p.datas)
			else
				data=p.data
			end

			if data and data~="" then -- ignore empty data ( sent as ack only )
				send_msg({
					why="data",
					addr=client.addr,
					data=data,
				})
			end

		end


	end


	while true do

		local _,memo= linda:receive( 0 , task_id ) -- wait for any memos coming into this thread

		if memo then
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

		local socks=socket.select({udp4,udp6},{},msgp.TIME.SELECT) -- wait for any udp packets
		for _,sock in ipairs(socks or {}) do
			while true do
				local dat,ip,port=sock:receivefrom(msgp.PACKET_SIZE_RAW)
				if dat then
					if not msgp.PACKET_DROP_TEST
					or math.random()>=msgp.PACKET_DROP_TEST
					then
						packet(dat,ip,port)
					else
						print(task_id,"packet drop test")
					end
				else
					break
				end
			end
		end

		if (now()-update_time) > msgp.TIME.UPDATE then
			update_time=now()
			for ip,client in pairs(clients) do
				update_client(client)
			end
		end
	end

end
