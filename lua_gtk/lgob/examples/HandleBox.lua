#! /usr/bin/env lua

-- HandleBox example

require("lgob.gtk")

local window = gtk.Window.new()
local vbox1	 = gtk.VBox.new(false, 5)
local vbox2  = gtk.VBox.new(false, 5)
local handle = gtk.HandleBox.new()
local l1, l2 = gtk.Label.new("Label 1"), gtk.Label.new("Label 2")
local button = gtk.Button.new_with_mnemonic("_Exit")
button:connect("clicked", gtk.main_quit)

vbox1:add(l1, l2)
handle:add(vbox1)
vbox2:add(handle, button)
window:add(vbox2)
window:set("title", "Hello HandleBox", "window-position", gtk.WIN_POS_CENTER,
	"width-request", 200, "height-request", 200)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
