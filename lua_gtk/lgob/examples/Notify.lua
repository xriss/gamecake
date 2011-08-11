#! /usr/bin/env lua

-- Notify property example

require("lgob.gtk")

local window = gtk.Window.new()
local entry = gtk.Entry.new()
entry:connect("notify::text", function() print(entry:get("text")) end)

window:add(entry)
window:set("title", "Notify", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)

window:show_all()
gtk.main()
