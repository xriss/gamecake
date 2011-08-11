#! /usr/bin/env lua

-- Effects on a image, from http://zetcode.com/tutorials/cairographicstutorial/cairoimages/

require("lgob.gtk")
require("lgob.gdk")
require("lgob.cairo")

local window = gtk.Window.new()
local image  = cairo.ImageSurface.create_from_png("tux.png")
local stop = false
local w, h = 0, 0

function expose(widget)
	local cr = gdk.cairo_create(widget:get_window())
	local iWidth, iHeight = image:get_width(), image:get_height()
	local surface = cairo.ImageSurface.create(cairo.FORMAT_ARGB32, iWidth, iHeight)
	local ic	  = cairo.Context.create(surface)
	ic:rectangle(0, 0, iWidth, h)
	ic:fill()

	h = h + 1

	if( h >= iHeight) then stop = true end

	cr:set_source_surface(image, 10, 10)
	cr:mask_surface(surface, 10, 10)
	surface:destroy()
	cr:destroy()
	ic:destroy()

  	return false
end

function update(window)
	if stop then
		window:set("title", "Done!")
		return false
	end

	window:queue_draw()
	return true
end

window:set("title", "Loading!", "window-position", gtk.WIN_POS_CENTER,
"app-paintable", true)

window:connect("delete-event", gtk.main_quit)
window:connect("expose-event", expose, window)
glib.timeout_add(glib.PRIORITY_DEFAULT_IDLE, 100, update, window)

window:show_all()
gtk.main()
