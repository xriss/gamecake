#!/usr/bin/env lua

--[[
	Checks for leaks in set/get. 
--]]

require("lgob.gtk")

local about = gtk.AboutDialog.new()

for i = 1, 20000 do
	about:set("program-name", "AboutDialoggggggggggggggggggggggggggggggggggg",
	"authors", {"a1Aaaaaaaa", "a2sdsadsadsad", "a3asdsadsa", "a4sadsadasds"})
	about:get("program-name", "authors")
	if i % 200 == 0 then collectgarbage() end
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
