
project "lua_grdmap"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" , "lib_grd" }


includedirs { "." , "../lua_grd" }


KIND{kind="lua",dir="wetgenes/grdmap",name="core",luaname="wetgenes.grdmap.core",luaopen="wetgenes_grdmap_core"}

