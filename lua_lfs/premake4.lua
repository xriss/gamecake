

project "lua_lfs"
language "C++"
files { "src/**.c" , "src/**.cpp" ,  "src/**.h" }


links { "lib_lua" }

KIND{kind="lua",name="lfs"}
