
project "lua"
language "C"

includedirs { "../lib_lua/src" }

if not NACL then -- disable lua stuff for small test

files { "hacks.c" }

dofile("cache.lua")
dofile("preloadlibs.lua")

--links(lua_lib_names)

end

links(static_lib_names)
links(static_lib_names) -- so good we linked it twice...


--print("LIBS TO LINK ",table.concat(static_lib_names,","))


if NACL then

	files { "../nacl/code/gltest.c" }

	linkoptions { "-v -O0" }

	links { "ppapi"  }
	links { "ppapi_gles2" }
	links { "m" , "stdc++" }
	
	KIND{kind="WindowedApp",name="lua.32.nexe"}

elseif ANDROID then 

--	linkoptions { "-v" }
	linkoptions { "-u JNI_OnLoad" } -- force exporting of JNI functions, without this it wont link
	
	links { "GLESv1_CM" }
	links { "dl", "log", "c", "m", "gcc" }	
	
--	linkoptions{ "-Bsymbolic"}

	files { "../lib_lua/src/*.h", --[["src/lua.c"]]  }
	KIND{kind="SharedLib",name="liblua"}


elseif WINDOWS then

	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }

	links { "opengl32" , "glu32" }
	links { "stdc++" , "ws2_32" , "gdi32"}
	
	links { "comdlg32" } -- we can dump this when we impliment our own freq

	KIND{kind="ConsoleApp",name="lua.exe"}

elseif NIX then

	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }
	
	links { "GL" , "GLU" }
	links { "crypt" }
	links { "pthread" }
	
	links { "dl" , "m" , "pthread" }
	
	KIND{kind="ConsoleApp",name="lua"}

end
