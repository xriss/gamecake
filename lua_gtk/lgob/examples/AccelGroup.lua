#! /usr/bin/env lua

-- AccelGroup example

require("lgob.gtk")

local window = gtk.Window.new()
local label = gtk.Label.new("Type Ctrl + q to exit!")
local accel  = gtk.AccelGroup.new()
accel:connect("<Ctrl>q", gtk.main_quit)
window:add(label)
window:add_accel_group(accel)
window:set("title", "Hello AccelGroup", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
