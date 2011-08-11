#! /usr/bin/env lua

--[[
	GRand example.
--]]

require("lgob.gobject")

for i = 1, 100 do
	print(glib.random_boolean(), glib.random_int_range(10, 99), glib.random_double())
end
