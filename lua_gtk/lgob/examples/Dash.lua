#! /usr/bin/env lua

-- Hello World

require("lgob.gtk")
require("lgob.gdk")
require("lgob.cairo")

function expose(widget)
	local cr = gdk.cairo_create(widget:get_window())
	local dashes = {1, 2, 4, 8, 16}
	local w, h = widget:get_size()
	
	cr:set_source_rgb(0, 0, 1)
	cr:set_dash(dashes, 5, 10)
	cr:set_line_width(1.5)
	
	cr:move_to(0, 0)
	cr:line_to(w, h)
	cr:stroke()
	
	cr:set_dash(dashes, 3, 10)
	cr:move_to(w, 0)
	cr:line_to(0, h)
	cr:set_source_rgb(1, 0, 0)
	cr:stroke()
	
	cr:destroy()
end

local window = gtk.Window.new()
window:set("title", "Hello World", "window-position", gtk.WIN_POS_CENTER,
"app-paintable", true)

window:connect("delete-event", gtk.main_quit)
window:connect("expose-event", expose, window)
window:connect("size-allocate", gtk.Widget.queue_draw, window, true)

window:show_all()
gtk.main()
