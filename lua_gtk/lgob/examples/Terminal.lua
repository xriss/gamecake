#! /usr/bin/env lua

-- VTE example.

require("lgob.vte")

local terminal = vte.Terminal.new()
terminal:connect("child-exited", gtk.main_quit)
terminal:fork_command()

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
window:set("title", "Terminal demo", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit) 

window:add(terminal)
window:show_all()

gtk.main()

