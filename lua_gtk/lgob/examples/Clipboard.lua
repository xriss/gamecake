#! /usr/bin/env lua

--[[
	Clipboard example.
--]]

require("lgob.gdk")
require("lgob.gtk")

-- Getting the shared clipboard
local clip = gtk.clipboard_get(gdk.Atom.intern("CLIPBOARD"))
local buff = gdk.Pixbuf.new_from_file("icon.png")

-- UI
local window = gtk.Window.new()

local button = gtk.Button.new_with_mnemonic("Click to test!") 
button:connect("clicked", 
	function()
		local t = "Text from " .. arg[0]
		clip:set_text(t, #t)
		print(clip:wait_for_text())
		clip:set_image(buff)
		print(clip:wait_for_image())
	end)

window:add(button)
window:set("window-position", gtk.WIN_POS_CENTER, "title", "Clipboard example")
window:connect("delete-event", gtk.main_quit)
window:show_all()

gtk.main()
