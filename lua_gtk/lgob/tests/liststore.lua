#! /usr/bin/env lua

require("lgob.gtk")

for i = 1, 5 do
	collectgarbage("collect")
	local list = gtk.ListStore.new("glong", "gchar")
	local iter = gtk.TreeIter.new()

	-- Adding some values
	for i = 1, 10000 do
		list:append(iter)
		list:set(iter, 0,  i * 5000, 1, "a")
	end
	
	-- Checking
	local valid = list:get_iter_first(iter)
	local i = 1
	
	while valid do
		local a, b = list:get(iter, 0, 1)
		assert(a == i * 5000)
		assert(b == 'a')
		i = i + 1
		valid = list:iter_next(iter)
	end
	
	list:foreach(
		function(ud, path, iter)
			assert(ud == "hey")
			assert(path)
			assert(iter)
		end, "hey")
    
    list:get_iter_first(iter)
    
    for i = 1, 1e3 do
        path = list:get_path(iter)
    end
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
