

project "lua_posix"
language "C"
files { "lposix.c" , "strlcpy.c" }
includedirs { "." }
links { "lib_lua" , "crypt" }

if OSX then
defines { "HAVE_STRLCPY" }
end


KIND{kind="lua",name="posix_c"}

