
project "lua_kissfft"
language "C"

-- use a local copy incase we tweak, the kissfft dir is just for reference

files { "code/lua_kissfft.c" }

-- ok lets use the kissfft from lib_speexdsp instead otherwise we link it twice.
-- should remove all the kiss code from in here...

includedirs { "code" }
includedirs { "../lib_luajit/src" }

links { "lib_lua" , "lua_pack" }

KIND{lua="kissfft.core"}

