
--include("zlib")
--include("libpng")
--include("libjpeg")


project "lua_grd"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" , "lib_png" , "lib_z" }


defines { "JPEGSTATIC" }

includedirs { "." , "../lib_z" , "../lib_png" , "../lib_jpeg" }


SET_KIND("lua","wetgenes.grd.core","wetgenes_grd_core")
SET_TARGET("/wetgenes/grd","core")

