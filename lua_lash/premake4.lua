

project "lua_lash"
language "C++"
files { "src/**.c" , "src/**.cpp" ,  "src/**.h" }
excludes { "src/rijndael.*" }

links { "lua51" }

SET_KIND("lua","lash","lash")
SET_TARGET("","lash")

