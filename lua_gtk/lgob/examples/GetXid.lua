#! /usr/bin/env lua

require("lgob.gtk")
require("lgob.gdk")

local win = gtk.Window.new()
win:realize()

local gdk_win = win:get("window")
print(gdk_win:get_handle())
