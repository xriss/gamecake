
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local wmidi=require("wetgenes.midi")
local wsandbox=require("wetgenes.sandbox")



local cmds={
	{ "list",		"List available midi connections."},
	{ "dump",		"Listen for and print events."},
	{ "join",		"Connect the output of one port into the input of another."},
	{ "break",		"Break the connection between two ports."},
	{ "tweak",		"Create a tweak port to automatically adjust events from a device."},
}
for i,v in ipairs(cmds) do
	v.name=v[1]
	v.help=v[2]
end
table.sort(cmds,function(a,b) return a.name<b.name end)


-- generic options for all commands
local default_inputs=function(inputs)
	for i,v in ipairs{

		{	"help",			false,	"Print help and exit.", },

	} do
		inputs[#inputs+1]=v
	end
	return inputs
end



local cmd=table.remove(arg,1)       -- check what cmd is asked for
local cmd=cmd and string.lower(cmd) -- force lowercase

local process_command

process_command=function( cmd , arg )

if cmd=="join" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	1,			arg[0].." "..cmd.." client:port client:port",	[[

Connect from the first client:port into the second client:port If no 
port is given than port 0 is assumed.

		]], },

	}}):parse(arg):sanity()
		
	if not ( arg[1] or arg[2] ) then args.data.help=true end 
	
	if args.data.help then
		print(table.concat(args:help(),"\n"))
		return
	end
	
	local m=wmidi.create("scancake")
	m:scan()

	local from_client,from_port=m:string_to_clientport(arg[1])
	local into_client,into_port=m:string_to_clientport(arg[2])
	from_port=from_port or 0
	into_port=into_port or 0
	
	if not from_client then
		print("unknown client "..arg[1])
		return
	end
	if not into_client then
		print("unknown client "..arg[2])
		return
	end
	
	local from=from_client..":"..from_port
	local into=into_client..":"..into_port
	local frompath=m.ports[from].path
	local intopath=m.ports[into].path

	print(" Creating connection from "..from.." into "..into.." ( "..frompath.." > "..intopath.." )")

	m:subscribe{
		source_client=from_client,
		source_port=from_port,
		dest_client=into_client,
		dest_port=into_port,
	}

elseif cmd=="break" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	1,			arg[0].." "..cmd.." client[:port] client[:port]",	[[

Disconnect the connection between the first client:port and the second client:port If no 
port is given than port 0 is assumed.

		]], },

		{	2,			arg[0].." "..cmd.." client[:port]",	[[

Disconnect all connection into or from this client:port If no 
port is given than all ports will be disconnected.

		]], },


	}}):parse(arg):sanity()
		
	if not ( arg[1] ) then args.data.help=true end 
	
	if args.data.help then
		print(table.concat(args:help(),"\n"))
		return
	end

	if not arg[2] then
	
		local m=wmidi.create("breakcake")
		m:scan()

		local client,port=m:string_to_clientport(arg[1])
		assert(client)
		
		if not port then

			print(" Breaking all connections on all ports, into or from "..client)
		
			for n,v in pairs(m.subscriptions) do
			
				if v.source_client == client or v.dest_client == client then

					print((" Disconnecting %3d:%-2d %3d:%-2d"):format(
						v.source_client,	v.source_port,
						v.dest_client,		v.dest_port))

					m:unsubscribe{
						source_client=v.source_client,
						source_port=v.source_port,
						dest_client=v.dest_client,
						dest_port=v.dest_port,
					}

				end
			end

		else

			print(" Breaking all connections, into or from "..client..":"..port)

			for n,v in pairs(m.subscriptions) do
			
				if	( v.source_client == client and v.source_port == port ) or
					( v.dest_client == client and v.dest_port == port ) then

					print((" Disconnecting %3d:%-2d %3d:%-2d"):format(
						v.source_client,	v.source_port,
						v.dest_client,		v.dest_port))

					m:unsubscribe{
						source_client=v.source_client,
						source_port=v.source_port,
						dest_client=v.dest_client,
						dest_port=v.dest_port,
					}

				end
			end

		end

	else

		local m=wmidi.create("breakcake")
		m:scan()

		local from_client,from_port=m:string_to_clientport(arg[1])
		local into_client,into_port=m:string_to_clientport(arg[2])
		assert(from_client)
		assert(into_client)
		from_port=from_port or 0
		into_port=into_port or 0

		print(" Breaking connection from "..from_client..":"..from_port.." into "..into_client..":"..into_port)
		
		m:unsubscribe{
			source_client=from_client,
			source_port=from_port,
			dest_client=into_client,
			dest_port=into_port,
		}
		
	end

elseif cmd=="list" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	1,			arg[0].." "..cmd,	[[

List all clients and ports and connections between ports.

		]], },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print(table.concat(args:help(),"\n"))
		return
	end
	
	local m=wmidi.create("listcake")
	m:scan()

	local maxn=8
	for n,v in pairs(m.clients) do
		if #v.name>maxn then maxn=#v.name end
	end
	for n,v in pairs(m.ports) do
		if #v.name>maxn then maxn=#v.name end
	end
	for n,v in pairs(m.subscriptions) do
		if #n>maxn then maxn=#n end
	end

	print( "CLIENTS" )
	local clist={}
	for n,v in pairs(m.clients) do clist[#clist+1]=n end
	table.sort(clist)
	for _,n in ipairs(clist) do local v=m.clients[n]
		if v.client ~= m.client then -- do not list ourselves
			local s=string.format("%"..maxn.."s %3d",v.name:sub(-maxn),v.client)
			print( s )
		end
	end
	print()
	
	print( "PORTS" )
	local plist={}
	for n,v in pairs(m.ports) do plist[#plist+1]=n end
	table.sort(plist,function(a,b)
		local ac,ap=a:match("(%d+):(%d+)")
		local bc,bp=b:match("(%d+):(%d+)")
		ac=tonumber(ac) ap=tonumber(ap) bc=tonumber(bc) bp=tonumber(bp)
		if ac<bc then return true end
		if ac==bc and ap<bp then return true end
		return false
	end)
	for _,n in ipairs(plist) do local v=m.ports[n]
		local c=m.clients[v.client]
		local flag1,flag2="",""
		if v.SUBS_WRITE and v.WRITE then flag1=flag1.."->" elseif v.WRITE then flag1=flag1.." >" else flag1=flag1.."  " end
		if v.SUBS_READ  and v.READ  then flag2=flag2.."->" elseif v.READ  then flag2=flag2.." >" else flag2=flag2.."  " end
		local s=string.format("%"..maxn.."s:%-"..maxn.."s   %s %3d:%-2d %s",c.name:sub(-maxn),v.name:sub(-maxn),flag1,v.client,v.port,flag2)
		print( s )
	end
	print()

	print( "SUBSCRIPTIONS" )
	print()
	local slist={}
	local slines={}
	local slinesort={}
	local soutput={}
	for n,v in pairs(m.subscriptions) do
		slist[#slist+1]=n
		local source=v.source_client..":"..v.source_port
		local dest=v.dest_client..":"..v.dest_port
		slines[ source ]=slines[ source ] or {}
		slines[ dest ]=slines[ dest ] or {}
		slines[ source ][ dest ]=true
		soutput[dest]=true
	end
	for n,v in pairs(slines) do slinesort[#slinesort+1]=n end
	table.sort(slinesort,function(a,b)
		local ac,ap=a:match("(%d+):(%d+)")
		local bc,bp=b:match("(%d+):(%d+)")
		ac=tonumber(ac) ap=tonumber(ap) bc=tonumber(bc) bp=tonumber(bp)
		if ac<bc then return true end
		if ac==bc and ap<bp then return true end
		return false
	end)
	table.sort(slist,function(a,b)
		local asc,asp,adc,adp=a:match("(%d+):(%d+) > (%d+):(%d+)")
		local bsc,bsp,adc,adp=b:match("(%d+):(%d+) > (%d+):(%d+)")
		asc=tonumber(asc) asp=tonumber(asp) bsc=tonumber(bsc) bsp=tonumber(bsp)
		adc=tonumber(adc) adp=tonumber(adp) bdc=tonumber(bdc) bdp=tonumber(bdp)
		if asc<bsc then return true end
		if asc==bdc and asp<bdp then return true end
		if asc==bdc and asp==bdp and asc<bdc then return true end
		if asc==bdc and asp==bdp and asc==bdc and asp<bdp then return true end
		return false
	end)

-- graphical display
	local uplink={}
	for _,sn in ipairs(slinesort) do local sv=slines[sn]
		local sc,sp=sn:match("(%d+):(%d+)")
		local name=string.format("%3d:%-2d",sc,sp)
		local blank="      "


		local pname=string.format("%"..maxn.."s:%-"..maxn.."s",
			m.clients[tonumber(sc)].name:sub(-maxn),
			m.ports[sc..":"..sp].name:sub(-maxn))

		local count=0
		for _,c in pairs(sv) do count=count+1 end
		if count>0 then
			local s="  "..name.."  >-"
			local em="--"
			local di=0
			for _,dn in ipairs(slinesort) do
				if soutput[dn] then
					if di>=count then
						s=s.."  "
					elseif sv[dn] then
						s=s.."-|"
						di=di+1
						uplink[dn]=true
					else
						if uplink[dn] then
							s=s..em
						else
							s=s.."--"
						end
					end
				end
			end
			s=s.."    "..blank.."  "
			s=s.." "..pname
			print( s )

			local s="  "..blank.."    "
			for _,dn in ipairs(slinesort) do
				if soutput[dn] then
					if uplink[dn] then
						s=s.." |"
					else
						s=s.."  "
					end
				end
			end
			s=s.."     "..blank.."  "
--			print( s )

		end
	end
	print()
	for si=#slinesort,1,-1 do local sn=slinesort[si] local sv=slines[sn]
		if soutput[sn] then
			local sc,sp=sn:match("(%d+):(%d+)")
			local name=string.format("%3d:%-2d",sc,sp)
			local blank="      "

			local pname=string.format("%"..maxn.."s:%-"..maxn.."s",
				m.clients[tonumber(sc)].name:sub(-maxn),
				m.ports[sc..":"..sp].name:sub(-maxn))

			local s="  "..blank.."    "
			local em="  "
			for _,dn in ipairs(slinesort) do
				if soutput[dn] then
					if dn==sn then
					s=s.." |"
					em="--"
					else
					s=s..em
					end
				end
			end
			s=s.."-->  "..name.."  "
			s=s.." "..pname
			print( s )
		end
	end
	
-- explicit list of connections
--[[
	print()
	for _,n in ipairs(slist) do local v=m.subscriptions[n]
		local s=string.format("  %3d:%-2d ->  %3d:%-2d ",v.source_client,v.source_port,v.dest_client,v.dest_port)
		print( s )
	end
]]
	print()

elseif cmd=="dump" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	"all",			true,	"Attempt to connect to all ports so we will receive any public event broadcast on this system.", },

		{	"from",			"",		"Connect from this input client:port into our dump port, this overrides the --all flag.", },
		{	"into",			"",		"Connect from our dump port into this output client:port.", },

		{	1,			arg[0].." "..cmd,	[[

Open a port and dump any events we receive to the console, one line per 
event, then send the even out of our port. We can be used as a debug 
shim to see what events are travelling between two ports.

		]], },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print(table.concat(args:help(),"\n"))
		return
	end
	
	local m=wmidi.create("dumpcake")

	local pi=m:port_create("dump",{"READ","SUBS_READ","DUPLEX","WRITE","SUBS_WRITE"},{"MIDI_GENERIC","SOFTWARE","PORT"})
	m:scan()
	pi=m.ports[ m.client..":"..pi ]
	
	if args.data.into and args.data.into~="" then

		local client,port=m:string_to_clientport(args.data.into)
		assert(client)
		port=port or 0

		local v={
			source_client=pi.client,
			source_port=pi.port,
			dest_client=client,
			dest_port=port,
		}
		print((" Connecting %3d:%-2d %3d:%-2d"):format(
			v.source_client,	v.source_port,
			v.dest_client,		v.dest_port))
		m:subscribe(v)
	end
	
	
	if args.data.from and args.data.from~="" then

		local client,port=m:string_to_clientport(args.data.from)
		assert(client)
		port=port or 0

		local v={
			source_client=client,
			source_port=port,
			dest_client=pi.client,
			dest_port=pi.port,
		}
		print((" Connecting %3d:%-2d %3d:%-2d"):format(
			v.source_client,	v.source_port,
			v.dest_client,		v.dest_port))
		m:subscribe(v)

	elseif args.data.all then

-- reset any subscriptions
		for n,v in pairs(m.subscriptions) do
			if v.source_client==pi.client or v.dest_client==pi.client then

				print((" Disconnecting %3d:%-2d %3d:%-2d"):format(
					v.source_client,	v.source_port,
					v.dest_client,		v.dest_port))

				m:unsubscribe{
					source_client=v.source_client,
					source_port=v.source_port,
					dest_client=v.dest_client,
					dest_port=v.dest_port,
				}
			end
		end

	-- subscribe to eveything	
		for n,v in pairs(m.ports) do
		
			if v.READ and v.SUBS_READ and v.client~=pi.client then

				local it={
					source_client=v.client,
					source_port=v.port,
					dest_client=pi.client,
					dest_port=pi.port,
				}
					print((" Connecting %3d:%-2d %3d:%-2d"):format(
					it.source_client,	it.source_port,
					it.dest_client,		it.dest_port))

				m:subscribe(it)
			end
		
		end

	end
	
	
print()
print("Waiting for events on "..pi.client..":"..pi.port.." press CTRL+C to exit.")
print()

	repeat
		local done=false

		local it=m:pull()
		
		if it then

			print( m:event_to_string(it) )

-- and output the event

			it.source="0:"..pi.port
			it.dest=nil

			m:push(it)

		end
	
	until done

elseif cmd=="tweak" then

	local args=require("cmd.args").bake({inputs=default_inputs{

		{	1,			arg[0].." "..cmd.." SCRIPT",	[[

Create a tweak port using the given script.

		]], },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print(table.concat(args:help(),"\n"))
		return
	end
	
	local tweaks={}
	local joins={}

-- load all supplied scripts
	for i=1,#arg do
		local fp=assert(io.open(arg[i],"rb"))
		local s=assert(fp:read("*a"))
		fp:close()
		
		local it=wsandbox.ini(s,{print=print,ls=ls})
		if it and it.tweaks then
			for _,v in ipairs(it.joins or {}) do
				joins[#joins+1]=v
			end
			for _,v in ipairs(it.tweaks or {}) do
				tweaks[#tweaks+1]=v
				print( "Loaded "..tostring(v.name).." from "..arg[i])
			end
			print( "Loaded "..(#it.tweaks).." tweaks from "..arg[i])
		else
			print( "No tweaks found in "..arg[i])
		end
	end
--ls(tweaks)

	local m=wmidi.create("tweakcake")
	
	local tweakports={}
	for i,v in ipairs(tweaks) do
		local id=assert( m:port_create(v.name or "tweak"..i,{"READ","SUBS_READ","DUPLEX","WRITE","SUBS_WRITE"},{"MIDI_GENERIC","SOFTWARE","PORT"}) )
		tweakports[id]=v
	end
	m:scan() -- update info about these new ports

print()
print("Performing port joins")
print()
for i,arg in ipairs(joins) do
	arg[0]=""
	process_command("join",arg)
end

print()
print("Waiting for events on ")
for n,v in pairs(tweakports) do
	local p=m.client..":"..n
	local port=m.ports[p]
	print( "\t"..m.client..":"..n.."\t"..port.path)
end
print("press CTRL+C to exit.")
print()

	repeat
		local done=false

		local it=m:pull()
		
		if it then
		
			local client,port=it.dest:match("(%d+):(%d+)")
			local tweak=tweakports[ tonumber(port) ]

			if tweak then
			
				local dest=it.dest -- remember dest as that is where we will broadcast from later
			
-- return the same event or a new event or nil to block this event
-- we will sort out the source and dest so it re broadcasts
				it=tweak.event(m,it)
-- you could alsp spit out multiple events inside the function with m:push() and just return a nil here
-- to say it has been deal with

-- and output the event if it still exists
				if it then
					it.source=dest
					it.dest=nil -- broadcast
					m:push(it)
				end

			end

		end
	
	until done

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	return

end

end
process_command(cmd,arg)
