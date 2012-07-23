
project "lua_win_android_core"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

links { "lib_lua" }

KIND{lua="wetgenes.win.android.core"}




