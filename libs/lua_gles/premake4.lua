
project "lua_gles"
language "C"

files { "code/lua_gles.c" , "code/**.h" }


includedirs { "include" }
includedirs { "../lib_hacks/code" }


if WINDOWS then

	files { "src/gl3w.c" }

else

	files { "src/gl3w.c" }
	defines "HAVE_FCNTL_H=1"
	
end

links { "lib_lua" }

KIND{lua="gles.core"}

