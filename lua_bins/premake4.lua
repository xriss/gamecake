
project "lua_bins"
language "C"
files { "src/**.c" , "src/**.h" }

links { "lib_lua" }



includedirs { "."  }


KIND{kind="lua",name="luabins"}

