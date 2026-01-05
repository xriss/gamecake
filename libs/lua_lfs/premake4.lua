

project "lua_lfs"
language "C++"
files { "git/src/**.c" , "git/src/**.cpp" ,  "git/src/**.h" }


links { "lib_lua" }

KIND{kind="lua",name="lfs"}
