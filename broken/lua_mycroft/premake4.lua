
project "lua_mycroft"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="wetgenes.mycroft.core"}

