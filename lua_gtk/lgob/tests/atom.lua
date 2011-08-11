#!/usr/bin/env lua

--[[
	Checks GdkAtom.
--]]

require("lgob.gdk")

local name = "This is my string.!"

for i = 1, 100000 do
	local a = gdk.Atom.intern(name)
	assert(gdk.Atom.name(a) == name)
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
