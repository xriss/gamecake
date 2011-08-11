#!/usr/bin/env lua

--[[
	Container child_get / child_set method test.
--]]

require("lgob.gtk")

local assist = gtk.Assistant.new()
local page1	 = gtk.Label.new("Page1")
local page2	 = gtk.Label.new("Page2")
assist:add(page1)
assist:add(page2)

-- child_set()

assist:child_set(page1, "title", "Page 1", "page-type", gtk.ASSISTANT_PAGE_INTRO, "complete", true)
assist:child_set(page2, "title", "Page 2")
assist:child_set(page2, "page-type", gtk.ASSISTANT_PAGE_CONFIRM)

-- childGet()

assert(assist:child_get(page1, "title") == "Page 1")
assert(assist:child_get(page1, "page-type") == gtk.ASSISTANT_PAGE_INTRO)
assert(assist:child_get(page1, "complete") == true)
local r1, r2 = assist:child_get(page2, "page-type", "title")
assert(r1 == gtk.ASSISTANT_PAGE_CONFIRM)
assert(r2 == "Page 2")

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
