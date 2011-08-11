#! /usr/bin/env lua

-- SpinButton

require("lgob.gtk")

local window = gtk.Window.new()
local adj = gtk.Adjustment.new(0, -10, 10, 1.334, 3, 0)
local spin   = gtk.SpinButton.new(adj, 1, 3)

window:add(spin)
window:set("title", "Hello Spin", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
