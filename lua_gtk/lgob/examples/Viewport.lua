#! /usr/bin/env lua

require("lgob.gtk")

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
window:connect("delete-event", gtk.main_quit)

local box = gtk.VBox.new(false, 5)

for i = 1, 100 do
	box:add(gtk.Label.new("Label " .. i))
end

local scroll = gtk.ScrolledWindow.new()

scroll:add_with_viewport(box)
window:add(scroll)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "Viewport demo",
	"default-height", 200, "default-width", 100)
window:show_all()

gtk.main()
