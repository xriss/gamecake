#!/usr/bin/env lua

--[[
	Object get/set method test.
--]]

require("lgob.gtk")

local label = gtk.Label.new("My label")
local about = gtk.AboutDialog.new()

-- object:set()

label:set("mnemonic-widget", about)
label:set("mnemonic-widget", nil)
label:set("mnemonic-widget", about)
label:set("wrap", true)
about:set("authors", nil)
about:set("program-name", "AboutDialog", "authors", {"a1", "a2", "a3", "a4"})

-- object:get()

assert(label:get("label") == "My label")
assert(label:get("angle") == 0)
local r1, r2 = label:get("cursor-position", "wrap")
assert(r1 == 0)
assert(r2 == true)
assert(label:get("justify") == gtk.JUSTIFY_LEFT)
assert(type(label:get("mnemonic-widget")) == "userdata")
assert(label:get("attributes") == nil)
assert(about:get("program-name") == "AboutDialog")
assert(about:get("authors")[2] == "a2")
assert(about:get("artists") == nil)

-- Test indirect access

label:set_label("new label")
label:set_angle(1)
assert(label:get_label() == "new label")
assert(label:get_angle() == 1)

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
