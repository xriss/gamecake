project "lua_zlib"
language "C"
files { "src/lua_zlib.c" }

links { "lib_lua" , "lib_z" }

includedirs { "." , "../lib_z/git" }


KIND{kind="lua",name="zlib"}

