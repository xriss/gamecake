
project "lua_nacl"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." }

KIND{lua="wetgenes.nacl.core"}

