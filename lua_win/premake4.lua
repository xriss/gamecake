
project "lua_win"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." , "code" }

KIND{lua="wetgenes.win.core"}

