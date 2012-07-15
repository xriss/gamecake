
project "lua_win_raspi"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

links { "lib_lua" }

KIND{lua="wetgenes.win.raspi"}




