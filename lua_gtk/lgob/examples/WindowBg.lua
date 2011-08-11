#! /usr/bin/env lua

require("lgob.gdk")
require("lgob.gtk")

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
local img = gdk.Pixbuf.new_from_file("tux.png")
local pix, bit = img:render_pixmap_and_mask(255)

window:set("window-position", gtk.WIN_POS_CENTER, "title", "BG demo",
    "default-width", 360, "default-height", 424)
window:realize()

local gw = window:get_window()
gw:set_back_pixmap(pix, false)

window:connect("delete-event", gtk.main_quit)
window:connect("expose-event", gtk['true'])

window:show_all()
gtk.main()
