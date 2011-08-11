#! /usr/bin/env lua

---
-- Simple music player. Usage example: ./GstPlayer.lua file:///home/myuser/myfile.ogg

require('lgob.gst')

-- User passed the music to play?
if not arg[1] then
	print(string.format("Usage: %s %s media_uri", arg[-1], arg[0]))
	os.exit(1)
end

-- Create the objects
local pipe = gst.ElementFactory.make('playbin', 'player')
local bus = pipe:get_bus()
local main = glib.MainLoop.new()

-- Configure them
pipe:set('uri', arg[1])

---
-- Watch callback
function watch_eos(userData, msg)
	-- Reached the end of file
	pipe:set_state(gst.STATE_NULL)
	main:quit()
end

---
-- Tag callback
function watch_tag(data, msg, oi)
	-- Push all the received tags into tbl. The tags usually doesn't
	-- come in the same callback.
	local tbl = {}
	gst.Message.parse_tag(msg, tbl)
	
	for i,j in pairs(tbl) do
		print(i,j)
	end
end

bus:add_signal_watch()
bus:connect('message::tag', watch_tag)
bus:connect('message::eos', watch_eos)
pipe:set_state(gst.STATE_PLAYING)

-- Display the current position and the total duration
glib.timeout_add(glib.PRIORITY_DEFAULT, 1000, 
	function()
		local r, t, p1 = pipe:query_position(gst.FORMAT_TIME)
		local r, t, p2 = pipe:query_duration(gst.FORMAT_TIME)
		io.write(string.format("Progress: %s of %s\r", p1, p2))
		io.flush()
		return true
	end
)

-- Run the main loop
main:run()

-- Clean
pipe:set_state(gst.STATE_NULL)
pipe:unref()
