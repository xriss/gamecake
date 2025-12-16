

project "lua_lash"
language "C++"
files { "src/**.c" , "src/**.cpp" ,  "src/**.h" }
excludes { "src/rijndael.*" }

links { "lib_lua" }

KIND{kind="lua",name="lash"}

