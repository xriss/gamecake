
project "lua_win_windows_core"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." , "code" }

KIND{lua="wetgenes.win.windows.core"}

