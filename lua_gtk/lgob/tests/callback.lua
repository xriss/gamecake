#!/usr/bin/env lua

--[[
	Tests callbacks.
--]]

require("lgob.gtk")

local label = gtk.Label.new("mylabel")
local count = 0

local button = gtk.Button.new()
button:connect("clicked", gtk.main_quit)

glib.timeout_add(glib.PRIORITY_DEFAULT, 1, 
	function(n)
		assert(n:get("label") == "mylabel")
		count = count + 1
		
		if count < 2000 then
			return true
		else
			button:clicked()
			return false
		end
	end, label)
	
gtk.main()

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
