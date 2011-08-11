#! /usr/bin/env lua

local gtk = require("lgob.gtk")

local win = gtk.Window.new(gtk.WINDOW_TOPLEVEL)
win:set("title", "InfoBar demo", "window-position", gtk.WIN_POS_CENTER)
win:connect("delete-event", gtk.main_quit)

local bar1 = gtk.InfoBar.new()
bar1:add_button("Yes", 1)
bar1:add_button("No", 2)
bar1:set("message-type", gtk.MESSAGE_QUESTION)
bar1:get_content_area():add(gtk.Label.new("Did you liked this new widget?"))

bar1:connect("response", function(data, resp) print(resp) end)

local bar2 = gtk.InfoBar.new()
bar2:set("message-type", gtk.MESSAGE_ERROR)
bar2:get_content_area():add(gtk.Label.new("Fatal error! OK, not so fatal..."))

local vbox = gtk.VBox.new(false, 0)
vbox:add(bar2)
vbox:add(bar1)

win:add(vbox)
win:show_all()

gtk.main()
