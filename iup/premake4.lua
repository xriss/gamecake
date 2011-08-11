



project "lua_iup"
language "C"

includedirs { "include" , "src" , "srccd" , "srcim" , "srclua5" }


files { "srcim/*.h" , "srcim/*.c" }

files { "srccd/*.h" , "srccd/*.c" }

files { "src/*.h" , "src/*.c" }

links { "lua51" }

if os.get() == "windows" then

	files { "src/win/*.h" , "src/win/*.c" }

else -- nix

	files { "src/gtk/iupgtk*.h" , "src/gtk/iupgtk*.c" }

	local fp=assert(io.popen("pkg-config --cflags gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	buildoptions { s }
	fp:close()

	local fp=assert(io.popen("pkg-config --libs gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	linkoptions { s }
	fp:close()
end




files { "srclua5/*.h" , "srclua5/*.c" }


SET_KIND("lua","lua_iup","iup")
SET_TARGET("","lua_iup")

