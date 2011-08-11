#! /usr/bin/env lua

require("lgob.gtk")

local window 	= gtk.Window.new()
local note 		= gtk.Notebook.new()

note:insert_page(gtk.TextView.new(), gtk.Label.new("A cool big page 2"), -1)
note:insert_page(gtk.TextView.new(), gtk.Label.new("A cool big ape 3"), -1)
note:insert_page(gtk.TextView.new(), gtk.Label.new("A cool big apple pie 4"), -1)
note:insert_page(gtk.TextView.new(), gtk.Label.new("A cool big page 1"), 0)
note:set("enable-popup", true, "scrollable", true)

window:add(note)

window:connect("delete-event", gtk.main_quit)
window:set("width-request", 350, "height-request", 300)
window:set("title", "Notebook demo", "window-position", gtk.WIN_POS_CENTER)
window:show_all()
gtk.main()
