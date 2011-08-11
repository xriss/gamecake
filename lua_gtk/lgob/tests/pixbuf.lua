#! /usr/bin/env lua

require("lgob.gdk")

local pb = gdk.Pixbuf.new_from_file('icon.png')
assert(pb)

for i = 1, 1000 do
	pb:flip():rotate_simple(gdk.PIXBUF_ROTATE_UPSIDEDOWN)
	collectgarbage()
end

print("End.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
