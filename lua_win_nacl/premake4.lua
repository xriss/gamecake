
project "lua_win_nacl"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." , "code" }

KIND{lua="wetgenes.win.nacl"}

