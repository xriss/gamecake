#!/usr/bin/env lua

--[[
	Checks for memory leaks in the metatable trick.
--]]

require("lgob.gtk")

local MyLabel = {test_this = "test"}

for i = 1, 20000 do
	local label = gtk.Label.new()
	setmetatable(MyLabel, {__index = gtk.Label})
	label:cast(MyLabel)
	if i % 200 == 0 then collectgarbage() end
end

for i = 1, 20000 do
	local label = gtk.Label.new()
	setmetatable(MyLabel, {__index = gtk.Label})
	label:cast(MyLabel)
	label:cast(gtk.Button)
	label:cast(gtk.HBox)
	if i % 200 == 0 then collectgarbage() end
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
