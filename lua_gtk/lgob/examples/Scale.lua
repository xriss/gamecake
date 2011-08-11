#! /usr/bin/env lua

-- Scale widgets

-- Importing the library
require("lgob.gtk")

-- Create the widgets
local window = gtk.Window.new()
local scale1 = gtk.HScale.new_with_range(1, 100, 10)
local adjust = gtk.Adjustment.new(1, 1, 100, 2, 10, 0)
local scale2 = gtk.HScale.new(adjust)
local vbox   = gtk.VBox.new(true, 5)

vbox:add(scale1)
vbox:add(scale2)
window:add(vbox)

-- Syntactic sugar for setting more than one property
window:set("title", "Hello World", "window-position", gtk.WIN_POS_CENTER)
window:set("width-request", 200)

-- Connect the callback
window:connect("delete-event", gtk.main_quit)

-- Show 'window' and all the widgets in 'window'
window:show_all()

-- Start the main loop
gtk.main()
