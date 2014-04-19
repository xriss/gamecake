
project "lua_kissfft"
language "C"

-- use a local copy incase we tweak, the kissfft dir is just for reference
files { "code/**.c" , "code/**.h" }

includedirs { "code" }
includedirs { "../lib_luajit/src" }

links { "lib_lua" }

KIND{lua="kissfft.core"}

