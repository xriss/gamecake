#! /usr/bin/env lua

require("lgob.clutter")
clutter.init_once()

function clicked_cb(data, event)
	local x, y = clutter.Event.get_coords(event)
	print(string.format("Stage clicked at (%d, %d)", x, y))
	
	return true
end

local stage = clutter.stage_get_default()
stage:set_size(200, 200)
stage:set_color(clutter.color_from_string("black"))
stage:show()
stage:connect("button-press-event", clicked_cb)

clutter.main()
