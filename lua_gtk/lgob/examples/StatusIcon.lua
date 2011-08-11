#! /usr/bin/env lua

-- StatusIcon example

require("lgob.gtk")

local window 	= gtk.Window.new()
local label 	= gtk.Label.new("Hello StatusIcon!")
local status 	= gtk.StatusIcon.new()
local menu		= gtk.Menu.new()
local i1, i2 	= gtk.ImageMenuItem.new_from_stock("gtk-open"), gtk.ImageMenuItem.new_from_stock("gtk-quit")
local visible	= false

menu:add(i1, i2)
menu:show_all()

function showHide()
	visible = not visible
	window:set("visible", visible)
end

function popup()
	menu:popup_from_status_icon(status)
end

label:show()
window:add(label)
window:set("title", "Hello StatusIcon", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

-- As any GTK application, using stock icons eats some memory
status:set("stock", "gtk-about", "visible", true)

i1:connect("activate", showHide)
i2:connect("activate", gtk.main_quit)
status:connect("activate", showHide)
status:connect("popup-menu", popup)

gtk.main()
