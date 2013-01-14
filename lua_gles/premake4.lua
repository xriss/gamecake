
project "lua_gles"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }


if WINDOWS then

	links { "opengl32" , "glu32" }
	links "gdi32"
	links "winmm"
--	defines "FREEGLUT_LIB_PRAGMAS=0"
--	defines ""
	
else -- nix

	links { "GL" }--, "GLU" }
	defines "HAVE_FCNTL_H=1"
	
end

links { "lib_lua" }

KIND{lua="gles.core"}

