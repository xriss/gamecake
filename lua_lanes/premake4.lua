
project "lua_lanes"
language "C"
files { "src/**.cpp" , "src/**.c" , "src/**.h" }

links { "lib_lua" }

KIND{lua="lanes.core"}
