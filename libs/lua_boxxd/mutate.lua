#!/usr/bin/env gamecake

mutate_from="2d"
mutate_into="3d"

mutate_files={
	["data/box"..mutate_from..".lua"] = "data/box"..mutate_into..".lua",
	["data/lua_box"..mutate_from..".c"] = "data/lua_box"..mutate_into..".c",
}

print("Mutating box"..mutate_from.." code into box"..mutate_into.." code.")

for from_filename,into_filename in pairs(mutate_files) do

	print("Mutating "..from_filename.." into "..into_filename)

end
