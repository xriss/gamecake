project "lib_nacl"

language "C"

files { "code/*.h" , "code/gltest.c" }

links { "lib_lua" }

KIND{}
