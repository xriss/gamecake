project "android_native"

language "C"

	includedirs { "code" }

files { "code/*.h" , "code/native.c" }

links { "lib_lua" }

KIND{}

