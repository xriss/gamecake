project "lua_linenoise"
language "C"
files { "linenoise/linenoise.c" }

includedirs { "linenoise" }

KIND{lua="linenoise"}
