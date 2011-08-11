#! /usr/bin/env lua

require("lgob.clutter")
clutter.init_once()

local stage = clutter.stage_get_default()
stage:set_size(200, 100)
stage:set_color(clutter.color_from_string("black"))

local entry = clutter.Text.new()
entry:set_color(clutter.color_from_string("white"))
entry:set_editable(true)
entry:set_reactive(true)
entry:set_position(10, 10)
entry:set_size(200, 100)
entry:set_text("Edit me!")

entry:connect("button-press-event", function() stage:set_key_focus(entry) end)
entry:connect("activate", function() stage:set_key_focus() end)

stage:add_actor(entry)

stage:show_all()
clutter.main()
