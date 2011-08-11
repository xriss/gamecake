#!/usr/bin/env lua

--[[
	Checks the metatable trick implement a fake subclassing from C classes.
--]]

require("lgob.gtk")

local label1 = gtk.Label.new()
local label2 = gtk.Label.new()

local MyLabel = {test_this = "test"}
setmetatable(MyLabel, {__index = gtk.Label})

-- it's function due to the property hack to save space in lgob
assert(type(label1.test_this) ~= "string")
assert(type(label2.test_this) ~= "string")

label2:cast(MyLabel)

assert(type(label1.test_this) ~= "string")
assert(type(label2.test_this) == "string")
assert(label2:get("label") == "")

local label3 = gtk.Label.new()
local mt1 = getmetatable(label3)
label3:cast(gtk.Button)
local mt2 = getmetatable(label3)
label3:cast(gtk.HBox)
local mt3 = getmetatable(label3)

assert(mt1 ~= mt2)
assert(mt2 == mt3)

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
