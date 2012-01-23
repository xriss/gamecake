
--include("zlib")
--include("libpng")
--include("libjpeg")


project "lua_grd"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" , "lib_png" , "lib_z" }




includedirs { "." , "../lib_z" , "../lib_png" , "../lib_jpeg" }


SET_KIND("lua","grd","grd")
SET_TARGET("","grd")

