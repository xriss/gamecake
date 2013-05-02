
project "lua_hid"
language "C"

includedirs { "." , "../lib_hidapi/hidapi" }

files { "code/*.c" }

links { "lib_lua" }
links { "lib_hidapi" }

KIND{kind="lua",dir="wetgenes/hid",name="core",luaname="wetgenes.hid.core",luaopen="wetgenes_hid_core"}

