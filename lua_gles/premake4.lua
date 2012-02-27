
project "lua_gles"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

links { "lib_lua" }

KIND{lua="gles.core"}

