--
-- (C) 2024 Kriss@XIXs.com
--

-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.PACKET={}
M.PACKET.PING  = 0x02
M.PACKET.PONG  = 0x03
M.PACKET.HAND  = 0x04
M.PACKET.SHAKE = 0x05
	
--[[

Simple UDP data packets with auto resend, little endian with this 8 byte header.

	u16		idx			//	incrementing and wrapping idx of this packet
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u16		acks		//	flags for ack+1 (0x0001) to ack+16 (0x8000) for other packets we acknowledge, probably all 0
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated

A maximum packet size of 8k seems the agreed upon reasonably safe 
maximum size that will probably work. Breaking a msg into 255 * 8k 
bits, we should be able to send just under 2meg in a single msg.

bit/bits only goes up to 255. the 0 value in either of these is 
reserved as a flag for possible slightly strange packets in the future 
eg maybe we need a ping? and should be ignored as a bad packet if you 
do not understand them.

if ack is 0x1234 and acks is 0xffff then that means we lost idx 0x1234 
but the next 16 packets are ok. We should flag a resend if any acks 
bits are set (ie reporting a hole in data) or if our unacknowledged 
packets become old. Lets say an age of 500ms with a 500ms delay between 
sends.

An ack of 0x1234 also implies that 0x9234 to 0x1233 are in the past and 
0x1234 to 0x9233 are in the future.

Also if we have nothing new to say after 100ms but have received new 
packets then we can send a packet with empty data (0 length) as an acks 
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

A PING packet will be accepted and acknowledged after handshaking. 
Sending a PING packet will cause a PONG response with the same data.

A PONG packet will be accepted and acknowledged after handshaking, its 
data should be the same as the PING it is responding too.

A HAND packet begins handshaking, the payload data is a string array. 
Consisting of a series of null terminated utf8 strings.

A SHAKE packet ends handshaking The payload data is a string array. 
Consisting of a series of null terminated utf8 strings.

Both the HAND and the SHAKE packets contain the same payload data which 
consists of the following utf8 string, each terminated by a 0 byte. 

	host name
	host ip4
	host ip6
	host port
	client addr

When received, each of the strings should to be clamped to 255 bytes 
and the values validated or replaced with empty strings before use.

hostname is maybe best considered a random string, it could be anything.

host ip4 will be the hosts best guess at their ip4, it is the ip4 they 
are listening on.

host ip6 will be the hosts best guess at their ip6, it is the ip6 they 
are listening on.

host port will be the local port the host is listening on, but as they 
may be port forwarding we might connect on a different port.

client addr is the client ip and port the sender sees from this host, 
so it is data about us not the host. If ipv4 it will be a string of the 
format "1.2.3.4:5" and if 1pv6 then "[1::2]:3" So the ip possibly 
wrapped in square brackets (ipv6) and then a colon followed by the 
port.

]]



-- only cache this stuff on main thread
do
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)

--[[

Parse an array of 8 numbers into an ip6 address with :: in the first 
longest run of zeros if needed and lowercase hex letters.

]]
M.functions.ip6_to_addr=function(list)

	local zer=nil -- idx to collapse
	local rep=0 -- number of 0s to remove
	local cnt=0
	for i=1,9 do
		if i<9 and list[i]==0 then
			cnt=cnt+1
		else
			if cnt>rep then
				rep=cnt
				zer=i-cnt
			end
			cnt=0 -- reset
		end
	end
	local s=""
	for i=1,8 do
		if rep>=2 and i==zer then -- 2 or more zeros
			s=s.."::"
			i=i+rep
			if i>=8 then return s end -- finished
		else
			if i>1 then
				s=s..":"
			end
		end
		s=s..string.format("%x",list[i])
	end
	return s
end

--[[

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

--[[

parse an ip address + maybe port encoded as a string into a list of 
numbers, the length of the list represents the type so

	#4 {1,2,3,4} -- ip4
	#5 {1,2,3,4,5} -- ip4:port
	#8 {1,2,3,4,5,6,7,8} -- ip6
	#9 {1,2,3,4,5,6,7,8,9} -- [ip6]:port

Optionally include a numeric port to add to the list after parsing.

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
	
	if port then list[#list+1]=port end -- append port
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

--[[

parse an ip string and port number from an addr/addr+port/addr_list

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

--[[

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


--[[

If given aa udp data packet string, convert it to a table.

If given table, convert it to a udp data packet string. little endian

	u16		idx			//	incrementing and wrapping idx of this packet
	u8		bit			//	which bit this is, all bits should be joined before parsing
	u8		bits		//	how many bits in total ( all bits will be in adjacent idxs )
	u16		ack			//	we acknowledge all the packets before this idx so please send this one next (or again maybe)
	u16		acks		//	flags for ack+1 (0x0001) to ack+16 (0x8000) for other packets we acknowledge, probably all 0
	u8		data[*]		//	The payload, if multiple bits then all payloads should be concatenated

]]
M.functions.pack=function(a)
	local ta=type(a)
	if ta=="string" then
	
		local b={ string.byte(a,1,8) }
		if #b<8 then return end -- bad header

		return {
			idx  = b[1]+b[2]*256	,
			bit  = b[3]				,
			bits = b[4]				,
			ack  = b[5]+b[6]*256	,
			acks = b[7]+b[8]*256	,
			data = string.sub(a,9)	}

	elseif ta=="table" then

		return string.char(
					   a.idx      %256	,
			math.floor(a.idx /256)%256	,
					   a.bit			,
					   a.bits			,
					   a.ack      %256	,
			math.floor(a.ack /256)%256	,
					   a.acks     %256	,
			math.floor(a.acks/256)%256	)..(a.data or "")

	else
		return {
			idx  = 0	,
			bit  = 0	,
			bits = 0	,
			ack  = 0	,
			acks = 0	}
	end
end


M.functions.test_server=function(tasks)

	tasks=tasks or require("wetgenes.tasks").create()

	local log,dump=require("wetgenes.logs"):export("log","dump")

	local baseport=2342
	local basepack=2342


	local lanes=require("lanes")

	tasks:add_global_thread({
		count=1,
		id="msgp1",
		code=M.functions.msgp_code,
	})
	local ret1=tasks:do_memo({
		task="msgp1",
		cmd="host",
		baseport=baseport,
		basepack=basepack,
	})
	dump(ret1)

	tasks:add_global_thread({
		count=1,
		id="msgp2",
		code=M.functions.msgp_code,
	})
	local ret2=tasks:do_memo({
		task="msgp2",
		cmd="host",
		baseport=baseport,
		basepack=basepack,
	})
	dump(ret2)

	local ret3=tasks:do_memo({
		task="msgp2",
		cmd="join",
		addr=ret1.addr,
	})
	dump(ret3)

	while true do
		local busy=false
		-- poll for new data
		local ret=tasks:do_memo({
			task="msgp1",
			cmd="poll",
		})
		for i,msg in ipairs( ret.msgs or {} ) do
			busy=true
			msg.from="msgp1"
			dump(msg)
		end
		-- poll for new data
		local ret=tasks:do_memo({
			task="msgp2",
			cmd="poll",
		})
		for i,msg in ipairs( ret.msgs or {} ) do
			busy=true
			msg.from="msgp2"
			dump(msg)
		end
		if not busy then lanes.sleep(0.001) end -- take a little nap
	end

end


-- this module must be configured before use
M.configure=function(conf)
conf=conf or {}
local C={}

setmetatable(C,M.metatable)

return C
end

------------------------------------------------------------------------
end -- The functions below are free running tasks and should not depend on any locals
------------------------------------------------------------------------

M.functions.msgp_code=function(linda,task_id,task_idx)

	local lanes=require("lanes")
	set_debug_threadname(task_id)

	local tasks_msgp=require("wetgenes.tasks_msgp")
	local socket = require("socket")

	local baseport=2342
	local basepack=2342
	local hostname=socket.dns.gethostname()
	local ip4,ip6=tasks_msgp.ipsniff()
	local udp4,udp6
	local port
	local clients={} -- client state, reset on host cmd
	local msgs -- list of msgs waiting to be polled by main thread
	
	local manifest_client=function(ip,port,reset)

		-- parse ip:port and rebuild it to a standard url format
		local addr_list=tasks_msgp.addr_to_list(ip,port)
		local addr=tasks_msgp.list_to_addr(addr_list)
		local addr_ip,addr_port=tasks_msgp.addr_to_ip_port(addr_list)

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
		client.sent_idx=basepack
		client.sent={}
		client.recv_idx=basepack
		client.recv={}
		return client
	end
	local send_client=function(client,idx,pack)
		if pack then client.sent[idx]={ socket.gettime() , pack } end
		local udp=(#client.addr_list>5) and udp6 or udp4
		udp:sendto( client.sent[idx] , client.ip , client.port )
	end
	local send_msg=function(msg)
		if not msgs then msgs={} end
		msgs[#msgs+1]=msg
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
			
			port=nil
			for i=0,1000 do -- try baseport to baseport+1000
				if ( not udp4 ) or ( udp4:setsockname("*", baseport+i) ) then
					if ( not udp6 ) or ( udp6:setsockname("*", baseport+i) ) then
						port=baseport+i
						break
					end
				end
			end
			if not port then ret.error="could not bind to port" end
			
			ret.port=port
			ret.hostname=hostname
			ret.ip4=ip4
			ret.ip6=ip6

			-- preferred connection addr
			ret.addr_list=tasks_msgp.addr_to_list(ip6 or ip4,port)
			ret.addr=tasks_msgp.list_to_addr(ret.addr_list)

		elseif memo.cmd=="join" then
		
			assert(port) -- must be connected

			local client=manifest_client(memo.addr)
			ret.client=client
			
			client.state="handshake"
			local p={}
			p.idx=basepack
			p.bit=M.PACKET.HAND
			p.bits=0
			p.ack=basepack
			p.acks=0
			p.data=hostname.."\0"..ip4.."\0"..ip6.."\0"..tostring(port).."\0"..clent.addr.."\0"
			send_client( client , p.idx , tasks_msgp.pack(p) )
	
		elseif memo.cmd=="poll" then
		
			ret.msgs=msgs -- any data we have waiting
			msgs=nil -- reset list of data to send

		end

		return ret
	end
	
	local packet=function(dat,ip,port)
	
		local hex=dat:gsub('.',function(c) return string.format('%02X',string.byte(c)) end)
		print("data",ip,port,hex)
		local p=tasks_msgp.pack(dat)
		if not p then return end -- bad header
		
		print("pack",p.idx,p.bit,p.bits,p.ack,string.format('%04X',p.acks))
		print(p.data)
		local client=manifest_client(ip,port)
		
		if p.bits=0 then -- protocol packet
		
			-- packets that we do not recognize here will be ignored
		
			if		p.bit=M.PACKET.HAND
			and		p.idx==basepack
			and		client.state~="handshake"
			then
			-- HAND handshake packet

				client=manifest_client(ip,port,true) -- reset client

				client.state="data"
				client.recv[p.idx]=p

				local p={} -- respond to handshake
				p.idx=basepack
				p.bit=M.PACKET.SHAKE
				p.bits=0
				p.ack=basepack
				p.acks=0
				p.data=hostname.."\0"..ip4.."\0"..ip6.."\0"..tostring(port).."\0"..clent.addr.."\0"
				send_client( client , p.idx , tasks_msgp.pack(p) )

			elseif	p.bit=M.PACKET.SHAKE
			and		p.idx==basepack
			and		client.state=="handshake"
			then
			-- SHAKE handshake packet

				client.state="data"
				client.recv[p.idx]=p
			
			end

		else -- data packet
		
			send_msg({
				why="data",
				addr=client.addr,
				data=p.data,
			})

		end
		

	end

	
	while true do
	
		local busy=false

		local _,memo= linda:receive( 0 , task_id ) -- wait for any memos coming into this thread
		
		if memo then
			busy=true -- got some data
			local ok,ret=xpcall(function() return request(memo) end,print_lanes_error) -- in case of uncaught error
			if not ok then ret={error=ret or true} end -- reformat errors
			if memo.id then -- result requested
				linda:send( nil , memo.id , ret )
			end
		end

		local socks=socket.select({udp4,udp6},{},0)
		for _,sock in ipairs(socks or {}) do
			local dat,ip,port=sock:receivefrom()
			if dat then busy=true end -- got some data
			packet(dat,ip,port)
		end
		
		if not busy then lanes.sleep(0.00001) end -- take a little nap

	end

end
