
project "lua_lanes"
language "C++"
files { "src/**.cpp" , "src/**.c" , "src/**.h" }

links { "lib_lua" }





SET_KIND("lua","lua51-lanes","lanes")
SET_TARGET("","lua51-lanes")

