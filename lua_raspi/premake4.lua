
project "lua_raspi"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

links { "lib_lua" }

KIND{lua="wetgenes.raspi.core"}




