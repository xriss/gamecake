#! /usr/bin/env lua

-- HPanned

require("lgob.gtk")

local window = gtk.Window.new()
local paned1  = gtk.HPaned.new()
local paned2  = gtk.VPaned.new()
local l1, l2, l3 = gtk.Label.new("Label 1"), gtk.Label.new("Label 2"),
	gtk.Label.new("Label 3")

l1:set("width-request", 200, "height-request", 200)
l2:set("width-request", 200, "height-request", 200)
l3:set("width-request", 200, "height-request", 200)

-- For child packing, use child{Set,Get} after adding the widgets
paned1:add(l1, l2)
paned2:add(paned1, l3)

window:add(paned2)
window:set("title", "Hello Paned", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
