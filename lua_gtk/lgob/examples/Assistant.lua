#! /usr/bin/env lua

-- Assistant example

require("lgob.gtk")

local assist = gtk.Assistant.new()
local page1, page2 = gtk.VBox.new(true, 10), gtk.VBox.new(true, 10)

page1:add(gtk.Label.new("First line of page 1!"), gtk.Label.new("Yeah!"))
page2:add(gtk.Label.new("First line of page 2!"), gtk.Label.new("Yeah!"))

assist:add(page1, page2)

assist:child_set(page1, "title", "Page 1", "page-type", gtk.ASSISTANT_PAGE_INTRO, "complete", true)
assist:child_set(page2, "title", "Page 2", "page-type", gtk.ASSISTANT_PAGE_CONFIRM)

assist:set("title", "Hello assistant!", "window-position", gtk.WIN_POS_CENTER)
assist:set("width-request", 400, "height-request", 300)
assist:connect("delete-event", gtk.main_quit)
assist:connect("cancel", gtk.main_quit)

assist:show_all()
gtk.main()
