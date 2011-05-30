

project "lua_lfs"
language "C++"
files { "src/**.c" , "src/**.cpp" ,  "src/**.h" }


links { "lua51" }


SET_KIND("lua","lfs","lfs")
SET_TARGET("","lfs")

