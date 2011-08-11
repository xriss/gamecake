#! /usr/bin/env lua

require('lgob.gtk')

local window = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
window:connect('delete-event', gtk.main_quit)

local button = gtk.Button.new_with_label("Click me")
button:connect('clicked', function() print("clicked") end )

window:add(button)
window:show_all()

gtk.main()
