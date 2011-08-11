#! /usr/bin/env lua

-- Expander

require("lgob.gtk")

local window 	= gtk.Window.new()
local expander 	= gtk.Expander.new_with_mnemonic("_Expander")
local label		= gtk.Label.new("Hidden label")

expander:add(label)
window:add(expander)
window:set("title", "Hello Expander", "window-position", gtk.WIN_POS_CENTER,
	"resizable", false)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
