
project "lua_win_linux"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." }

KIND{lua="wetgenes.win.linux"}

