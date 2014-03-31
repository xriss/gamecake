
project "lua_profiler"
language "C"
files { "src/**.c" , "src/**.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="profiler"}

