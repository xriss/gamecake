
project "lua_win"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." }

KIND{lua="wetgenes.win.core"}

