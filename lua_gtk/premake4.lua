

project "lua_wx"
language "C++"

includedirs { "wx"  }


for i,v in ipairs{
"jpeg",
"png", 
"tiff", 
"zlib", 
"regex", 
"expat",
--"base", 
--"net", 
--"odbc", 
--"core", 
--"gl", 
"html", 
"xml", 
--"media", 
--"qa", 
--"adv", 
--"dbgrid", 
"xrc", 
"aui",
"richtext",
} do

files { "wx/"..v.."/*.h" , "wx/"..v.."/*.c" , "wx/"..v.."/*.cpp" }

end

--links { "lua51" }

if os.get() == "windows" then

--	files { "iup/src/win/*.h" , "iup/src/win/*.c" }

else -- nix

--	files { "iup/src/gtk/iupgtk*.h" , "iup/src/gtk/iupgtk*.c" }

	local fp=assert(io.popen("pkg-config --cflags gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	buildoptions { s }
	fp:close()

	local fp=assert(io.popen("pkg-config --libs gtkmm-2.4"))
	local s=assert(fp:read("*l"))
	linkoptions { s }
	fp:close()
end


-- files { "iup/srclua5/*.h" , "iup/srclua5/*.c" }


SET_KIND("lua","lua_gtk","gtk")
SET_TARGET("","lua_gtk")

