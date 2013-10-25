
project "lua_win_osx_core"
language "C"

files {  "code/**.c" ,  "code/**.m"  , "code/**.h" , "all.h" }
includedirs { "." }

KIND{lua="wetgenes.win.osx.core"}

