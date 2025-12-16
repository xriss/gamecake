
--[[

This should be run as tweak script, something like this.

	gamecake.midi tweak lua_midi/example_tweaks.lua

]]

-- after setting up the tweaks we want to try and join these ports
-- our clientname is tweakcake and the portname is the name used in the tweak
joins={
	{"Midi Through:Midi Through Port-0","tweakcake:testing"},
}


tweaks={

	{
-- name of tweak
		name="testing",
		
		event=function(m,e)

-- if you want to see what each event looks like then uncomment the next line
-- ls(e)

			if e.type=="NOTE" or e.type=="NOTEON" or e.type=="NOTEOFF" then

-- only deal with non zero velocity
				if e.velocity>0 then

-- twiddle the note velocity
-- crank it up from 1-127 to 65-127
-- so it will never be below 50% no matter how soft you press the key

					e.velocity=math.ceil(e.velocity/2)+64

				end

-- clamp velocity
				if e.velocity<=0   then e.velocity=0 end
				if e.velocity>=127 then e.velocity=127 end

-- make sure it is the right event
				if e.velocity==0 then
					e.type="NOTEOFF"
				else
					e.type="NOTEON"
				end
			
			end

-- print it (this might slow down event processing so if you notice latency then stop doing this)
			print( m:event_to_string(e) )

-- return event to be broadcast
			return e
--[[

Alternatively rather than returning an event to be broadcast we could 
broadcast it ourselves and return nil, this way we can easily turn one 
event into many.

			e.source=e.dest		-- move the dest to source
			e.dest=nil			-- and set the dest to nil
			m:push(e)			-- will broadcast to any listeners
]]
		end
	},
	
}
