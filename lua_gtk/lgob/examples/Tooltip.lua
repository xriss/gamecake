#! /usr/bin/env lua

--[[
	Tooltip example.
--]]

require("lgob.gtk")

function on_tooltip(data, x, y, key, tool)
	tool:set_markup("<b>Hello World!</b> (nice)")
	tool:set_icon_from_stock("gtk-about", gtk.ICON_SIZE_BUTTON)
	
	return true
end

local window = gtk.Window.new()
local button = gtk.Button.new_with_mnemonic("This button have a tooltip!")
button:set("has-tooltip", true)
button:connect("query-tooltip", on_tooltip)

window:add(button)
window:set("title", "Hello Tooltip", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
