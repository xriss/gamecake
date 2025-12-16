--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end


local core=require("midi_alsa_core")


local midi=M
local base={}
local meta={}
meta.__index=base

setmetatable(midi,meta)

midi.SND_SEQ_PORT_CAP={}
midi.SND_SEQ_PORT_CAP.READ=(2^0)
midi.SND_SEQ_PORT_CAP.WRITE=(2^1)
midi.SND_SEQ_PORT_CAP.SYNC_READ=(2^2)
midi.SND_SEQ_PORT_CAP.SYNC_WRITE=(2^3)
midi.SND_SEQ_PORT_CAP.DUPLEX=(2^4)
midi.SND_SEQ_PORT_CAP.SUBS_READ=(2^5)
midi.SND_SEQ_PORT_CAP.SUBS_WRITE=(2^6)
midi.SND_SEQ_PORT_CAP.NO_EXPORT=(2^7)
 
midi.SND_SEQ_PORT_TYPE={}
midi.SND_SEQ_PORT_TYPE.SPECIFIC=(2^0)
midi.SND_SEQ_PORT_TYPE.MIDI_GENERIC=(2^1)
midi.SND_SEQ_PORT_TYPE.MIDI_GM=(2^2)
midi.SND_SEQ_PORT_TYPE.MIDI_GS=(2^3)
midi.SND_SEQ_PORT_TYPE.MIDI_XG=(2^4)
midi.SND_SEQ_PORT_TYPE.MIDI_MT32=(2^5)
midi.SND_SEQ_PORT_TYPE.MIDI_GM2=(2^6)
midi.SND_SEQ_PORT_TYPE.SYNTH=(2^10)
midi.SND_SEQ_PORT_TYPE.DIRECT_SAMPLE=(2^11)
midi.SND_SEQ_PORT_TYPE.SAMPLE=(2^12)
midi.SND_SEQ_PORT_TYPE.HARDWARE=(2^16)
midi.SND_SEQ_PORT_TYPE.SOFTWARE=(2^17)
midi.SND_SEQ_PORT_TYPE.SYNTHESIZER=(2^18)
midi.SND_SEQ_PORT_TYPE.PORT=(2^19)
midi.SND_SEQ_PORT_TYPE.APPLICATION=(2^20)

midi.SND_SEQ_EVENT={}
midi.SND_SEQ_EVENT.SYSTEM = 0
midi.SND_SEQ_EVENT.RESULT = 1
midi.SND_SEQ_EVENT.NOTE = 5
midi.SND_SEQ_EVENT.NOTEON = 6
midi.SND_SEQ_EVENT.NOTEOFF = 7
midi.SND_SEQ_EVENT.KEYPRESS = 8
midi.SND_SEQ_EVENT.CONTROLLER = 10
midi.SND_SEQ_EVENT.PGMCHANGE = 11
midi.SND_SEQ_EVENT.CHANPRESS = 12
midi.SND_SEQ_EVENT.PITCHBEND = 13
midi.SND_SEQ_EVENT.CONTROL14 = 14
midi.SND_SEQ_EVENT.NONREGPARAM = 15
midi.SND_SEQ_EVENT.REGPARAM = 16
midi.SND_SEQ_EVENT.SONGPOS = 20
midi.SND_SEQ_EVENT.SONGSEL = 21
midi.SND_SEQ_EVENT.QFRAME = 22
midi.SND_SEQ_EVENT.TIMESIGN = 23
midi.SND_SEQ_EVENT.KEYSIGN = 24
midi.SND_SEQ_EVENT.START = 30
midi.SND_SEQ_EVENT.CONTINUE = 31
midi.SND_SEQ_EVENT.STOP = 32
midi.SND_SEQ_EVENT.SETPOS_TICK = 33
midi.SND_SEQ_EVENT.SETPOS_TIME = 34
midi.SND_SEQ_EVENT.TEMPO = 35
midi.SND_SEQ_EVENT.CLOCK = 36
midi.SND_SEQ_EVENT.TICK = 37
midi.SND_SEQ_EVENT.QUEUE_SKEW = 38
midi.SND_SEQ_EVENT.SYNC_POS = 39
midi.SND_SEQ_EVENT.TUNE_REQUEST = 40
midi.SND_SEQ_EVENT.RESET = 41
midi.SND_SEQ_EVENT.SENSING = 42
midi.SND_SEQ_EVENT.ECHO = 50
midi.SND_SEQ_EVENT.OSS = 51
midi.SND_SEQ_EVENT.CLIENT_START = 60
midi.SND_SEQ_EVENT.CLIENT_EXIT = 61
midi.SND_SEQ_EVENT.CLIENT_CHANGE = 62
midi.SND_SEQ_EVENT.PORT_START = 63
midi.SND_SEQ_EVENT.PORT_EXIT = 64
midi.SND_SEQ_EVENT.PORT_CHANGE = 65
midi.SND_SEQ_EVENT.PORT_SUBSCRIBED = 66
midi.SND_SEQ_EVENT.PORT_UNSUBSCRIBED = 67
midi.SND_SEQ_EVENT.USR0 = 90
midi.SND_SEQ_EVENT.USR1 = 91
midi.SND_SEQ_EVENT.USR2 = 92
midi.SND_SEQ_EVENT.USR3 = 93
midi.SND_SEQ_EVENT.USR4 = 94
midi.SND_SEQ_EVENT.USR5 = 95
midi.SND_SEQ_EVENT.USR6 = 96
midi.SND_SEQ_EVENT.USR7 = 97
midi.SND_SEQ_EVENT.USR8 = 98
midi.SND_SEQ_EVENT.USR9 = 99
midi.SND_SEQ_EVENT.SYSEX = 130
midi.SND_SEQ_EVENT.BOUNCE = 131
midi.SND_SEQ_EVENT.USR_VAR0 = 135
midi.SND_SEQ_EVENT.USR_VAR1 = 136
midi.SND_SEQ_EVENT.USR_VAR2 = 137
midi.SND_SEQ_EVENT.USR_VAR3 = 138
midi.SND_SEQ_EVENT.USR_VAR4 = 139
midi.SND_SEQ_EVENT.NONE = 255
  
  

-- setup number to string and string to number
for _,t in ipairs{
		midi.SND_SEQ_PORT_CAP ,
		midi.SND_SEQ_PORT_TYPE ,
		midi.SND_SEQ_EVENT ,
	} do
	local tmp={}
	for n,v in pairs(t) do
		if type(n)=="string" and type(v)=="number" then
			tmp[n]=v
		end
	end
	for n,v in pairs(tmp) do
		t[v]=n
	end
end


-- possible ascii notations for each octave
midi.note_names_hash= { "C" ,"C#","D" ,"D#","E" ,"F" ,"F#","G" ,"G#","A" ,"A#","B" }
midi.note_names_dash= { "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"}
midi.note_names_s=    { "C" ,"Cs","D" ,"Ds","E" ,"F" ,"Fs","G" ,"Gs","A" ,"As","B" }
midi.note_names_lower={ "C" ,"c" ,"D" ,"d" ,"E" ,"F" ,"f" ,"G" ,"g" ,"A" ,"a" ,"B" }

midi.notes={}
for i=0,127 do
	midi.notes[i]=midi.note_names_hash[(i%12)+1]..(math.floor(i/12)-1)
	midi.notes[ midi.note_names_hash[(i%12)+1]..(math.floor(i/12)-1) ]=i
	midi.notes[ midi.note_names_dash[(i%12)+1]..(math.floor(i/12)-1) ]=i
	midi.notes[ midi.note_names_s[(i%12)+1]..(math.floor(i/12)-1) ]=i
	midi.notes[ midi.note_names_lower[(i%12)+1]..(math.floor(i/12)-1) ]=i
end



--[[#lua.wetgenes.midi.create

	m=wmidi.create()

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a midi object.

]]
-- many options
midi.create=function(name)
	local m={}
	setmetatable(m,meta)
	m[0]=core.create()
	
	m.name=name
	m:set()

	m:get()
	
	return m
end

--[[#lua.wetgenes.midi.destroy

	m:destroy()

Free the midi and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.

]]
base.destroy=function(m)
	return core.destroy(m[0])
end

--[[#lua.wetgenes.midi.clients

	m:clients()

fetch table of clients


]]
base.scan=function(m,t)
	t=t or m
	local it=core.scan(m[0],{})
	t.clients={}
	t.ports={}
	t.subscriptions={}
	for _,c in ipairs(it.clients) do
		t.clients[c.client]=c
		for _,p in ipairs(c.ports) do

			for _,s in ipairs(p.subscriptions) do
				t.subscriptions[ s.source_client..":"..s.source_port.." > "..s.dest_client..":"..s.dest_port]=s
			end
--			p.subscriptions=nil -- remove from inside port
			
			t.ports[p.client..":"..p.port]=p

			for n,v in pairs(midi.SND_SEQ_PORT_TYPE) do
				if type(n)=="string" and type(v)=="number" then
					p[n]=nil
					if (math.floor(p.type/v)%2)==1 then
						p[n]=true
					end
				end
			end

			for n,v in pairs(midi.SND_SEQ_PORT_CAP) do
				if type(n)=="string" and type(v)=="number" then
					p[n]=nil
					if (math.floor(p.capability/v)%2)==1 then
						p[n]=true
					end
				end
			end

		end
--		c.ports=nil -- remove from inside client
	end
	
	for n,p in pairs(t.ports) do
		local c=t.clients[p.client]
		p.path=c.name..":"..p.name
	end
	

	return m
end

--[[#lua.wetgenes.midi.get

	m:get()

get all values for this connection and store them in m


]]
base.get=function(m)
	return core.get(m[0],m)
end

--[[#lua.wetgenes.midi.set

	m:set()

set all values for this connection from values found in m


]]
base.set=function(m)
	return core.set(m[0],m)
end

--[[#lua.wetgenes.midi.port_create

	p = m:port_create(name,caps,type)

Create a port with the given name and capability bits and type.

bits and type are either a number or a table of bitnames.

Returns the number of the port created which should be used in 
port_destroy or nil if something went wrong.

]]
base.port_create=function(m,n,c,t)
	if type(c)=="table" then
		local b=0
		for i,v in ipairs(c) do b=b+midi.SND_SEQ_PORT_CAP[v] end
		c=b
	end
	if type(t)=="table" then
		local b=0
		for i,v in ipairs(t) do b=b+midi.SND_SEQ_PORT_TYPE[v] end
		t=b
	end
	return core.port_create(m[0],n,c,t)
end

--[[#lua.wetgenes.midi.port_destroy

	m:port_destroy(num)

Destroy a previously created port. Returns nil on failure, true on 
success.

]]
base.port_destroy=function(m,p)
	return core.port_destroy(m[0],p)
end

-- do the reverse of ravel
local unravel=function(o)

	local e={}
	
	local event_type=o.type
	
	e.type=o.type
	if type(e.type)=="string" then
		e.type=midi.SND_SEQ_EVENT[e.type] -- convert to number
	else
		event_type=midi.SND_SEQ_EVENT[e.type] -- convert from number
	end
	
	if o.source then
		e.source_client,e.source_port=o.source:match("(%d+):(%d+)")
	else
		e.source_client=o.source_client or -1 -- this will get changed to our client id
		e.source_port=o.source_port
	end
	
	if o.dest then
		e.dest_client,e.dest_port=o.dest:match("(%d+):(%d+)")
	elseif not o.dest_client and not o.dest_port then -- default
		e.dest_client=254 -- broadcast to subscribers
		e.dest_port=0 -- this port is ignored
	else
		e.dest_client=o.dest_client
		e.dest_port=o.dest_port
	end

	local fill_note=function()
		e[1]=o.channel
		e[2]=o.note
		e[3]=o.velocity
	end

	if     event_type == "PORT_SUBSCRIBED"   then

		e[1],e[2]=o.sub_source:match("(%d+):(%d+)")
		e[3],e[4]=o.sub_dest:match("(%d+):(%d+)")
		e[1]=tonumber(e[1]) e[2]=tonumber(e[2])
		e[3]=tonumber(e[3]) e[4]=tonumber(e[4])
		return e

	elseif event_type == "PORT_UNSUBSCRIBED" then

		e[1],e[2]=o.sub_source:match("(%d+):(%d+)")
		e[3],e[4]=o.sub_dest:match("(%d+):(%d+)")
		e[1]=tonumber(e[1]) e[2]=tonumber(e[2])
		e[3]=tonumber(e[3]) e[4]=tonumber(e[4])
		return e

	elseif event_type == "PORT_START" then

		e[13]=o.client
		e[14]=o.dat2
		e[15]=o.dat3
		return e

	elseif event_type == "START" then

		e[13]=o.dat1
		e[14]=o.dat2
		e[15]=o.dat3
		return e

	elseif event_type == "TEMPO" then

		e[14]=o.tempo
		return e

	elseif event_type == "NOTE" then

		fill_note()
		return e

	elseif event_type == "NOTEON" then

		fill_note()
		return e

	elseif event_type == "NOTEOFF" then

		fill_note()
		return e
		
	elseif event_type == "PITCHBEND" then

		e[13]=o.channel
		e[15]=o.value
		if e[15]<0 then e[15]=e[15]+0x100000000 end
		return e

	elseif event_type == "CONTROLLER" then

		e[13]=o.channel
		e[14]=o.control
		e[15]=o.value
		return e

	elseif event_type == "PGMCHANGE" then

		e[13]=o.channel
		e[15]=o.program
		return e
		
	elseif event_type == "CLIENT_START" then

		e[13]=o.client
		return e
		
	elseif event_type == "CLIENT_EXIT" then

		e[13]=o.client
		return e
		
	else

		e[13]=o.dat1
		e[14]=o.dat2
		e[15]=o.dat3
		return e
		
	end

	return o
end

-- make the events a little bit more succinct
local ravel=function(e)

	local event_type=midi.SND_SEQ_EVENT[e.type]
	
	local o={}
	o.type=event_type
	o.source=e.source_client..":"..e.source_port
	o.dest=e.dest_client..":"..e.dest_port
	
	local fill_note=function()
		o.channel=e[1]
		o.note=e[2]
		o.velocity=e[3]
	end
	

	if     event_type == "PORT_SUBSCRIBED"   then

		o.sub_source=e[1]..":"..e[2]
		o.sub_dest=e[3]..":"..e[4]
		return o

	elseif event_type == "PORT_UNSUBSCRIBED" then

		o.sub_source=e[1]..":"..e[2]
		o.sub_dest=e[3]..":"..e[4]
		return o

	elseif event_type == "PORT_START" then

		o.client=e[13]
		o.dat2=e[14]
		o.dat3=e[15]
		return o

	elseif event_type == "START" then

		o.dat1=e[13]
		o.dat2=e[14]
		o.dat3=e[15]
		return o

	elseif event_type == "TEMPO" then

		o.tempo=e[14]
		return o

	elseif event_type == "NOTE" then

		fill_note()
		return o

	elseif event_type == "NOTEON" then

		fill_note()
		return o

	elseif event_type == "NOTEOFF" then

		fill_note()
		return o
		
	elseif event_type == "PITCHBEND" then

		o.channel=e[13]
		local v=e[15]
		if v>=0x80000000 then v=v-0x100000000 end
		o.value=v
		return o

	elseif event_type == "CONTROLLER" then

		o.channel=e[13]
		o.control=e[14]
		o.value=e[15]
		return o

	elseif event_type == "PGMCHANGE" then

		o.channel=e[13]
		o.program=e[15]
		return o
		
	elseif event_type == "CLIENT_START" then

		o.client=e[13]
		return o
		
	elseif event_type == "CLIENT_EXIT" then

		o.client=e[13]
		return o
		
	else

		o.dat1=e[13]
		o.dat2=e[14]
		o.dat3=e[15]
		return o
		
	end
	
	return e
end



--[[#lua.wetgenes.midi.push

	m:push(it)

Send an output midi event.

]]
base.push=function(m,it)
	it=unravel(it)
	return core.push(m[0],it)
end



--[[#lua.wetgenes.midi.pull

	m:pull(it)

Receive an input midi event, blocking until there is one.

Occasionally, for "reasons" this may return nil.

]]
base.pull=function(m)
	local e=core.pull(m[0])
	if e then e=ravel(e) end
	return e
end

--[[#lua.wetgenes.midi.peek

	m:peek(it)

Returns a event if there is one or null if none are currently 
available.

]]
base.peek=function(m)
	local e=core.peek(m[0])
	if e then e=ravel(e) end
	return e
end

--[[#lua.wetgenes.midi.subscribe

	m:subscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:subscribe{
		source="0:0",
		dest="1:0",
	}

Creates a persistent subscription between two ports.

]]
base.subscribe=function(m,it)
	if it.source then
		it.source_client,it.source_port=it.source:match("(%d+):(%d+)")
		it.source_client=tonumber(it.source_client)
		it.source_port=tonumber(it.source_port)
	end
	if it.dest then
		it.dest_client,it.dest_port=it.dest:match("(%d+):(%d+)")
		it.dest_client=tonumber(it.dest_client)
		it.dest_port=tonumber(it.dest_port)
	end
	return core.subscribe(m[0],it)
end

--[[#lua.wetgenes.midi.unsubscribe

	m:unsubscribe{
		source_client=0,	source_port=0,
		dest_client=1,		dest_port=0,
	}

	m:unsubscribe{
		source="0:0",
		dest="1:0",
	}

Removes a persistent subscription from between two ports.

]]
base.unsubscribe=function(m,it)
	if it.source then
		it.source_client,it.source_port=it.source:match("(%d+):(%d+)")
		it.source_client=tonumber(it.source_client)
		it.source_port=tonumber(it.source_port)
	end
	if it.dest then
		it.dest_client,it.dest_port=it.dest:match("(%d+):(%d+)")
		it.dest_client=tonumber(it.dest_client)
		it.dest_port=tonumber(it.dest_port)
	end
	return core.unsubscribe(m[0],it)
end

--[[#lua.wetgenes.midi.string_to_clientport

	client,port = m:string_to_clientport(str)

Convert a "client:port" string to two numbers client,port this can 
either be two decimal numbers or, if a m:scan() has been performed, 
then a partial case insensitive matching to the name of existing 
clients and ports may get a port number.

Will return a nil if we can not work out which client or port you mean.

]]
base.string_to_clientport=function(m,str)

	local client,port

	if str:find(":",1,true) then -- string contains a port
		client,port=str:match("(.*):(.*)")
	else -- string is just a client
		client=str
	end

-- check for explicit numbers
	if client and client~="" and ( tostring(tonumber(client) or 0) == client ) then
		client=tonumber(client)
	end

	if port and port~="" and ( tostring(tonumber(port) or 0) == port ) then
		port=tonumber(port)
	end
	
	if m.clients and m.ports then -- we can check for partial string matches

-- partial name match for a client
		
		for n,v in pairs( m.clients ) do
			if type(client)=="string" and client~="" then
				if v.name:lower():find(client:lower(),1,true) then
					client=v.client
				end
			end
		end

-- if we have a client then we can also search for a port
		
		for n,v in pairs( m.ports ) do
			if type(port)=="string" and ( type(client)=="number" or client=="" ) and port~="" then
				if v.client==client or client=="" then
					if v.name:lower():find(port:lower(),1,true) then
						client=v.client
						port=v.port
					end
				end
			end
		end

	end

-- if still a string then we failed to match a name

	if type(client)=="string" then client=nil end
	if type(port)=="string" then port=nil end

	return client,port
end



--[[#lua.wetgenes.midi.event_to_string

	str = m:event_to_string(event)

Convert an event to a single line string for printing to the console.

]]
base.event_to_string=function(m,it)

	local s

	local render_bar=function(w,nl,nh,n)
		local s=""
		local f=math.ceil(((n-nl)/(nh-nl))*w)
		for i=1,w do
			if i<=f then
				s=s.."|"
			else
				s=s.."-"
			end
		end
		return s
	end
	
	local get_d32=function()
		return string.format("%08x %08x %08x",it.dat1 or 0,it.dat2 or 0,it.dat3 or 0)
	end
	
	local get_type=function()
		return string.format("%17s",it.type)
	end

	local get_path=function()
		local sc,sp=it.source:match("(%d+):(%d+)")
		local dc,dp=it.dest:match("(%d+):(%d+)")
		return string.format("%3d:%-2d > %3d:%-2d",sc,sp,dc,dp)
	end

	local get_subpath=function()
		local sc,sp=it.sub_source:match("(%d+):(%d+)")
		local dc,dp=it.sub_dest:match("(%d+):(%d+)")
		return string.format("%3d:%-2d > %3d:%-2d",sc,sp,dc,dp)
	end

	local get_note=function()
		return string.format("%2d %3d %4s %s %3d",it.channel,it.note,midi.notes[it.note],render_bar(32,0,127,it.velocity),it.velocity)
	end

	local get_control=function()
		return string.format("%2d %3d %s %3d",it.channel,it.control,render_bar(32,0,127,it.value),it.value)
	end

	local get_bend=function()
		return string.format("%2d %s %5d",it.channel,render_bar(32,-8192,8912,it.value),it.value)
	end

	local get_program=function()
		return string.format("%2d %3d",it.channel,it.program)
	end
	

	if it then

		s=get_path().." "..get_type().." "

		if it.type=="PORT_SUBSCRIBED" then

			s=s..get_subpath()

		elseif it.type=="PORT_UNSUBSCRIBED" then

			s=s..get_subpath()

		elseif it.type=="NOTE" then

			s=s..get_note()

		elseif it.type=="NOTEON" then

			s=s..get_note()

		elseif it.type=="NOTEOFF" then

			s=s..get_note()

		elseif it.type == "PITCHBEND" then

			s=s..get_bend()

		elseif it.type == "CONTROLLER" then

			s=s..get_control()

		elseif it.type == "PGMCHANGE" then

			s=s..get_program()

		else

			s=s..get_d32()

		end

	end
	
	return s
end
