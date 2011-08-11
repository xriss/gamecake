#! /usr/bin/env lua

require("lgob.gdk")
require("lgob.cairo")
require("lgob.gtk")

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "Shape demo",
    "default-width", 400, "default-height", 400)
window:set("decorated", false)

window:realize()
local gw = window:get_window()

window:connect("delete-event", gtk.main_quit)
window:connect("expose-event", gtk['true'])
window:connect("size-allocate", function(data, alloc)
    local x, y, w, h = gdk.Rectangle.get(alloc)
    local bitmap = gdk.Pixmap.new(nil, w, h, 1)
    local cr = gdk.cairo_create(bitmap)

    cr:set_source_rgb(0.0, 0.0, 0.0)
    cr:set_operator(cairo.OPERATOR_CLEAR)
    cr:paint()
    
    cr:set_source_rgb(1.0, 1.0, 1.0)
    cr:set_operator(cairo.OPERATOR_SOURCE)
    cr:arc(w / 2, h / 2, math.min(h, w) / 2, 0, 2 * math.pi)
    cr:fill()

    gw:shape_combine_mask(bitmap, 0, 0)
end)

window:show_all()
gtk.main()
