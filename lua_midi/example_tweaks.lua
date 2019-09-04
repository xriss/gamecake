

tweaks={

	{
		name="testing",
		from="Midi Through:Midi Through Port-0", -- explicit device and port name
		event=function(m,e)
		
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

-- print it (this might slow down event processing)
			print( m:event_to_string(e) )

			return e
		end
	},
	
}
