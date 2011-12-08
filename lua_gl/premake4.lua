
project "lua_gl"
language "C"
files { "luagl/src/**.cpp" , "luagl/src/**.c" , "luagl/src/**.h" , "luagl/include/**.h" ,
	 "freeglut/src/**.cpp" , "freeglut/src/**.c" , "freeglut/src/**.h"  , "freeglut/include/**.h" }
excludes{ "src/Interpreter.c" , }

links { "lib_lua" }

defines "FREEGLUT_STATIC"

if os.get() == "windows" then

	links { "opengl32" , "glu32" }
	links "gdi32"
	links "winmm"
	defines "FREEGLUT_LIB_PRAGMAS=0"
	defines ""
	
else -- nix

	links { "GL" , "GLU" }
	defines "HAVE_FCNTL_H=1"
	
end


includedirs { ".", "luagl/src" , "luagl/include" , "freeglut/src" , "freeglut/include" , "freeglut/include/GL" }

SET_KIND("lua","gl","luagl")
SET_TARGET("","gl")

