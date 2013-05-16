
project "lua_pack"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

--buildoptions{ "-Wno-multichar" } -- we are using mulibyte char constants so please don't whine

includedirs { "." }

KIND{lua="wetgenes.pack.core"}

