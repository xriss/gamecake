#! /usr/bin/env lua

-- Bows & Arrows

require("lgob.gtk")

local window = gtk.Window.new()
local box = gtk.HBox.new(true, 5)

function buildArrow(...)
	for i, dir in pairs({...}) do
		local button = gtk.Button.new()
		button:add(gtk.Arrow.new(dir, gtk.SHADOW_IN))
		box:add(button)
		button:connect("clicked", function() print("Dir: ", dir) end)
	end
end

buildArrow(gtk.ARROW_LEFT, gtk.ARROW_UP, gtk.ARROW_RIGHT, gtk.ARROW_DOWN)

window:add(box)	
window:set("title", "Arrows!", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
