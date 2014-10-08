
project "lua_rex_pcre"
language "C"

files { "src/common.c" , "src/common.h" }

files { "src/pcre/lpcre.c" , "src/pcre/lpcre_f.c" }

links { "lib_lua" , "lib_pcre" }

defines "VERSION=\\\"2.7.1\\\""

includedirs { "src" , "src/pcre" , "../lib_pcre"}

KIND{kind="lua",name="rex_pcre"}

