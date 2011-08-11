#!/usr/bin/env lua

--[[
	Tests the reference counting.
	Remember that the objects are finalized only on garbage collection!
--]]

require("lgob.gtk")

-- Object.get ~= TreeModel.get
local obj_get = gobject.Object.get

for i = 1, 25000 do
	local l1 = gtk.ListStore.new("gchararray")
	local l2 = gtk.TreeModelFilter.new(l1)
	obj_get(l2, "child-model")
   	obj_get(l2, "child-model")
	if i % 200 == 0 then collectgarbage() end
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
