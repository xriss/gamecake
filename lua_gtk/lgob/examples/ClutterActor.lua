#! /usr/bin/env lua

require("lgob.clutter")
clutter.init_once()

-- Colors
local stage_color = clutter.color_from_string("black")
local actor_color = clutter.color_from_string("red")

-- Stage
local stage = clutter.stage_get_default()
stage:set_size(200, 200)
stage:set_color(stage_color)

-- Add a rectangle actor to the stage
local rect = clutter.Rectangle.new_with_color(actor_color)
rect:set_size(100, 100)
rect:set_position(20, 20)
stage:add_actor(rect)

-- Add a label actor to the stage
local label = clutter.Text.new_full("Sans 12", "Some Text", actor_color)
label:set_size(50, 50)
label:set_position(20, 150)
stage:add_actor(label)

-- Run
stage:show_all()
clutter.main()
