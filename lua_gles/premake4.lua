
project "lua_gles"
language "C"

files { "code/lua_gles.c" , "code/**.h" }

includedirs { "code" }


if WINDOWS then

	files { "code/gl3w.c" }

	links { "opengl32" , "glu32" }
	links "gdi32"
	links "winmm"
--	defines "FREEGLUT_LIB_PRAGMAS=0"
--	defines ""
	
elseif OSX then

	files { "code/gl3w.c" }

	links { "GL" }--, "GLU" }
	defines "HAVE_FCNTL_H=1"

elseif LSB then

	files { "code/gl3w.c" }

	links { "GL" }--, "GLU" }
	defines "HAVE_FCNTL_H=1"

elseif RASPI or GAMECAKE_WIN_TYPE=="raspi" then

	includedirs { "/opt/vc/include" }

-- dont gl3w ??

elseif NIX then

	files { "code/gl3w.c" }

	links { "GL" }--, "GLU" }
	defines "HAVE_FCNTL_H=1"

else

	links { "GL" }--, "GLU" }
	defines "HAVE_FCNTL_H=1"
	
end

links { "lib_lua" }

KIND{lua="gles.core"}

