#!/usr/bin/env lua

--[[
	Checks for leaks and errors in lgui.Cursor.
--]]

require("lgob.gdk")
require("lgob.gtk")

local res = gtk.check_version(2, 13, 0)
if res then error(res) end
			
local w = gtk.Window.new()
local b = gtk.Button.new()
w:add(b)
b:realize()
local gw = b:get("window")

for i = 1, 9999 do
	gw:set_cursor(gdk.Cursor.new(gdk.CROSS))
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
