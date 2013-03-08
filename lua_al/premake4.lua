
project "lua_al"
language "C"

defines( "AL_LIBTYPE_STATIC" )

files { "code/*.c" }

links { "lib_lua" }

includedirs { "../lib_openal/asoft/include"}



KIND{lua="al.core"}


project "lua_alc"
language "C"

defines( "AL_LIBTYPE_STATIC" )

files { "code/lua_alc.c" }

links { "lib_lua" }

includedirs { "../lib_openal/asoft/include" }


KIND{lua="alc.core"}

