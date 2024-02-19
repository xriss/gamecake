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

Special packets that do not add to the stream of user data but instead 
are used internally by this protocol.

	bits,bit
	0x00,0x00

Sending a packet with bits=0 and bit=0 is an empty packet and will 
Ignore any data sent in the packet, which will probably be empty but 
does not have to be. Useful if you want to probe to find out if the 
network gets better or worse when sending different size packets.


	bits,bit
	0x00,0x01

Sending a packet with bits=0 and bit=1 is a hostname packet. The 
payload data is a simple null terminated utf8 string with the hostname 
of the sender. A null terminated utf8 string of the senders ip4 address 
and a null terminated utf8 string of the senders ip6 address. These 3 
values combined will be assumed uniqueish, technically they can clash 
but we only have so much to go on here and its a reasonable assumption. 

When received, each of the 3 strings needs to be clamped to 255 bytes 
before use and the ip4 and ip6 addresses replaced with empty strings if 
invalid.


	bits,bit
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

]]
M.functions.addr_to_list=function(addr)
	
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
	
	local len=#list
	local p=(last and 1 or 0)
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
		if last then -- port
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
		if last then -- port
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
		if udp6 then	assert( udp6:sendto(m,"::1",       baseport) )
		else			assert( udp4:sendto(m,"127.0.0.1", baseport) )
		end
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
