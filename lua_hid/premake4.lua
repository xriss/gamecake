
project "lua_hid"
language "C"

includedirs { "." , "../lib_hidapi/hidapi" }

files { "*.c" }

links { "lib_lua" }
links { "lib_hidapi" }

KIND{kind="lua",name="hid"}

