#! /usr/bin/env lua

require('lgob.loader') -- special loader to fix dlopen issues
require('lgob.gtk')

-- GtkBuilder!

local builder = gtk.Builder.new()
builder:add_from_file('demo.ui')

-- List all objects
for i, j in pairs(builder:get_objects()) do
	print(i, j)
end

local handlers = {}

function handlers.connect_clicked(data)
    print("Connect clicked!", data)
end

function handlers.disconnect_clicked(data)
    print("Disconnect clicked!", data)
end

function handlers.quit_clicked(data)
    print("Quit clicked!", data)
    gtk.main_quit()
end

builder:connect_signals(handlers, "my userdata") -- default: _G, nil

local window = builder:get_object('window')
window:connect('delete-event', gtk.main_quit)
window:show_all()

gtk.main()
