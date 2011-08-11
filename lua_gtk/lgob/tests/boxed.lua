#!/usr/bin/env lua

--[[
	Tests and checks for leaks in Boxed values. 
--]]

require("lgob.gdk")
require("lgob.gtk")

local btn = gtk.ColorButton.new()
local color = gdk.color_parse("#006688")

for i = 1, 50000 do
	btn:set("color", color)
	btn:get("color")
	assert(gdk.Color.to_string(color) == gdk.Color.to_string(btn:get("color")))
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
