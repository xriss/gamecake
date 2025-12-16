
project "lua_grd"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" , "lib_png" , "lib_z" }


defines { "JPEGSTATIC" }

includedirs { "." , "../lib_z" , "../lib_png/fixed" , "../lib_jpeg/jpeg" , "../lib_gif/giflib/lib" }



KIND{kind="lua",dir="wetgenes/grd",name="core",luaname="wetgenes.grd.core",luaopen="wetgenes_grd_core"}

