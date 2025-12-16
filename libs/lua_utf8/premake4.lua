
project "lua_utf8"
language "C"
files { "lutf8lib.c"}

links { "lib_lua" }

includedirs { "." }

KIND{lua="utf8"}

