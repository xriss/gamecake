--
-- (C) 2024 Kriss@XIXs.com
--


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
bits, we should be able to send just under 2meg in a single call.

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

Special packets

	bits,bit
	0x00,0x01

Sending a packet with bits=0 and bit=1 is a hostname packet. The 
payload data is a simple utf8 string with the hostname of the sender. ( 
possibly / probably unique until proven otherwise ) 

	0x00,0x02

Sending a packet with bits=0 and bit=2 is a peername packet. The 
payload data is a simple utf8 string with the peer ip and port the 
sender sees for this host. If ipv4 it will be a string of the format 
"1.2.3.4:5" and if 1pv6 then "[1::2]:3" So the ip possibly wrapped in 
square brackets (ipv6) and then a colon followed by the port.

]]


-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.functions={}
M.metatable={__index=M.functions}
setmetatable(M,M.metatable)


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
		local udp4 = socket.udp()
		udp4:settimeout(0)
		udp4:setpeername(dns4,53)
		ipv4=udp4:getsockname() -- return value
		udp4:close()
	end)

	pcall(function()
		local udp6 = socket.udp()
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

	local baseport=2342
	local basepack=2342

	local socket = require("socket")


	local hostname=socket.dns.gethostname()
	local ip4,ip6=M.ipsniff()
	
	local udp4,udp6
	
	if ip6 then
		udp6 = socket.udp()
		udp6:settimeout(0)
	end
	if ip4 or ( not ip6 ) then
		udp4 = socket.udp()
		udp4:settimeout(0)
	end
	
	local port
	for i=0,1000 do
		if ( not udp4 ) or ( udp4:setsockname("0.0.0.0", baseport+i) ) then
			if ( not udp6 ) or ( udp6:setsockname("::", baseport+i) ) then
				port=baseport+i
				break
			end
		end
	end
	if not port then error("could not bind to port") end

print( hostname , ip4 , ip6 , port )
	
	if port ~= baseport then
		local m=M.pack()
		m.bit=1
		m=M.pack(m)
		assert( udp4:sendto(m,"127.0.0.1",baseport) )
		assert( udp6:sendto(m,"::1",baseport) )
	end
	
	while true do
		local socks=socket.select({udp4,udp6},{},1)
		for _,sock in ipairs(socks or {}) do
			local dat,ip,port=sock:receivefrom()
			print(dat,ip,port)
		end
	end
end


M.functions.msgp_code=function(linda,task_id,task_idx)

end


-- this module must be configured before use
M.configure=function(conf)
conf=conf or {}
local C={}

setmetatable(C,M.metatable)

return C
end
