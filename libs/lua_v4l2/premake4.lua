
project "lua_v4l2"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs {
	"." ,
	"code/" ,
	"../lua_grd/code/" ,
}
includedirs { "../lib_hacks/code" }

KIND{lua="wetgenes.v4l2.core"}

