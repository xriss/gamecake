
project "lua_chipmunk"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }
files { "master/src/**.c" , "master/src/**.h" , "master/include/**.h" }
excludes {
	"master/src/cpHastySpace.c", -- faster? but less portable, needs pthreads
}
links { "lib_lua" }

includedirs { "." , "master/include" }

buildoptions{ "-std=c99" } -- newfangled flag

-- disable debugs and asserts
configuration {"Release"}
	defines{"NDEBUG"}
configuration {}

KIND{lua="wetgenes.chipmunk.core"}

