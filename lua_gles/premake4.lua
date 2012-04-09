
project "lua_gles"
language "C"

files { "code/**.c" , "code/**.h" }

includedirs { "code" }

if ANDROID then

	defines{ "LUA_GLES_GLES1" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES/gl.h\\\"" }
	
elseif NACL then

	defines{ "LUA_GLES_GLES2" }
	defines{ "INCLUDE_GLES_GL=\\\"GLES2/gl2.h\\\"" }
	
else -- use GL 
	defines{ "LUA_GLES_GL" }
	defines{ "INCLUDE_GLES_GL=\\\"GL/gl.h\\\"" }
end


links { "lib_lua" }

KIND{lua="gles.core"}

