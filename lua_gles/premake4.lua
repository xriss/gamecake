
project "lua_gles"
language "C"

files { "code/*.c" }

links { "lib_lua" }

includedirs { "." , "code" }


KIND{lua="gles.core"}

