
require("apps").default_paths()


local wstr=require("wetgenes.string")
local ls=function(s) print(wstr.dump(s)) end

local wmidi=require("wetgenes.midi")

local m=wmidi.create("tweaker")

local pi=m:port_create("tweak",{"READ","SUBS_READ","DUPLEX","WRITE","SUBS_WRITE"},{"MIDI_GENERIC","SOFTWARE","PORT"})

repeat
	local done=false

	local it=m:pull()

	if it then

		if it.type=="NOTE" or it.type=="NOTEON" or it.type=="NOTEOFF" then
		
			if it.velocity==0 then
				it.type="NOTEOFF"
			else
				it.type="NOTEON"
				it.velocity=math.ceil(it.velocity/2)
			end
		
		end


		it.source="0:"..pi
		it.dest=nil

		m:push(it)

	end

until done
