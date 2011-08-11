#! /usr/bin/env lua

require("lgob.cluttergtk")
cluttergtk.init_once()

function clicked_cb(data, event)
	local x, y = clutter.Event.get_coords(event)
	print(string.format("Stage clicked at (%d, %d)", x, y))
	
	return true
end

local win = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
local vbox = gtk.VBox.new(false, 5)
win:set("title", "ClutterGtk demo", "window-position", gtk.WIN_POS_CENTER)
win:connect("delete-event", gtk.main_quit)

local embed = cluttergtk.Embed.new()
embed:set("width-request", 200, "height-request", 200)
local btn = gtk.Button.new_with_mnemonic("_Quit")
btn:connect("clicked", gtk.main_quit)

vbox:add(embed, btn) 
win:add(vbox)

local stage = embed:get_stage()
stage:set_size(200, 200)
stage:set_color(clutter.color_from_string("black"))
stage:connect("button-press-event", clicked_cb)

win:show_all()

gtk.main()
