
project "lua_win_emcc"
language "C"

files {  "code/**.c" , "code/**.h" , "all.h" }
includedirs { "." , "code" }

KIND{lua="wetgenes.win.emcc.core"}

