
local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wgrd=require("wetgenes.grd")

local doublewrap=require("cmd.args").doublewrap
local splitpath=require("cmd.args").splitpath

local wmidi=require("wetgenes.midi")

local cmds={
	{ "list",		"List available midi connections."},
	{ "dump",		"Listen for and print events."},
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

if cmd=="list" then

	local args=require("cmd.args").bake({inputs=default_inputs{

--		{	"in",		false,	"Input chanels only.", },
--		{	"out",		false,	"Output chanels only.", },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print("\n"..arg[0].." list --OPTIONS \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	local m=wmidi.create("gamecake-midi")

--	local pi=m:port_create("scan",{"READ","SUBS_READ","WRITE","SUBS_WRITE"},{"MIDI_GENERIC","SOFTWARE","PORT"})

--ls(wmidi.notes)

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
		local s=string.format("%"..maxn.."s %3d",v.name:sub(-maxn),v.client)
		print( s )
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
		local asc,asp,adc,adp=a:match("(%d+):(%d+) %-> (%d+):(%d+)")
		local bsc,bsp,adc,adp=b:match("(%d+):(%d+) %-> (%d+):(%d+)")
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

		local count=0
		for _,c in pairs(sv) do count=count+1 end
		if count>0 then
			local s="  "..name.."  ->"
			local em="-|"
			local di=0
			for _,dn in ipairs(slinesort) do
				if soutput[dn] then
					if di>=count then em=" |" end
					if sv[dn] then
						s=s.."-+"
						di=di+1
						uplink[dn]=true
					else
						if uplink[dn] then
							s=s..em
						else
							s=s.."  "
						end
					end
				end
			end
			s=s.."    "..blank.."  "
			print( s )
		end
	end
	
	for si=#slinesort,1,-1 do local sn=slinesort[si] local sv=slines[sn]
		if soutput[sn] then
			local sc,sp=sn:match("(%d+):(%d+)")
			local name=string.format("%3d:%-2d",sc,sp)
			local blank="      "

			local s="  "..blank.."    "
			local em=" |"
			for _,dn in ipairs(slinesort) do
				if soutput[dn] then
					if dn==sn then
					s=s.." +"
					em="--"
					else
					s=s..em
					end
				end
			end
			s=s.."->  "..name.."  "
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

--		{	"in",		false,	"Input chanels only.", },
--		{	"out",		false,	"Output chanels only.", },

	}}):parse(arg):sanity()
	
	if args.data.help then
		print("\n"..arg[0].." dump --OPTIONS \n")
		print( "where --OPTIONS is any combination of the following :\n")
		print(table.concat(args:help(),"\n"))
		print("")
		os.exit(0)
	end
	
	local m=wmidi.create("gamecake-midi")

	local pi=m:port_create("dump",{"WRITE","SUBS_WRITE"},{"MIDI_GENERIC","SOFTWARE","PORT"})
	m:scan()
	pi=m.ports[ m.client..":"..pi ]
	

ls(m.subscriptions)
-- reset any subscriptions
	for n,v in pairs(m.subscriptions) do
		if v.source_client==pi.client or v.dest_client==pi.client then

print("unsubscribing from "..n)

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

print("subscribing to "..n)

			m:subscribe{
				source_client=v.client,
				source_port=v.port,
				dest_client=pi.client,
				dest_port=pi.port,
			}
		end
	
	end

print()
print("Waiting for events CTRL+C to exit")
print()

	repeat
		local done=false

		local it=m:pull()
		ls(it)
	
	until done

else -- print help

	print("\n"..arg[0].." COMMAND ".."...\n")
	print("Where COMMAND is one of the following :\n")
	print("  "..table.concat(doublewrap(cmds,78,30," : "),"\n  ").."\n")
	os.exit(0)

end
