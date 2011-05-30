
project "lua_gl"
language "C++"
files { "src/**.cpp" , "src/**.c" , "src/**.h" , "include/**.h" ,
	 "freeglut/src/**.cpp" , "freeglut/src/**.c" , "freeglut/src/**.h"  , "freeglut/include/**.h" }
excludes{ "src/Interpreter.c" , }

links { "lua51" }

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


includedirs { "src" , "include" , "freeglut/src" , "freeglut/include" , "freeglut/include/GL" }

SET_KIND("lua","gl","gl")
SET_TARGET("","gl")

