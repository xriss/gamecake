
project "lua_android"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

links { "lib_lua" }

KIND{lua="wetgenes.android.core"}




