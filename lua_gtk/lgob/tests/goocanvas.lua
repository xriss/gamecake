#!/usr/bin/env lua

--[[
	Checks for errors in goocanvas.
--]]

require("lgob.goocanvas")

-- test1: no parent
for i = 1, 10000 do
	local rect = goocanvas.Rect.new(nil, 10, 10, 100, 100)
	assert(rect)
	if i % 100 == 0 then collectgarbage() end
end

-- test2: with parent
local r = goocanvas.Canvas.new():get_root_item()

for i = 1, 10000 do
	local rect = goocanvas.Rect.new(r, 10, 10, 100, 100)
	assert(rect)
	if i % 100 == 0 then collectgarbage() end
end

-- test3: with parent v2
for i = 1, 100 do
	local r = goocanvas.Canvas.new():get_root_item()
	
	for i = 1, 100 do
		local rect = goocanvas.Rect.new(r, 10, 10, 100, 100)
		assert(rect)
	end
	
	collectgarbage()
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
