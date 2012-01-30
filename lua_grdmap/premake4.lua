
project "lua_grdmap"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" , "lib_grd" }


includedirs { "." , "../lua_grd" }


SET_KIND("lua","wetgenes.grdmap.core","wetgenes_grdmap_core")
SET_TARGET("/wetgenes/grdmap","core")

