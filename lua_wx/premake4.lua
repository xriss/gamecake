



project "lua_iup"
language "C"

includedirs { "cd/include" , "im/include" , "iup/include" , "iup/src" }


files { "iup/srcim/*.h" , "iup/srcim/*.c" }

files { "iup/srccd/*.h" , "iup/srccd/*.c" }

files { "iup/src/*.h" , "iup/src/*.c" }

links { "lua51" }

if os.get() == "windows" then

	files { "iup/src/win/*.h" , "iup/src/win/*.c" }

else -- nix

	files { "iup/src/gtk/iupgtk*.h" , "iup/src/gtk/iupgtk*.c" }

	local fp=assert(io.popen("pkg-config --cflags gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	buildoptions { s }
	fp:close()

	local fp=assert(io.popen("pkg-config --libs gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	linkoptions { s }
	fp:close()
end


files { "iup/srclua5/*.h" , "iup/srclua5/*.c" }


SET_KIND("lua","lua_iup","iup")
SET_TARGET("","lua_iup")

