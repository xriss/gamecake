
project "lua_zip"
language "C"
files { "src/**.c" , "src/**.h" }

links { "lib_lua" , "lib_zzip" }



includedirs { "." , "../lib_zzip" }


SET_KIND("lua","zip","zip")
SET_TARGET("","zip")

