#! /usr/bin/env lua

-- Cairo toy text example

require("lgob.gtk")
require("lgob.gdk")
require("lgob.cairo")

function expose(widget)
	local cr = gdk.cairo_create(widget:get_window())
	local width, height = widget:get_size()
	
	cr:select_font_face("sans", cairo.FONT_SLANT_OBLIQUE, cairo.FONT_WEIGHT_BOLD)
	cr:set_font_size(14)
	cr:move_to(20, 20)
	cr:show_text("Hello! This is an example of the cairo toy text API")
	cr:move_to(20, 40)
	cr:show_text("For a serious work, use pango to handle text!")
	
	cr:destroy()
end

local window = gtk.Window.new()
window:set("title", "Cairo Toy Text", "window-position", gtk.WIN_POS_CENTER,
	"app-paintable", true, "width-request", 450, "height-request", 50)

window:connect("delete-event", gtk.main_quit)
window:connect("expose-event", expose, window)
window:connect("size-allocate", gtk.Widget.queue_draw, window, true)

window:show_all()
gtk.main()
