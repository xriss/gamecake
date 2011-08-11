#! /usr/bin/env lua

-- Calendar

require("lgob.gtk")

local window = gtk.Window.new()
local vbox 	 = gtk.VBox.new(false, 5)
local cal	 = gtk.Calendar.new()
local button = gtk.Button.new_with_mnemonic("_Get Date!")

function get_date()
	print(cal:get("day", "month", "year"))
	cal:mark_day(cal:get("day"))
end

vbox:add(cal)
vbox:add(button)
window:add(vbox)

window:set("title", "Calendar", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)
button:connect("clicked", get_date)

window:show_all()
gtk.main()
