#! /usr/bin/env lua

-- Moving window (very annoying)!

require("lgob.gtk")
require("lgob.gdk")

local window = gtk.Window.new()
window:set("window-position", gtk.WIN_POS_CENTER, "title", "Moving window")
window:connect("delete-event", gtk.main_quit)
window:show_all()

local w, h = gdk.screen_get_default():get_size()
local ws, hs = window:get_size()
w = w - ws 
h = h - hs

local inc = 10
local x, y = 0, 0

glib.timeout_add(glib.PRIORITY_DEFAULT, 100, 
	function()
		window:move(x, y)
		x = x + inc
		y = y + inc
		
		if x > w then x = 0 end
		if y > h then y = 0 end
		
		return true
	end
)

gtk.main()
