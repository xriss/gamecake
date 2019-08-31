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
	core.scan(m[0],t)
	t.ports={}
	t.subscriptions={}
	for _,c in ipairs(t.clients) do
		for _,p in ipairs(c.ports) do

			for _,s in ipairs(p.subscriptions) do
				t.subscriptions[ s.source_client..":"..s.source_port.." -> "..s.dest_client..":"..s.dest_port]=s
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

	return m
end

--[[#lua.wetgenes.midi.get

	m:get()

get all values for this connection and store thme in m


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
sucess.

]]
base.port_destroy=function(m,p)
	return core.port_destroy(m[0],p)
end

-- do the reverse of ravel
local unravel=function(e)

	local o={}

	return e
end

-- make the events a little bit more suscinct
local ravel=function(e)

	local event_type=midi.SND_SEQ_EVENT[e.type]
	
	local o={}
	o.type=event_type
	o.source=e.source_client..":"..e.source_port
	o.dest=e.dest_client..":"..e.dest_port
	
	local fill_note=function()
		local channel=e[1]
		local note=e[2]
		local velocity=e[3]
		o.channel=channel
		o.note=note
		o.velocity=velocity
	end
	

	if     event_type == "PORT_SUBSCRIBED"   then

		return o

	elseif event_type == "PORT_UNSUBSCRIBED" then

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



