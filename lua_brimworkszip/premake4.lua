
project "lua_brimworkszip"
language "C"
files { "./**.c" , "./**.h" }

links { "lib_lua" , "lib_zzip" }

includedirs { "." , "../lib_zip/lib" }

defines { "ZIP_EXTERN=extern" }

KIND{kind="lua",name="brimworks_zip"}

