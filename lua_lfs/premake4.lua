

project "lua_lfs"
language "C++"
files { "src/**.c" , "src/**.cpp" ,  "src/**.h" }


links { "lib_lua" }


SET_KIND("lua","lfs","lfs")
SET_TARGET("","lfs")

