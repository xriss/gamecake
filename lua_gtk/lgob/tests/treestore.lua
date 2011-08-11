#! /usr/bin/env lua

require("lgob.gtk")

for i = 1, 5 do
	collectgarbage("collect")
	local tree = gtk.TreeStore.new("glong", "gchar")
	local iter = gtk.TreeIter.new()

	-- Adding some values
	for i = 1, 5000 do
		tree:append(iter)
		tree:set(iter, 0,  i * 5000, 1, "a")
	end
	
	-- Checking
	local valid = tree:get_iter_first(iter)
	local i = 1
	
	while valid do
		local a, b = tree:get(iter, 0, 1)
		assert(a == i * 5000)
		assert(b == 'a')
		i = i + 1
		valid = tree:iter_next(iter)
	end
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
